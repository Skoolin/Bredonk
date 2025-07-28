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
class SpreadIterator : public MoveIterator {
public:
	SpreadIterator(int start_square, int height, magic_table_entry_t entry);
	move_t next() override;
	void clear() override;
	bool is_empty() const override;
	bool has_next() const override;
private:
	void _forward();

	int start_square; // starting square index for the spread
	int max_height;
	uint8_t square_and_type; // square index and type of the piece being spread
	magic_table_entry_t distances; // magic lists for each direction (up, right, down, left)
	uint32_t current_perm; // current permutation
	uint32_t current_direction; // current direction of the spread

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

constexpr const std::array<uint64_t, 64> generate_magics()
{

	std::array<uint64_t, 64> magics = {};
	// initialize random
	auto rand = ConstRandom::gen(87654321);

	for (int row = 0; row < 6; row++) {
		for (int col = 0; col < 6; col++) {
			int square_idx = (row + 1) * 8 + col; // 6x6 board, 8x8 indexing for padded bitboards
			int max_attempts = 500000000; // limit attempts to find a valid magic number
			int current_attempt = 0; // current attempt counter

			int collision_table[1 << BITS_PER_INDEX] = { 0 }; // collision table for magic hashes

			for (current_attempt = 1; current_attempt <= max_attempts; current_attempt++) {
				bool end_attempt = false;
				// generate a random 64bit magic number
				uint64_t magic_hash = 0;
				do {
					rand = ConstRandom::next(rand);
					magic_hash = rand.value;
				} while (magic_hash == 0); // ensure the magic number is non-zero

				for (uint8_t row_bitboard = 0; row_bitboard < (1 << 6); row_bitboard++) {
					for (uint8_t col_bitboard = 0; col_bitboard < (1 << 6); col_bitboard++) {
						uint64_t blocker_bitboard = 0;
						for (int r = 0; r < 6; r++) {
							if (row_bitboard & (1ULL << r)) {
								blocker_bitboard |= (1ULL << ((r + 1) * 8 + col)); // set row blocker
							}
						}
						for (int c = 0; c < 6; c++) {
							if (col_bitboard & (1ULL << c)) {
								blocker_bitboard |= (1ULL << ((row + 1) * 8 + c)); // set column blocker
							}
						}

						// ignore the origin square
						if (blocker_bitboard & (1ULL << square_idx)) {
							continue;
						}

						uint64_t hash = blocker_bitboard * magic_hash;
						hash >>= (64 - BITS_PER_INDEX); // reduce to BITS_PER_INDEX bits

						// check if this hash is already in use
						if (collision_table[hash] == current_attempt) {
							// collision detected; move on to next attempt
							end_attempt = true;
							break;
						}
						collision_table[hash] = current_attempt; // mark this hash as used
					}
					if (end_attempt)
						break;
				}

				// save magic number
				magics[square_idx] = magic_hash;

				break;

				if (end_attempt)
					continue; // label to break out of the inner loop

			}
		}
	}

	return magics;
}

// magic bitboard class
class Magic {
public:
	static void init();

	static SpreadIterator get_spread_iterator(int square_idx, uint64_t blocker_bitboard, int height);
	static SpreadIterator get_capstone_iterator(int square_idx, uint64_t wall_bitboard, uint64_t capstone_bitboard, int height);

private:
	static bool is_initialized; // flag to check if the magic bitboard is initialized
	static constexpr std::array<uint64_t, 64> magic_masks = generate_square_masks(); // masks for each square, used for fast lookup in the magic table

	static constexpr std::array<uint64_t, 64> magic_hashes = generate_magics(); // precomputed magic numbers for multiplication with blocker bitboards

	static std::array<std::array<magic_table_entry_t, 1 << BITS_PER_INDEX>, 64> magic_table; // magic table for fast lookup of spreads

};
