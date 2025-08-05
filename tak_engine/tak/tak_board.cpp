#include "tak_board.h"
#include "../search/search.h"
#include "../params.h"
#include "move_list.h"
#include "enums.h"
#include "magic.h"
#include <iostream>
#include <string>

TakBoard::TakBoard() :
	top_stones{ Piece::NONE },
	stack_sizes{ 0 },
	stacks{ Piece::NONE },

	previous_moves{ 0, 0 },
	move_count(0),
	did_flatten{ false },

	state(STATE_ONGOING),
	current_player(PLAYER_WHITE),
	zobrist(0),

	w_reserves(30),
	b_reserves(30),
	w_cap_placed(false),
	b_cap_placed(false),

	bordered_bitboards{
		BORDER_MASK, // NONE
		0ULL, // W_FLAT
		0ULL, // W_WALL
		0ULL, // W_CAP
		0ULL, // __ILLEGAL__
		0ULL, // B_FLAT
		0ULL, // B_WALL
		0ULL  // B_CAP
	}
{
	Magic::init();
	for (int i = 0; i < MAX_GAME_LENGTH; i++)
		move_lists[i] = nullptr;
}

TakBoard::~TakBoard()
{
	// clean up move lists
	for (int i = 0; i < MAX_GAME_LENGTH; i++) {
		if (move_lists[i] != nullptr) {
			delete move_lists[i];
			move_lists[i] = nullptr;
		}
	}
}

std::string TakBoard::get_tps() const
{
	std::string tps = "";

	for (int row = 0; row < 6; row++) {
		for (int col = 0; col < 6; col++) {
			int tps_col = 5 - col;
			int square_idx = 8 * row + tps_col;
			if (stack_sizes[square_idx]) {
				auto stack = stacks[square_idx];
				for (int i = 0; i < stack_sizes[square_idx] - 1; i++)
					tps.append(stack[i].get_player() == -1 ? "2" : "1");
				auto top_stone = top_stones[square_idx];
				tps.append(top_stone.get_player() == -1 ? "2" : "1");
				if (top_stone.is_capstone())
					tps.append("C");
				if (top_stone.is_wall())
					tps.append("S");
			} else {
				tps.append("x");
			}
			if (col < 5)
				tps.append(",");
		}
		if (row < 5)
			tps.append("/");
	}

	tps.append(current_player == 1 ? " 1 " : " 2 ");
	tps.append(std::to_string(move_count/2+1));

	return tps;
}

/*
* flood fill search on bit mask
* TODO move to bitboard.h, check border masks, make faster
*/
static bool has_road(bitboard_t bitmap) {
	// bordered layout: (1 = board, 0 = border)
	// This helps with flood fill!
	// 00000000
	// 00000000
	// 00111111
	// 00111111
	// 00111111
	// 00111111
	// 00111111
	// 00111111

	// TODO early stopping if road already found even if floodfill incomplete
	// TODO test loop unrolling

	const bitboard_t LEFT = 0b000000000000000000100000001000000010000000100000001000000010000000000000ULL;
	const bitboard_t RIGHT = 0b000000000000000000000000010000000100000001000000010000000100000001000000ULL;
	const bitboard_t TOP = 0b000000000000000000111111000000000000000000000000000000000000000000000000ULL;
	const bitboard_t BOTTOM = 0b000000000000000000000000000000000000000000000000000000000000000000111111ULL;

	// search left to right
	bitboard_t fill_map = 0;
	bitboard_t step_map = LEFT & bitmap;
	while (step_map != fill_map) {
		fill_map = step_map;
		step_map = (fill_map | fill_map << 1 | fill_map >> 1 | fill_map << 8 | fill_map >> 8) & bitmap;
	}
	if (step_map & RIGHT)
		return true;

	// search top to bottom
	fill_map = 0;
	step_map = TOP & bitmap;
	while (step_map != fill_map) {
		fill_map = step_map;
		step_map = (fill_map | fill_map << 1 | fill_map >> 1 | fill_map << 8 | fill_map >> 8) & bitmap;
	}
	if (step_map & BOTTOM)
		return true;

	return false;
}

