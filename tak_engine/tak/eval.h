#pragma once

#include <stdint.h>
#include <immintrin.h>
#include <algorithm>

class Eval {
public:
	Eval();
	int16_t get_eval();
	void incremental_add(int square, int feature);
	void incremental_remove(int square, int feature);
	constexpr static int MIN_EVAL = -30000;
	constexpr static int MAX_EVAL = 30000;
private:
	constexpr static int CONSIDERED_CAPTIVES = 10;
	constexpr static int INPUTS_PER_SQUARE = 8 + 2 * CONSIDERED_CAPTIVES;
	constexpr static int HIDDEN_COUNT = 128; // must be multiple of 32!
	constexpr static int ACCUMULATOR_COUNT = 4 * HIDDEN_COUNT; // x4 because of pairwise multiplication and addition

	int16_t accumulators[ACCUMULATOR_COUNT];
	const static int16_t accumulator_weights[64][INPUTS_PER_SQUARE][ACCUMULATOR_COUNT];
	const static int8_t output_weights[HIDDEN_COUNT];
};
