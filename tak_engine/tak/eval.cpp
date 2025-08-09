#include "eval.h"
#include <vector>
#include <fstream>
#include <iostream>

alignas(64) int32_t Eval::output_bias = {};
alignas(64) int8_t Eval::output_weights[Eval::ACCUMULATOR_COUNT] = {};
alignas(64) int16_t Eval::accumulator_bias[Eval::ACCUMULATOR_COUNT] = {};
alignas(64) int16_t Eval::accumulator_weights[64][Eval::INPUTS_PER_SQUARE][Eval::ACCUMULATOR_COUNT] = {};

void Eval::init(const char* filePath)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);

	if (!file) throw std::runtime_error("cannot NNUE file!");

	auto file_size = file.tellg();
	if (file_size != (sizeof(output_bias) + sizeof(output_weights) + sizeof(accumulator_bias) + sizeof(accumulator_weights)))
		throw std::runtime_error("NNUE weight file has wrong size!");

	file.seekg(0, std::ios::beg);

	// load output bias
	file.read(reinterpret_cast<char*>(&output_bias), sizeof(output_bias));
	// load output weights
	file.read(reinterpret_cast<char*>(&output_weights), sizeof(output_weights));
	// load accumulator bias
	file.read(reinterpret_cast<char*>(&accumulator_bias), sizeof(accumulator_bias));
	// load accumulator weights
	file.read(reinterpret_cast<char*>(&accumulator_weights), sizeof(accumulator_weights));

	std::cout << "initialized nnue!" << std::endl;
}

Eval::Eval()
{
	// on startup, accumulators only hold bias
	for (int i = 0; i < ACCUMULATOR_COUNT; i++)
		accumulators[i] = accumulator_bias[i];
}

