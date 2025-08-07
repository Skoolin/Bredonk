#include "eval.h"

const int16_t Eval::accumulator_weights[64][Eval::INPUTS_PER_SQUARE][Eval::ACCUMULATOR_COUNT]{};
const int8_t Eval::output_weights[Eval::ACCUMULATOR_COUNT / 4]{};

Eval::Eval()
	: accumulators()
{
}

// ACCUMULATORS -> OUTPUT
// 512x int16_t -> 1x int16_t in ~32 cycles
// -> 640x clamp, 384x multiply, 487x addition
int16_t Eval::get_eval()
{
	// optimized inference
	const __m256i* accum_vec = (const __m256i*) &accumulators[0]; // this is very probably always cached
	const __m256i* out_weights = (const __m256i*) &output_weights[0];
	const __m256i identity = _mm256_set1_epi16(1);
	__m256i sum_vec = _mm256_setzero_si256(); // 8x int32_t
	// TODO manual unroll
#pragma loop(unroll(2))
	for (int a = 0; a < ACCUMULATOR_COUNT / 128; a++) { // this should take ~8 cycles per loop, 32 cycles total!!!
		// CReLU: clamp accum results to byte values, alternating signed and unsigned vectors for the maddubs instruction
		auto clamped_1 = _mm256_packus_epi16(accum_vec[8 * a], accum_vec[8 * a + 1]); // 32x int16_t => 32x uint8_t
		auto clamped_2 = _mm256_packs_epi16(accum_vec[8 * a + 2], accum_vec[8 * a + 3]); // 32x int16_t => 32x int8_t
		auto clamped_3 = _mm256_packus_epi16(accum_vec[8 * a + 4], accum_vec[8 * a + 5]);
		auto clamped_4 = _mm256_packs_epi16(accum_vec[8 * a + 6], accum_vec[8 * a + 7]);
		// pairwise multiplication: (https://www.chessprogramming.org/NNUE#Pairwise_Multiplication)
		// modified with an additional horizontal add to be abuse 8-bit fused multiply add of my favorite avx2 instruction
		// this is like an additional microscopic hidden layer
		auto pair_1 = _mm256_maddubs_epi16(clamped_1, clamped_2); // 32x uint8_t * 32x int8_t => 16x int16_t
		auto pair_2 = _mm256_maddubs_epi16(clamped_3, clamped_4);
		// CReLU: pack into unsigned 8 bit integers, this clamps / saturates to unsigned 8-bit range automatically
		auto crelu = _mm256_packus_epi16(pair_1, pair_2); // 32x int16_t => 32x uint8_t
		// apply weights: multiply with output weights and do first horizontal add
		// TODO perspective accumulators
		auto activation = _mm256_maddubs_epi16(crelu, out_weights[a]); // 32x uint8_t * 32x int8_t => 16x int16_t
		// unpack to int32_t and do second horizontal add
		auto unpacked = _mm256_madd_epi16(activation, identity); // 16x int16_t => 8x int32_t
		// add to partially summed result accumulation vector
		sum_vec = _mm256_add_epi32(sum_vec, unpacked);
	}

	// rest of horizontal sum of result accumulation vector
	sum_vec = _mm256_hadd_epi32(sum_vec, sum_vec); // 8 -> 4
	sum_vec = _mm256_hadd_epi32(sum_vec, sum_vec); // 4 -> 2
	sum_vec = _mm256_hadd_epi32(sum_vec, sum_vec); // 2 -> 1

	int32_t eval = _mm256_cvtsi256_si32(sum_vec);

	// re-scale TODO test scale values
	eval /= 64;
	// clamp to allowed values and return
	return std::max<int32_t>(MIN_EVAL, std::min<int32_t>(MAX_EVAL, eval));
}

void Eval::incremental_add(int square, int feature_idx)
{
	const __m256i* square_accums = (const __m256i*) &accumulator_weights[square][feature_idx];
	__m256i* acc = (__m256i*) &accumulators[0];
	// TODO this doesn't unroll correctly with MSVC...
#pragma loop(unroll(32))
	for (int a = 0; a < ACCUMULATOR_COUNT / 16; a++) {
		acc[a] = _mm256_add_epi16(acc[a], square_accums[a]);
	}
}

void Eval::incremental_remove(int square, int feature_idx)
{
	const __m256i* square_accums = (const __m256i*) & accumulator_weights[square][feature_idx];
	__m256i* acc = (__m256i*) & accumulators[0];
	// TODO this doesn't unroll correctly with MSVC...
#pragma loop(unroll(32))
	for (int a = 0; a < ACCUMULATOR_COUNT / 16; a++) {
		acc[a] = _mm256_sub_epi16(acc[a], square_accums[a]);
	}
}
