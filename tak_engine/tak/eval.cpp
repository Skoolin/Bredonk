#include "eval.h"

const int16_t Eval::accumulator_weights[64][Eval::INPUTS_PER_SQUARE][Eval::ACCUMULATOR_COUNT]{};
const int8_t Eval::output_weights[Eval::ACCUMULATOR_COUNT]{};

Eval::Eval()
	: accumulators()
{
}

// ACCUMULATORS -> OUTPUT
// 512x int16_t -> 1x int16_t
int16_t Eval::get_eval()
{
	// optimized inference
	const __m256i* accum_vec = (const __m256i*) &accumulators[0]; // this is very probably always cached
	const __m256i* out_weights = (const __m256i*) &output_weights[0];
	const __m256i identity = _mm256_set1_epi16(1);
	__m256i sum_vec = _mm256_setzero_si256(); // 8x int32_t
	// TODO manual unroll
	for (int a = 0; a < ACCUMULATOR_COUNT / 32; a++) {
		// CReLU: clamp accum results to unsigned byte values
		auto crelu = _mm256_packus_epi16(accum_vec[2 * a], accum_vec[2 * a + 1]); // 32x int16_t => 32x uint8_t
		// apply weights: multiply with output weights and do first horizontal add
		// TODO perspective accumulators
		auto activation = _mm256_maddubs_epi16(crelu, out_weights[a]); // 32x uint8_t * 32x int8_t => 16x int16_t (partially summed)
		// unpack to int32_t and do second horizontal add
		auto unpacked = _mm256_madd_epi16(activation, identity); // 16x int16_t => 8x int32_t (partially summed)
		// add to partially summed result accumulation vector
		sum_vec = _mm256_add_epi32(sum_vec, unpacked);
	}

	// rest of horizontal sum of result accumulation vector
	sum_vec = _mm256_hadd_epi32(sum_vec, sum_vec); // 8 -> 4
	sum_vec = _mm256_hadd_epi32(sum_vec, sum_vec); // 4 -> 2
	sum_vec = _mm256_hadd_epi32(sum_vec, sum_vec); // 2 -> 1

	int32_t eval = _mm256_cvtsi256_si32(sum_vec);

	// re-scale
	// TODO test scale values
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
