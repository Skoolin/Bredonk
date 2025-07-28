#pragma once

#include "move_iterator.h"
#include <array>
#include "../util/random.h"

constexpr uint32_t BITS_PER_INDEX = 10; // bits for indexing magic table entries

struct magic_table_entry_t {
	uint8_t distances[4]; // 4 directions (up, right, down, left)

	uint8_t operator[](int direction) const {
		return distances[direction];
	}
};

// spread iterator
class SpreadIterator {
public:
	SpreadIterator(int start_square, int height, magic_table_entry_t entry);
	move_t next();
	void clear();
	bool is_empty() const;
	bool has_next() const;
private:
	void _forward();

	magic_table_entry_t distances; // magic lists for each direction (up, right, down, left)
	uint16_t current_perm; // current permutation
	uint8_t start_square; // starting square index for the spread
	uint8_t max_height;
	uint8_t square_and_type; // square index and type of the piece being spread
	uint8_t current_direction; // current direction of the spread

	bool cleared; // flag to check if the iterator has been cleared
};

constexpr std::array<uint64_t, 64> generate_square_masks()
{
	std::array<uint64_t, 64> masks = {};

	for (int row = 0; row < 6; row++) {
		for (int column = 0; column < 6; column++) {
			int square_idx = (row + 1) * 8 + column; // 6x6 board, 8x8 indexing for padded bitboards

			// create the magic mask for this square
			masks[square_idx] = (0x000000000000007FULL << ((1+row) * 8)) | (0x0001010101010100ULL << column);
		}
	}

	return masks;
}

// magic bitboard class
class Magic {
public:
	static void init();

	static SpreadIterator get_spread_iterator(int square_idx, uint64_t blocker_bitboard, int height);
	static SpreadIterator get_capstone_iterator(int square_idx, uint64_t wall_bitboard, uint64_t capstone_bitboard, int height);

private:
	static constexpr std::array<uint64_t, 64> magic_masks = generate_square_masks(); // masks for each square, used for fast lookup in the magic table
	static std::array<uint64_t, 64> magic_hashes; // precomputed magic numbers for multiplication with blocker bitboards

	static bool is_initialized; // flag to check if the magic bitboard is initialized
	static std::array<std::array<magic_table_entry_t, 1 << BITS_PER_INDEX>, 64> magic_table; // magic table for fast lookup of spreads
};
