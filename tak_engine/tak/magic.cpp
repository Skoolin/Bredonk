#include "magic.h"

#include <bit>
#include <iostream>
#include <random>

std::array<std::array<magic_table_entry_t, 1 << BITS_PER_INDEX>, 6> Magic::magic_table = {};
std::array < std::array<magic_table_entry_t, 1 << BITS_PER_INDEX>, 6> Magic::smash_magic_table = {};
std::array<std::array<uint8_t, 256>, 5> Magic::permutations = {};
int Magic::permutation_counts[6][5] = {};

uint64_t Magic::magic_hashes[64] = {};
uint64_t Magic::smash_magic_hashes[64] = {};

bool Magic::is_initialized = false;

void Magic::init() {
	if (is_initialized) {
		return; // already initialized
	}

	// initialize permutations and offset table
	for (int dist = 0; dist < 5; dist++) {
		int permutation_idx = 0;
		for (int height = 0; height < 6; height++) {
			for (uint8_t perm = 0; perm < (1 << height); perm++) {
				uint8_t full_perm = (1 << height) | perm;
				if (std::popcount(perm) > dist) {
					continue; // skip permutations with further spreads than allowed
				}
				permutations[dist][permutation_idx++] = full_perm; // store the permutation
			}
			permutation_counts[dist][height] = permutation_idx; // store the index of the last permutation for this height
		}
	}

	generate_magic_table();

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
					hash >>= (64 - BITS_PER_INDEX); // reduce to BITS_PER_INDEX bits

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

					// fill the magic table entry
					for (int direction = 0; direction < 4; direction++) {
						for (int height = 0; height < 6; height++) {
							auto entry = &Magic::magic_table[height][hash];

							int dist = max_distances[direction];
							if (dist > 0) {
								// find the index for this distance
								int index = Magic::permutation_counts[dist - 1][height];
								if (index > 0) {
									entry->lists[direction].num_moves = index;
									entry->lists[direction].dist = dist;
								}
								else {
									entry->lists[direction].num_moves = 0; // no moves for this direction
								}
							}
							else {
								entry->lists[direction].num_moves = 0; // no moves for this direction
							}
						}
					}
				}
			}
		}
	}

	is_initialized = true;
}

void Magic::generate_magic_table()
{
	std::random_device rd; // use a random device to seed the generator
	std::mt19937_64 gen(rd()); // 64-bit Mersenne Twister generator
	std::uniform_int_distribution<uint64_t> dis(1ULL << 63, UINT64_MAX); // ensure the magic number is non-zero

	for (int row = 0; row < 6; row++) {
		for (int col = 0; col < 6; col++) {
			int square_idx = (row + 1) * 8 + col; // 6x6 board, 8x8 indexing for padded bitboards
			int max_attempts = 50000000; // limit attempts to find a valid magic number
			int current_attempt = 0; // current attempt counter

			int collision_table[1 << BITS_PER_INDEX] = { 0 }; // collision table for magic hashes

			for (current_attempt = 1; current_attempt <= max_attempts; current_attempt++) {
				// generate a random 64bit magic number
				uint64_t magic_hash = 0;
				do {
					magic_hash = dis(gen); // generate a random magic number
					magic_hash &= dis(gen); // less popular magic numbers are more likely to be valid
					magic_hash &= dis(gen);
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
							goto _end_attempt;
						}
						collision_table[hash] = current_attempt; // mark this hash as used
					}
				}

				// no collisions found, print the magic number, save and move on to the next square!
				std::cout << "Magic number for square " << square_idx << ": " << magic_hash << std::endl;
				std::cout << "took " << current_attempt << " attempts." << std::endl;

				// save magic number
				Magic::magic_hashes[square_idx] = magic_hash;

				break;

			_end_attempt:
				continue; // label to break out of the inner loop

			}
			if (current_attempt > max_attempts) {
				std::cerr << "Failed to find a valid magic number for square " << square_idx << " after " << max_attempts << " attempts." << std::endl;
			}
		}
	}
}

SpreadIterator Magic::get_spread_iterator(int square_idx, uint64_t blocker_bitboard, int height)
{
	if (height > 6)
		height = 6;
	uint64_t mask = Magic::magic_masks[square_idx];
	uint64_t hash = (blocker_bitboard & mask) * Magic::magic_hashes[square_idx];

	hash >>= (uint64_t)( 64U - BITS_PER_INDEX); // reduce to BITS_PER_INDEX bits
	magic_table_entry_t entry = Magic::magic_table[height-1][hash];

	return SpreadIterator(square_idx, height, entry);
}

SpreadIterator Magic::get_capstone_iterator(int square_idx, uint64_t wall_bitboard, uint64_t capstone_bitboard, int height)
{
	// TODO implement capstone iterator creation
	return SpreadIterator(0, 0, Magic::smash_magic_table[0][0]);
}

SpreadIterator::SpreadIterator(int start_square, int height, magic_table_entry_t entry)
	: start_square(start_square),
	current_direction(0),
	current_index(0),
	cleared(false),
	entry(entry),
	square_and_type((uint8_t)(start_square & 0b00111111)) // square index and type bits
{
	while (current_direction < 4 && entry[current_direction].num_moves == 0) {
		current_direction++; // skip empty directions
	}
}

bool SpreadIterator::has_next() const
{
	if (cleared) {
		return false;
	}
	return current_direction < 4 && current_index < entry[current_direction].num_moves;
}

move_t SpreadIterator::next()
{
	uint8_t permutation = Magic::permutations[entry[current_direction].dist][current_index++];

	while (current_direction < 4 && current_index >= entry[current_direction].num_moves) {
		current_index = 0;
		current_direction++;
		square_and_type = (uint8_t)((start_square & 0b00111111) | (current_direction << 6)); // update direction bits
	}
	return { square_and_type, permutation };
}

void SpreadIterator::clear()
{
	cleared = true;
}

bool SpreadIterator::is_empty() const
{
	return cleared;
}