// ACCUMULATORS -> OUTPUT
// 512x int16_t -> 1x int16_t
int16_t Eval::get_eval()
{
	// optimized inference
	const __m256i* accum_vec = (const __m256i*) &accumulators[0]; // this is very probably always cached
	const __m256i* out_weights = (const __m256i*) &output_weights[0];
	const __m256i identity = _mm256_set1_epi16(1);

	// parallel iterations to reduce register dependencies and branches
	__m256i sum_vec_1 = _mm256_setzero_si256(); // 8x int32_t
	__m256i sum_vec_2 = _mm256_setzero_si256();
	__m256i sum_vec_3 = _mm256_setzero_si256(); // 8x int32_t
	__m256i sum_vec_4 = _mm256_setzero_si256();
	for (int a = 0; a < ACCUMULATOR_COUNT / 32; a+=4) {
		// CReLU: clamp accum results to unsigned byte values
		auto crelu_1 = _mm256_packus_epi16(accum_vec[2 * a    ], accum_vec[2 * a + 1]); // 32x int16_t => 32x uint8_t
		auto crelu_2 = _mm256_packus_epi16(accum_vec[2 * a + 2], accum_vec[2 * a + 3]); // 32x int16_t => 32x uint8_t
		auto crelu_3 = _mm256_packus_epi16(accum_vec[2 * a + 4], accum_vec[2 * a + 5]); // 32x int16_t => 32x uint8_t
		auto crelu_4 = _mm256_packus_epi16(accum_vec[2 * a + 6], accum_vec[2 * a + 7]); // 32x int16_t => 32x uint8_t
		// apply weights: multiply with output weights and do first horizontal add
		auto activation_1 = _mm256_maddubs_epi16(crelu_1, out_weights[a]); // 32x uint8_t * 32x int8_t => 16x int16_t (partially summed)
		auto activation_2 = _mm256_maddubs_epi16(crelu_2, out_weights[a + 1]); // 32x uint8_t * 32x int8_t => 16x int16_t (partially summed)
		auto activation_3 = _mm256_maddubs_epi16(crelu_3, out_weights[a + 2]); // 32x uint8_t * 32x int8_t => 16x int16_t (partially summed)
		auto activation_4 = _mm256_maddubs_epi16(crelu_4, out_weights[a + 3]); // 32x uint8_t * 32x int8_t => 16x int16_t (partially summed)
		// unpack to int32_t and do second horizontal add
		auto unpacked_1 = _mm256_madd_epi16(activation_1, identity); // 16x int16_t => 8x int32_t (partially summed)
		auto unpacked_2 = _mm256_madd_epi16(activation_2, identity); // 16x int16_t => 8x int32_t (partially summed)
		auto unpacked_3 = _mm256_madd_epi16(activation_3, identity); // 16x int16_t => 8x int32_t (partially summed)
		auto unpacked_4 = _mm256_madd_epi16(activation_4, identity); // 16x int16_t => 8x int32_t (partially summed)
		// add to partially summed result accumulation vector
		sum_vec_1 = _mm256_add_epi32(sum_vec_1, unpacked_1);
		sum_vec_2 = _mm256_add_epi32(sum_vec_2, unpacked_2);
		sum_vec_3 = _mm256_add_epi32(sum_vec_3, unpacked_3);
		sum_vec_4 = _mm256_add_epi32(sum_vec_4, unpacked_4);
	}

	// rest of horizontal sum of result accumulation vector
	sum_vec_1 = _mm256_add_epi32(sum_vec_1, sum_vec_2);
	sum_vec_2 = _mm256_add_epi32(sum_vec_3, sum_vec_4);
	auto sum_vec = _mm256_add_epi32(sum_vec_1, sum_vec_2); // 16 -> 8
	__m128i lo = _mm256_castsi256_si128(sum_vec);
	__m128i hi = _mm256_extracti128_si256(sum_vec, 1);

	auto sum = _mm_add_epi32(lo, hi); // 8 -> 4
	auto shuf = _mm_shuffle_epi32(sum, _MM_SHUFFLE(2, 3, 0, 1));
	sum = _mm_add_epi32(sum, shuf); // 4 -> 2
	shuf = _mm_shuffle_epi32(sum, _MM_SHUFFLE(1, 0, 3, 2));
	sum = _mm_add_epi32(sum, shuf); // 2 -> 1

	int32_t eval = _mm_cvtsi128_si32(sum) + output_bias;

	// clamp to allowed values and return
	// quantization: no tanh as in training, training tanh saturates approximately at -51200 (-2f) and +51200 (+2f)
	eval /= 4; // apply scaling
	return std::max<int32_t>(MIN_EVAL, std::min<int32_t>(MAX_EVAL, eval));
}

void Eval::incremental_add(int square, int feature_idx)
{
	const __m256i* square_accums = std::assume_aligned<64>((const __m256i*) &accumulator_weights[square][feature_idx]);
	__m256i* acc = std::assume_aligned<64>((__m256i*) &accumulators[0]);
	for (int a = 0; a < ACCUMULATOR_COUNT / 16; a+=4) {
		acc[a] = _mm256_add_epi16(acc[a], square_accums[a]);
		acc[a+1] = _mm256_add_epi16(acc[a + 1], square_accums[a + 1]);
		acc[a+2] = _mm256_add_epi16(acc[a + 2], square_accums[a + 2]);
		acc[a+3] = _mm256_add_epi16(acc[a + 3], square_accums[a + 3]);
	}
}

void Eval::incremental_remove(int square, int feature_idx)
{
	const __m256i* square_accums = std::assume_aligned<64>((const __m256i*) & accumulator_weights[square][feature_idx]);
	__m256i* acc = std::assume_aligned<64>((__m256i*) & accumulators[0]);
	for (int a = 0; a < ACCUMULATOR_COUNT / 16; a += 4) {
		acc[a] = _mm256_sub_epi16(acc[a], square_accums[a]);
		acc[a + 1] = _mm256_sub_epi16(acc[a + 1], square_accums[a + 1]);
		acc[a + 2] = _mm256_sub_epi16(acc[a + 2], square_accums[a + 2]);
		acc[a + 3] = _mm256_sub_epi16(acc[a + 3], square_accums[a + 3]);
	}
}