bool TakBoard::is_final() const
{
	// check reserves
	if (w_reserves == 0 && w_cap_placed)
		return true;
	if (b_reserves == 0 && b_cap_placed)
		return true;

	// check board fill
	bitboard_t empty_bitmap = get_bordered_bitboard(Piece::NONE);
	if (empty_bitmap == 0)
		return true;

	// check for roads
	bitboard_t w_bitmap = get_bordered_bitboard(Piece::W_CAP) | get_bordered_bitboard(Piece::W_FLAT);
	bitboard_t b_bitmap = get_bordered_bitboard(Piece::B_CAP) | get_bordered_bitboard(Piece::B_FLAT);
	
	if (has_road(w_bitmap))
		return true;
	return has_road(b_bitmap);
}

int32_t TakBoard::get_result() const
{
	if (!is_final())
		return STATE_ONGOING;

	// check for roads
	bitboard_t w_bitmap = get_bordered_bitboard(Piece::W_CAP) | get_bordered_bitboard(Piece::W_FLAT);
	bitboard_t b_bitmap = get_bordered_bitboard(Piece::B_CAP) | get_bordered_bitboard(Piece::B_FLAT);

	// TODO don't repeat has_road check, already done in is_final()
	bool w_road = has_road(w_bitmap);
	bool b_road = has_road(b_bitmap);

	// dragon clause: if both players have a road, the player who made the final move wins
	if (current_player == PLAYER_BLACK && w_road)
		return STATE_WHITE_WIN;
	if (current_player == PLAYER_WHITE && b_road)
		return STATE_BLACK_WIN;
	if (w_road)
		return STATE_WHITE_WIN;
	if (b_road)
		return STATE_BLACK_WIN;

	int w_count = get_bordered_bitboard(Piece::W_FLAT).count();
	int b_count = get_bordered_bitboard(Piece::B_FLAT).count();

	//TODO komi
	if (w_count > b_count)
		return STATE_WHITE_WIN;
	if (b_count > w_count)
		return STATE_BLACK_WIN;

	return STATE_DRAW;
}

// returns zobrist hash of the current board state
uint64_t TakBoard::get_hash()
{
	// TODO implement zobrist
	return 0;
}

static int32_t get_player(Piece p) {
	if (p == 0)
		return 0;
	return (p > 0) ? 1 : -1;
}

