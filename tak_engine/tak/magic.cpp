#include "magic.h"

#include <bit>
#include <iostream>
#include <vector>

static std::array<uint64_t, 64> generate_magics()
{
	std::array<uint64_t, 64> magics = {};

	// initialize random
	auto rand = ConstRandom::gen(87654321);

	for (int row = 0; row < 6; row++) {
		for (int col = 0; col < 6; col++) {
			int square_idx = (row + 1) * 8 + col; // 6x6 board, 8x8 indexing for padded bitboards
			uint64_t current_attempt = 0; // current attempt counter

			uint64_t collision_table[1 << BITS_PER_INDEX] = { 0 }; // collision table for magic hashes

			// generate all possible bitboards!
			std::vector<uint64_t> bitboards = {};
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

					bitboards.push_back(blocker_bitboard);
				}
			}


			while (++current_attempt) {
				if (current_attempt % 100000000 == 0)
					std::cout << "attempt # " << current_attempt << std::endl;

				bool end_attempt = false;

				// generate a random 64bit magic number
				uint64_t magic_hash = 0;
				magic_hash = ConstRandom::next(&rand);
				magic_hash &= ConstRandom::next(&rand);
				magic_hash &= ConstRandom::next(&rand);

				for (uint64_t blocker_bitboard : bitboards) {
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
		int row_idx = (row + 1) * 8; // 6x6 board, 8x8 indexing for padded bitboards
		for (int col = 0; col < 6; col++) {
			int square_idx = row_idx + col; // calculate square index

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

					// calculate magic table index
					uint64_t mask = Magic::magic_masks[square_idx];
					uint64_t hash = (blocker_bitboard & mask) * Magic::magic_hashes[square_idx];

					// calculate maximum distances for each direction
					int max_distances[4] = { 0, 0, 0, 0 }; // up, right, down, left

					// top
					uint8_t top_blockers = col_bitboard & (0b111111U << row);
					// count trailing zeros in top_blockers to find maximum distance upwards
					max_distances[0] = 6 - row - std::countr_zero(top_blockers); // up

					// right
					uint8_t right_blockers = row_bitboard & (0b111111U << col);
					// count trailing zeros in right_blockers to find maximum distance to the right
					max_distances[1] = 6 - col - std::countr_zero(right_blockers); // right

					// bottom
					uint8_t bottom_blockers = col_bitboard & (0b111111U >> (5 - row));
					// count leading zeros in bottom_blockers to find maximum distance downwards
					max_distances[2] = row - std::countl_zero(bottom_blockers); // down

					// left
					uint8_t left_blockers = row_bitboard & (0b111111U >> (5 - col));
					// count leading zeros in left_blockers to find maximum distance to the left
					max_distances[3] = col - std::countl_zero(left_blockers); // left

					hash >>= (uint64_t)(64U - BITS_PER_INDEX); // reduce to BITS_PER_INDEX bits
					auto entry = Magic::magic_table[square_idx][hash].distances;
					for (int direction = 0; direction < 4; direction++) {
						entry[direction] = (uint8_t) max_distances[direction];
					}
				}
			}
		}
	}

	is_initialized = true;
}

SpreadIterator Magic::get_spread_iterator(int square_idx, uint64_t blocker_bitboard, int height)
{
	if (height > 6)
		height = 6;
	uint64_t mask = Magic::magic_masks[square_idx];
	uint64_t hash = (blocker_bitboard & mask) * Magic::magic_hashes[square_idx];

	hash >>= (uint64_t)( 64U - BITS_PER_INDEX); // reduce to BITS_PER_INDEX bits
	magic_table_entry_t entry = Magic::magic_table[square_idx][hash];

	return SpreadIterator(square_idx, height, entry);
}

SpreadIterator Magic::get_capstone_iterator(int square_idx, uint64_t wall_bitboard, uint64_t capstone_bitboard, int height)
{
	// TODO implement capstone iterator creation
	return SpreadIterator(0, 0, Magic::magic_table[0][0]);
}

SpreadIterator::SpreadIterator(int start_square, int height, magic_table_entry_t entry) :
	start_square(start_square),
	max_height(height > 6 ? 6 : height),
	distances(entry),
	current_direction(0),
	current_perm(1),
	cleared(false)
{
	_forward();
}

// TODO is this cheaper than fetching from a pre-generated permutation table?
void SpreadIterator::_forward() {
	uint32_t max_perm = 1 << max_height;
	while (current_direction < 4) {
		if (current_perm >= max_perm) {
			current_direction++;
			current_perm = 1;
			continue;
		}
		int dist = std::popcount(current_perm);
		if (dist > distances[current_direction]) {
			int trailing_zeros = std::countr_zero(current_perm);
			current_perm += 1 << trailing_zeros;
			continue;
		}
		break;
	}
}

bool SpreadIterator::has_next() const {
	return (!cleared && current_direction < 4);
}

move_t SpreadIterator::next() {
	move_t move = { (uint8_t) (start_square | (current_direction << 6U)), (uint8_t) current_perm++ };
	_forward();
	return move;
}

void SpreadIterator::clear() {
	cleared = true;
}

bool SpreadIterator::is_empty() const {
	return !has_next();
}
