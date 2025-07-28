#pragma once
#include <cstdint>

// xorshift*
const class ConstRandom {
public:
	struct _random {
		uint64_t state;
	};

	constexpr static _random gen(uint64_t seed) {
		_random r = { seed ^ 0b1101010010010101010010101001000001111110100100010111000110000101ULL };
		next(&r);
		return r;
	};
	constexpr static uint64_t next(_random *random) {
		uint64_t state = random->state;
		state ^= state >> 12;
		state ^= state << 25;
		state ^= state >> 27;
		random->state = state;

		return state * 0x2545F4914F6CDD1DULL;
	};
};