void TakBoard::make_move(move_t m)
{
	// switch player
	zobrist ^= ZOBRISTS[NUM_ZOBRISTS - 1];
	if (m.is_spread()) { // spread move
		uint8_t start_square = m.square_idx();
		int direction = m.spread_direction();
		int square_offset = offsets[direction];

		// picked up stones
		Piece stones[8];

		uint8_t perm = m.spread_perm;
		// count how many captives need to be picked up
		int leading_zeros = std::countl_zero(perm); // count trailing zeros in spread_perm
		int picked_up = 8 - leading_zeros; // 8 is the max number of pieces in a move representation
		
		perm <<= leading_zeros; // remove trailing zeros from perm
		
		// pick up stones

		// top stone:
		// TODO this can be done directly on the square stack!
		stones[0] = top_stones[start_square];
		bordered_bitboards[top_stones[start_square].to_int()] &= ~(1ULL << start_square); // remove top stone from bitboard
		zobrist ^= ZOBRISTS[start_square * 12 + top_stones[start_square].to_int()];
		stack_sizes[start_square]--;

		// rest of stack:
		for (int i = 1; i < picked_up; i++) {
			stack_sizes[start_square]--;
			auto size = stack_sizes[start_square];
			stones[i] = stacks[start_square][size];
			bool b = stones[i] == Piece::B_FLAT;
			uint64_t upper = ZOBRISTS[start_square * 12 + 8 + 2 * b];
			uint64_t lower = ZOBRISTS[start_square * 12 + 9 + 2 * b];
			zobrist ^= (upper << size) | (lower >> (64-size));
		}

		// fix top stone
		if (stack_sizes[start_square]) {
			auto size = stack_sizes[start_square] - 1;
			Piece p = stacks[start_square][size];
			top_stones[start_square] = p;
			zobrist ^= ZOBRISTS[start_square * 12 + p.to_int()];
			bordered_bitboards[p.to_int()] |= (1ULL << start_square); // add piece to bitboard
			bool b = p == Piece::B_FLAT;
			uint64_t upper = ZOBRISTS[start_square * 12 + 8 + 2 * b];
			uint64_t lower = ZOBRISTS[start_square * 12 + 9 + 2 * b];
			zobrist ^= (upper << size) | (lower >> (64-size));
		}
		else {
			top_stones[start_square] = Piece::NONE;
			// no stone left on this square
			bordered_bitboards[((Piece)Piece::NONE).to_int()] |= (1ULL << start_square); // add empty square to bitboard
		}


		// put down stones
		int current_square = start_square;
		for (int i = 0; i < picked_up; i++) {
			Piece stone = stones[picked_up - i - 1];

			bordered_bitboards[top_stones[current_square].to_int()] &= ~(1ULL << current_square); // remove top stone from bitboard
			if (top_stones[current_square] != Piece::NONE) {
				zobrist ^= ZOBRISTS[current_square * 12 + top_stones[current_square].to_int()];
			}

			if (perm & 0b10000000U) { // move to next square
				current_square += square_offset;
				// smash top wall
				if (stone == Piece::W_CAP || stone == Piece::B_CAP) {
					if (top_stones[current_square].is_wall()) {
						did_flatten[move_count] = true;
						if (top_stones[current_square] == Piece::B_WALL) {
							top_stones[current_square] = Piece::B_FLAT;
						}
						if (top_stones[current_square] == Piece::W_WALL) {
							top_stones[current_square] = Piece::W_FLAT;
						}
					}
					else {
						did_flatten[move_count] = false;
					}
				}
			}
			if (stack_sizes[current_square]) {
				Piece p = top_stones[current_square];
				int size = stack_sizes[current_square] - 1;
				stacks[current_square].set(size, p);
				bool b = p == Piece::B_FLAT;
				uint64_t upper = ZOBRISTS[start_square * 12 + 8 + 2 * b];
				uint64_t lower = ZOBRISTS[start_square * 12 + 9 + 2 * b];
				zobrist ^= (upper << size) | (lower >> (64 - size));
			}
			top_stones[current_square] = stone;
			zobrist ^= ZOBRISTS[current_square * 12 + stone.to_int()];
			bordered_bitboards[stone.to_int()] |= (1ULL << current_square); // add piece to bitboard
			stack_sizes[current_square]++;
			perm <<= 1; // shift to next stone
		}
	}
	else { // placement move
		int square = m.square_idx();
		Piece p = m.piece_type(is_swap() ? (0 - current_player) : current_player);
		zobrist ^= ZOBRISTS[square * 12 + p.to_int()];

		top_stones[square] = p;
		bordered_bitboards[((Piece)Piece::NONE).to_int()] &= ~(1ULL << square); // remove empty square from bitboard
		bordered_bitboards[p.to_int()] |= (1ULL << square); // add piece to bitboard
		stack_sizes[square] = 1;

		// adjust reserves
		if (m.is_cap_placement()) {
			if (current_player == 1) // white
				w_cap_placed = true;
			else
				b_cap_placed = true;
		}
		// incorrect for first move but correct after
		else {
			if (current_player == 1) // white
				w_reserves--;
			else
				b_reserves--;
		}
	}

	current_player = -current_player;
	move_count++;
}

