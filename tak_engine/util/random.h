#pragma once
#include <cstdint>

const static class ConstRandom {
public:
	struct _random {
		uint64_t state;
		uint64_t value;
	};

	constexpr static _random gen(uint64_t seed) {
		return next({ seed ^ 0b1101010010010101010010101001000001111110100100010111000110000101ULL, 0 });
	};
	constexpr static _random next(_random random) {
		uint64_t state = random.state;
		state ^= state >> 12;
		state ^= state << 25;
		state ^= state >> 27;
		uint64_t value = state * 0x2545F4914F6CDD1DULL;
		return { state, value };
	};
};
