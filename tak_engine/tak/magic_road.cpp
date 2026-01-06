#include "magic_road.h"
#include "../util/random.h"
#include <immintrin.h>

inline const static __m128i flood_expand(__m128i flood_vec, __m128i border_vec) {
	__m128i up = _mm_slli_si128(flood_vec, 8);
	__m128i down = _mm_srli_si128(flood_vec, 8);
	__m128i right = _mm_slli_si128(flood_vec, 1);
	__m128i left = _mm_srli_si128(flood_vec, 1);

	__m128i expanded = _mm_or_si128(flood_vec,
		_mm_or_si128(_mm_or_si128(up, down),
			_mm_or_si128(left, right)));

	return _mm_and_si128(expanded, border_vec);  // clear padding
}

inline const static bool equal128(__m128i a, __m128i b) {
	__m128i x = _mm_xor_si128(a, b);      // 0 where equal
	return _mm_testz_si128(x, x);         // ZF=1 if x == 0

}

inline const static bool has_road(uint64_t board) {
	uint64_t bot_mask = 0x000000000000003FULL;
	uint64_t top_mask = 0x00003F0000000000ULL;

	uint64_t current_top = board & top_mask;
	uint64_t current_bot = board & bot_mask;

	__m128i flood_vec = _mm_set_epi64x(current_top, current_bot);
	__m128i border_vec = _mm_set_epi64x(0x00003F3F3F3F3F3FULL, 0x00003F3F3F3F3F3FULL);

	while (true) {
		// do multiple iterations between each check
		auto next = flood_expand(flood_vec, border_vec);
		next = flood_expand(next, border_vec);
		next = flood_expand(next, border_vec);
		next = flood_expand(next, border_vec);

		if (equal128(flood_vec, next)) {
			// Final overlap check in scalar
			uint64_t lo = _mm_cvtsi128_si64(next);
			uint64_t hi = _mm_extract_epi64(next, 1);
			return (lo & hi) != 0;
		}
		flood_vec = next;
	}
}

// only check vertical road for now
const bool check_random(uint64_t board, uint64_t hash, uint64_t& remaining_mask) {
	if (has_road(board)) {
		// update mask
		uint64_t product = board * hash;


		// recursive call
		for (int i = 8; i < 48; i++) {
			uint64_t bit = 1ULL << i;
			if (board & bit) {
				bool rec = check_random(board ^ bit, hash, remaining_mask);
			}
		}
	}
}

const magic_hash_t generate_road_magic(uint64_t first_row)
{
	uint64_t start_bitboard = 0x003F3F3F3F3F00ULL | first_row; // start with full board in recursive search
	uint64_t start_mask = 0xFFFFFFFFFFFFFFFFULL;

	auto rand = ConstRandom::gen(123456787654);

	while (true)
	{
		uint64_t random_hash = ConstRandom::next(&rand);
		random_hash &= ConstRandom::next(&rand);
		random_hash &= ConstRandom::next(&rand);

		uint64_t current_mask = start_mask;

		if (check_random(start_bitboard, random_hash, current_mask))
			return { random_hash, current_mask };
	}

}

const std::array<magic_hash_t, 64> generate_road_magics()
{
	std::array<magic_hash_t, 64> result{};

	// indexed by 1st row, so less collision and less search space
	for (uint64_t i = 0; i < 63; i++) {
		result[i] = generate_road_magic(i);
	}

	// when 1st row is all ones, there is always a road
	result[63] = { 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL };

	return result;
}

std::array<magic_hash_t, 64> road_magics = generate_road_magics();