void TakBoard::undo_move(move_t m)
{
	// TODO update zobrist

	move_count--;
//	move_t m = previous_moves[move_count];
	current_player = -current_player;

	if (m.is_spread()) { // undo spread move
		int start_square = m.square_idx();
		int direction = m.spread_direction();
		int square_offset = offsets[direction];
		int distance = m.spread_distance();

		uint8_t perm = m.spread_perm;
		int leading_zeros = std::countl_zero(perm); // count trailing zeros in spread_perm
		int pieces_to_put_back = 8 - leading_zeros; // 8 is the max number of pieces in a move representation

		// TODO this can be done directly on the square stack!
		Piece stones[8]{ Piece::NONE };

		// pick up stones
		int current_square = start_square + square_offset * distance;
		bool is_last_square = true;

		for (int i = pieces_to_put_back - 1; i >= 0; i--) {
			Piece stone = top_stones[current_square];
			bordered_bitboards[stone.to_int()] &= ~(1ULL << current_square); // remove piece from bitboard
			zobrist ^= ZOBRISTS[current_square * 12 + stone.to_int()];
			stones[i] = stone;

			stack_sizes[current_square]--;
			if (stack_sizes[current_square] > 0) {
				int size = stack_sizes[current_square] - 1;
				Piece p = stacks[current_square][size];
				bool b = p == Piece::B_FLAT;
				uint64_t upper = ZOBRISTS[start_square * 12 + 8 + 2 * b];
				uint64_t lower = ZOBRISTS[start_square * 12 + 9 + 2 * b];
				zobrist ^= (upper << size) | (lower >> (64 - size));
				top_stones[current_square] = p;
				zobrist ^= ZOBRISTS[current_square * 12 + p.to_int()];
				bordered_bitboards[top_stones[current_square].to_int()] |= (1ULL << current_square); // add piece to bitboard
			}
			else {
				top_stones[current_square] = Piece::NONE;
				bordered_bitboards[((Piece)Piece::NONE).to_int()] |= (1ULL << current_square); // add empty square to bitboard
			}
			if (perm & 0b1U) {
				// undo flatten
				if (is_last_square && stone.is_capstone() && did_flatten[move_count]) {
					Piece top_stone = top_stones[current_square];
					bordered_bitboards[top_stone.to_int()] &= ~(1ULL << current_square); // remove top stone from bitboard
					Piece p = top_stone == Piece::B_FLAT ? Piece::B_WALL : Piece::W_WALL;
					zobrist ^= ZOBRISTS[current_square * 12 + top_stone.to_int()];
					zobrist ^= ZOBRISTS[current_square * 12 + p.to_int()];
					top_stones[current_square] = p;
					bordered_bitboards[p.to_int()] |= (1ULL << current_square);
				}
				// move to previous square
				current_square -= square_offset;
				is_last_square = false;
			}
			perm >>= 1; // shift to next stone
		}

		// put back stones
		if (stack_sizes[start_square] > 0) {
			int size = stack_sizes[start_square];
			Piece p = top_stones[start_square];
			stacks[start_square].set(size, p);
			zobrist ^= ZOBRISTS[start_square * 12 + p.to_int()];
			bool b = p == Piece::B_FLAT;
			uint64_t upper = ZOBRISTS[start_square * 12 + 8 + 2 * b];
			uint64_t lower = ZOBRISTS[start_square * 12 + 9 + 2 * b];
			zobrist ^= (upper << size) | (lower >> (64 - size));
		}
		bordered_bitboards[top_stones[start_square].to_int()] &= ~(1ULL << start_square); // remove top stone from bitboard
		for (int i = 0; i < pieces_to_put_back; i++) {
			int size = stack_sizes[start_square] + i;
			Piece p = stones[i];
			stacks[start_square].set(size, p);
			bool b = p == Piece::B_FLAT;
			uint64_t upper = ZOBRISTS[start_square * 12 + 8 + 2 * b];
			uint64_t lower = ZOBRISTS[start_square * 12 + 9 + 2 * b];
			zobrist ^= (upper << size) | (lower >> (64 - size));
		}
		stack_sizes[start_square] += pieces_to_put_back;
		// adjust top stone
		int size = stack_sizes[start_square] - 1;
		Piece p = stacks[start_square][size];
		top_stones[start_square] = p;
		zobrist ^= ZOBRISTS[start_square * 12 + p.to_int()];
		bool b = p == Piece::B_FLAT;
		uint64_t upper = ZOBRISTS[start_square * 12 + 8 + 2 * b];
		uint64_t lower = ZOBRISTS[start_square * 12 + 9 + 2 * b];
		zobrist ^= (upper << size) | (lower >> (64 - size));
		bordered_bitboards[top_stones[start_square].to_int()] |= (1ULL << start_square); // add piece to bitboard
	}

	else { // undo placement
		// remove stone
		int square = m.square_idx();
		Piece p = top_stones[square];
		zobrist ^= ZOBRISTS[square * 12 + p.to_int()];
		bordered_bitboards[p] &= ~(1ULL << square); // remove piece from bitboard
		top_stones[square] = Piece::NONE;
		bordered_bitboards[((Piece)Piece::NONE).to_int()] |= (1ULL << square); // add empty square to bitboard
		stack_sizes[square] = 0;

		// adjust reserves
		if (m.is_cap_placement()) {
			if (current_player == 1) // white
				w_cap_placed = false;
			else
				b_cap_placed = false;
		}
		// inaccurate for first move, but correct after
		else {
			if (current_player == 1) // white
				w_reserves++;
			else
				b_reserves++;
		}
	}

	// clear move list for this move
	if (move_lists[move_count+1] != nullptr) {
		move_lists[move_count+1]->clear();
	}
}

