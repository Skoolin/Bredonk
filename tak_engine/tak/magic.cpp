#include "magic.h"
#include "bitboard.h"

#include <bit>
#include <iostream>
#include <vector>
#include "tak_board.h"

// TODO should be done at compile time for slightly faster startup
static std::array<uint64_t, 64> generate_magics()
{
	std::array<uint64_t, 64> magics = {};

	// initialize random
	auto rand = ConstRandom::gen(87654321);

	uint64_t current_attempt = 0; // current attempt counter
	for (int row = 0; row < 6; row++) {
		for (int col = 0; col < 6; col++) {
			int square_idx = row * 8 + col; // 6x6 board, 8x8 indexing for padded bitboards

			uint64_t collision_table[1 << BITS_PER_INDEX] = { 0 }; // collision table for magic hashes

			// generate all possible bitboards!
			std::vector<bitboard_t> bitboards = {};
			for (uint8_t row_bitboard = 0; row_bitboard < (1 << 6); row_bitboard++) {
				for (uint8_t col_bitboard = 0; col_bitboard < (1 << 6); col_bitboard++) {
					bitboard_t blocker_bitboard = 0;
					for (int r = 0; r < 6; r++) {
						if (row_bitboard & (1ULL << r)) {
							blocker_bitboard |= (1ULL << (r * 8 + col)); // set row blocker
						}
					}
					for (int c = 0; c < 6; c++) {
						if (col_bitboard & (1ULL << c)) {
							blocker_bitboard |= (1ULL << (row * 8 + c)); // set column blocker
						}
					}

					// ignore the origin square
					if (blocker_bitboard & (1ULL << square_idx)) {
						continue;
					}

					bitboards.push_back(blocker_bitboard);
				}
			}


			while (++current_attempt) {
				bool end_attempt = false;

				// generate a random 64bit magic number
				uint64_t magic_hash = ConstRandom::next(&rand);
				magic_hash &= ConstRandom::next(&rand);
				magic_hash &= ConstRandom::next(&rand);

				for (bitboard_t blocker_bitboard : bitboards) {
					bitboard_t hash = blocker_bitboard * magic_hash;
					hash >>= (64 - BITS_PER_INDEX); // reduce to BITS_PER_INDEX bits

					// check if this hash is already in use
					if (collision_table[hash] == current_attempt) {
						// collision detected; move on to next attempt
						end_attempt = true;
						break;
					}
					collision_table[hash] = current_attempt; // mark this hash as used
				}

				if (!end_attempt) {
					// save magic number
					magics[square_idx] = magic_hash;
					break;
				}
			}
		}
	}

	return magics;
}

std::array<uint64_t, 64> Magic::magic_hashes = generate_magics();
std::array<std::array<magic_table_entry_t, 1 << BITS_PER_INDEX>, 64> Magic::magic_table = {};

bool Magic::is_initialized = false;

