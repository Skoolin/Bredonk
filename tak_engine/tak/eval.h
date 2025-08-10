#pragma once

#include <stdint.h>
#include <immintrin.h>
#include <algorithm>
#include "stack.h"

class Eval {
public:
	static void init(const char* filePath);

	Eval();
	int16_t get_eval();
	void incremental_add(int square, int feature);
	void incremental_remove(int square, int feature);
	void incremental_add_stack(int square, int stack_height, stack_t stack);
	void incremental_remove_stack(int square, int stack_height, stack_t stack);

	inline static void incremental_prefetch(int square, int feature_idx) {
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][0], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][32], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][64], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][96], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][128], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][160], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][192], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][224], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][256], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][288], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][320], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][352], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][384], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][416], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][448], _MM_HINT_T0);
		_mm_prefetch((const char*)&accumulator_weights[square][feature_idx][480], _MM_HINT_T0);
	};
	constexpr static int MIN_EVAL = -30000;
	constexpr static int MAX_EVAL = 30000;

	inline int16_t get_sample_accum() const {
		return accumulators[0];
	}
private:
	constexpr static int CONSIDERED_CAPTIVES = 10;
	constexpr static int INPUTS_PER_SQUARE = 8 + 2 * CONSIDERED_CAPTIVES;
	constexpr static int ACCUMULATOR_COUNT = 512; // must be multiple of 64!

	alignas(64) int16_t accumulators[ACCUMULATOR_COUNT];

	alignas(64) static int16_t accumulator_weights[64][INPUTS_PER_SQUARE][ACCUMULATOR_COUNT]; // quantization: weights = ~40x float weights
	alignas(64) static int16_t accumulator_bias[ACCUMULATOR_COUNT];
	alignas(64) static int8_t output_weights[ACCUMULATOR_COUNT]; // quantization: weights = ~40x float weights
	alignas(64) static int32_t output_bias;
};