bool TakBoard::is_legal(move_t m)
{
	// TODO implement legality checks
	// for now, assume all moves are legal
	return true;
}

MoveList* TakBoard::get_legal_moves()
{
	MoveList* move_iter = move_lists[move_count];
	if (move_iter == nullptr) {
		move_iter = new MoveList();
		move_lists[move_count] = move_iter;
	}

	// TODO partial move generation
	if (move_iter->is_empty()) {
		generate_moves(move_iter);
	}
	else if (!move_iter->has_next()) {
		move_iter->reset();
	}

	return move_iter;
}

bool TakBoard::is_swap() const
{
	return move_count < 2;
}

bitboard_t TakBoard::get_bordered_bitboard(Piece type) const
{
	return bordered_bitboards[type.to_int()] & BORDER_MASK;
}

void TakBoard::generate_moves(MoveList* move_list)
{
	for (uint8_t col = 0; col < 6; col++) {
		for (uint8_t row = 0; row < 6; row++) {
			uint8_t square_idx = row * 8 + col; // 6x6 board, 8x8 indexing for padded bitboards

			if (top_stones[square_idx] == Piece::NONE) { // add placements
				uint32_t reserves = (current_player == PLAYER_WHITE) ? w_reserves : b_reserves;
				bool capstone_placed = (current_player == PLAYER_WHITE) ? w_cap_placed : b_cap_placed;

				if (reserves > 0) { // can place a flat/wall
					// flat placement
					move_list->add_move({ (uint8_t)(square_idx | 0b01000000U), 0 }); // flat
					// wall_placement
					if (!is_swap()) {
						move_list->add_move({ (uint8_t)(square_idx | 0b10000000U), 0 }); // wall
					}
				}
				if (!is_swap() && !capstone_placed) { // can place a capstone
					move_list->add_move({ (uint8_t)(square_idx | 0b11000000U), 0 }); // capstone
				}
			}
			else if (!is_swap() && top_stones[square_idx].get_player() == current_player) { // add spreads
				bool is_capstone = top_stones[square_idx].is_capstone();
				uint8_t stack_size = stack_sizes[square_idx];

				bitboard_t walls = get_bordered_bitboard(Piece::W_WALL) | get_bordered_bitboard(Piece::B_WALL);
				bitboard_t capstones = get_bordered_bitboard(Piece::W_CAP) | get_bordered_bitboard(Piece::B_CAP);

				SpreadIterator spread_iter = Magic::get_spread_iterator(square_idx, walls, capstones, stack_size);

				// TODO add the iterator to the move list instead of copying all moves
				if (spread_iter.has_next()) {
					move_list->add_spread(spread_iter);
				}
			}
		}
	}
}