void Magic::init() {
	if (is_initialized) {
		return; // already initialized
	}

	// initialize magic table
	for (int row = 0; row < 6; row++) {
		int row_idx = row * 8; // 6x6 board, 8x8 indexing for padded bitboards
		for (int col = 0; col < 6; col++) {
			int square_idx = row_idx + col; // calculate square index

			if (square_idx == (2 * 8 + 1)) {
				std::cout << std::endl;
			}

			for (uint8_t row_bitboard = 0; row_bitboard < (1 << 6); row_bitboard++) {
				for (uint8_t col_bitboard = 0; col_bitboard < (1 << 6); col_bitboard++) {
					bitboard_t blocker_bitboard = 0;
					for (int r = 0; r < 6; r++) {
						if (row_bitboard & (1ULL << r)) {
							blocker_bitboard |= (1ULL << (r * 8 + col)); // set row blocker
						}
					}
					for (int c = 0; c < 6; c++) {
						if (col_bitboard & (1ULL << c)) {
							blocker_bitboard |= (1ULL << (row * 8 + c)); // set column blocker
						}
					}

					if (blocker_bitboard & (1ULL << square_idx))
						continue;

					// calculate magic table index
					uint64_t hash = blocker_bitboard * Magic::magic_hashes[square_idx];

					// calculate maximum distances for each direction
					uint8_t max_distances[4] = { 0, 0, 0, 0 }; // up, right, down, left
					uint8_t max_dist = 0;

					// up
					uint8_t top_blockers = row_bitboard >> (row+1);
					// count trailing zeros in top_blockers to find maximum distance upwards
					max_dist = std::countr_zero(top_blockers);
					uint8_t up_distance = 5 - row;
					if (max_dist < up_distance)
						up_distance = max_dist;
					max_distances[0] = up_distance;

					// bottom
					uint8_t bottom_blockers = row_bitboard << (8-row);
					// count leading zeros to find maximum distance downwards
					max_dist = std::countl_zero(bottom_blockers);
					uint8_t down_distance = row;
					if (max_dist < down_distance)
						down_distance = max_dist;
					max_distances[2] = down_distance;

					// right
					uint8_t right_blockers = col_bitboard >> (col+1);
					// count trailing zeros to find maximum distance to right
					max_dist = std::countr_zero(right_blockers);
					uint8_t right_distance = 5 - col;
					if (max_dist < right_distance)
						right_distance = max_dist;
					max_distances[1] = right_distance;

					// left
					uint8_t left_blockers = col_bitboard << (8-col);
					// count leading zeros to find maximum distance to left
					max_dist = std::countl_zero(left_blockers);
					uint8_t left_distance = col;
					if (max_dist < left_distance)
						left_distance = max_dist;
					max_distances[3] = left_distance;

					hash >>= (64U - BITS_PER_INDEX); // reduce to BITS_PER_INDEX bits
					for (int direction = 0; direction < 4; direction++) {
						Magic::magic_table[square_idx][hash].distances[direction] = max_distances[direction];
					}
				}
			}
		}
	}

	is_initialized = true;
}

SpreadIterator Magic::get_spread_iterator(int square_idx, bitboard_t wall_bitboard, bitboard_t capstone_bitboard, int height)
{
	if (height > 6)
		height = 6;

	bitboard_t mask = Magic::magic_masks[square_idx];
	uint64_t hash = ((wall_bitboard | capstone_bitboard) & mask) * Magic::magic_hashes[square_idx];

	hash >>= (uint64_t)(64U - BITS_PER_INDEX); // reduce to BITS_PER_INDEX bits
	magic_table_entry_t entry = Magic::magic_table[square_idx][hash];
	auto is_cap = capstone_bitboard & (1ULL << square_idx);

	return SpreadIterator(square_idx, height, entry, (is_cap ? (wall_bitboard & mask) : 0ULL));
}

SpreadIterator::SpreadIterator(int start_square, int height, magic_table_entry_t entry, bitboard_t wall_bitboard) :
	start_square(start_square),
	max_perm(1U << (height > 6 ? 6 : height)),
	distances(entry),
	current_direction(0),
	current_perm(0),
	cleared(false),
	can_smash(wall_bitboard != 0),
	wall_bitboard(wall_bitboard)
{
	_forward();
}

int SpreadIterator::count()
{
	int c = 0;
	while (has_next()) {
		next();
		c++;
	}
	return c;
}

// TODO is this cheaper than fetching from a pre-generated permutation table?
void SpreadIterator::_forward() {
	while (current_direction < 4) {
		// check for smash
		if (can_smash && !(current_perm & 0b1U) // only if has reach
			) {
			auto dist = 1U + std::popcount(current_perm);
			bitboard_t smash_square_bitboard = 1ULL << (start_square + dist * TakBoard::offsets[current_direction]);
			if (smash_square_bitboard & wall_bitboard) { // check if next square is blocker
				current_perm++;
				return;
			}
		}
		current_perm++;
		_inner:
		if (current_perm >= max_perm) {
			current_direction++;
			current_perm = 0;
			continue;
		}
		int dist = std::popcount(current_perm);
		if (dist > distances[current_direction]) {
			int trailing_zeros = std::countr_zero(current_perm);
			current_perm += 1 << trailing_zeros;
			goto _inner;
		}
		break;
	}
}

bool SpreadIterator::has_next() const {
	return (!cleared && current_direction < 4);
}

move_t SpreadIterator::next() {
	move_t move = { (uint8_t) (start_square | (current_direction << 6U)), (uint8_t) current_perm };
	_forward();
	return move;
}

void SpreadIterator::clear() {
	cleared = true;
}

bool SpreadIterator::is_empty() const {
	return !has_next();
}
