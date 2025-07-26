#include "tak_board.h"
#include "../params.h"

TakBoard::TakBoard() :
	top_stones{ Piece::NONE },
	stack_sizes{ 0 },
	stacks{ Piece::NONE },

	previous_moves{ 0, 0 },
	move_count(0),
	move_lists{  },
	did_flatten{ false },

	state(STATE_ONGOING),
	current_player(PLAYER_WHITE),
	zobrist(0),

	w_reserves(30),
	b_reserves(30),
	w_cap_placed(false),
	b_cap_placed(false)
{

}

/*
* flood fill search on bit mask
*/
bool has_road(uint64_t bitmap) {
	// bordered layout: (1 = board, 0 = border)
	// 00000000
	// 11111100
	// 11111100
	// 11111100
	// 11111100
	// 11111100
	// 11111100
	// 00000000

	// TODO early stopping if road already found even if floodfill incomplete
	// TODO test loop unrolling

	const uint64_t LEFT = 0b000000001000000010000000100000001000000010000000100000000000000000000000;
	const uint64_t RIGHT = 0b000000000000000100000001000000010000000100000001000000010000000000000000;
	const uint64_t TOP = 0b000000001111110000000000000000000000000000000000000000000000000000000000;
	const uint64_t BOTTOM = 0b000000000000000000000000000000000000000000000000000000001111110000000000;

	// search left to right
	uint64_t fill_map = 0;
	uint64_t step_map = LEFT & bitmap;
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

bool TakBoard::is_final()
{
	// check reserves
	if (w_reserves == 0 && w_cap_placed)
		return true;
	if (b_reserves == 0 && b_cap_placed)
		return true;

	// check board fill
	uint64_t empty_bitmap = get_bitmap(Piece::NONE);
	if (empty_bitmap == 0)
		return true;

	// check for roads
	uint64_t w_bitmap = get_bordered_bitmap(Piece::W_CAP) | get_bordered_bitmap(Piece::W_FLAT);
	uint64_t b_bitmap = get_bordered_bitmap(Piece::B_CAP) | get_bordered_bitmap(Piece::B_FLAT);
	
	if (has_road(w_bitmap))
		return true;
	return has_road(b_bitmap);
}

int32_t TakBoard::get_result()
{
	if (!is_final())
		return STATE_ONGOING;

	// check for roads
	uint64_t w_bitmap = get_bordered_bitmap(Piece::W_CAP) | get_bordered_bitmap(Piece::W_FLAT);
	uint64_t b_bitmap = get_bordered_bitmap(Piece::B_CAP) | get_bordered_bitmap(Piece::B_FLAT);

	bool w_road = has_road(w_bitmap);
	bool b_road = has_road(b_bitmap);

	if (current_player == PLAYER_WHITE && w_road)
		return STATE_WHITE_WIN;
	if (current_player == PLAYER_BLACK && b_road)
		return STATE_BLACK_WIN;
	if (w_road)
		return STATE_WHITE_WIN;
	if (b_road)
		return STATE_BLACK_WIN;

	int w_count = std::popcount(get_bitmap(Piece::W_FLAT));
	int b_count = std::popcount(get_bitmap(Piece::B_FLAT));

	if (w_count > b_count)
		return STATE_WHITE_WIN;
	if (b_count > w_count)
		return STATE_BLACK_WIN;

	return STATE_DRAW;
}

int32_t get_player(Piece p) {
	if (p == 0)
		return 0;
	return (p > 0) ? 1 : -1;
}

void TakBoard::make_move(move_t m)
{
	previous_moves[move_count] = m;
	move_count++;

	// TODO update zobrist

	if (m.is_spread()) { // spread move
		int start_square = m.square_idx();
		int direction = m.spread_direction();
		int square_offset = offsets[direction];

		// picked up stones
		Piece stones[8];

		uint8_t perm = m.spread_perm;
		// count how many captives need to be picked up
		int trailing_zeros = std::countr_zero(perm); // count trailing zeros in spread_perm
		int picked_up = 8 - trailing_zeros; // 8 is the max number of pieces in a move representation
		
		perm >>= trailing_zeros; // remove trailing zeros from perm
		
		// pick up stones

		// top stone:
		stones[0] = top_stones[start_square];
		stack_sizes[start_square]--;

		// rest of stack:
		for (int i = 1; i < picked_up; i++) {
			stack_sizes[start_square]--;
			stones[i] = stacks[start_square][stack_sizes[start_square]];
		}

		// fix top stone
		if (stack_sizes[start_square])
			top_stones[start_square] = stacks[start_square][stack_sizes[start_square] - 1];
		else
			top_stones[start_square] = Piece::NONE;

		// put down stones
		int current_square = start_square;
		for (int i = 0; i < picked_up; i++) {
			Piece stone = stones[picked_up - i - 1];
			if (perm & 0b1) { // move to next square
				current_square += square_offset;
				// smash top wall
				if (stone == Piece::W_CAP || stone == Piece::B_CAP) {
					if (is_wall(top_stones[current_square])) {
						did_flatten[move_count - 1] = true;
						if (top_stones[current_square] == Piece::B_WALL) {
							top_stones[current_square] = Piece::B_FLAT;
						}
						if (top_stones[current_square] == Piece::W_WALL) {
							top_stones[current_square] = Piece::W_FLAT;
						}
					}
					else {
						did_flatten[move_count - 1] = false;
					}
				}
			}
			if (stack_sizes[current_square])
				stacks[current_square][stack_sizes[current_square] - 1] = top_stones[current_square];
			top_stones[current_square] = stone;
			stack_sizes[current_square]++;
			perm >>= 1; // shift to next stone
		}
	}
	else { // placement move
		int square = m.square_idx();
		Piece p = m.piece_type(current_player);

		top_stones[square] = p;
		stack_sizes[square] = 1;

		// adjust reserves
		if (m.is_cap_placement()) {
			if (current_player == 1) // white
				w_cap_placed = true;
			else
				b_cap_placed = true;
		}
		else {
			if (current_player == 1) // white
				w_reserves--;
			else
				b_reserves--;
		}
	}

	current_player = -current_player;
}

void TakBoard::undo_move()
{
	// TODO update zobrist

	move_count--;
	move_t m = previous_moves[move_count];
	current_player = -current_player;

	if (m.is_spread()) { // undo spread move
		int start_square = m.square_idx();
		int direction = m.spread_direction();
		int square_offset = offsets[direction];
		int distance = m.spread_distance();

		uint8_t perm = m.spread_perm;
		int trailing_zeros = std::countr_zero(perm); // count trailing zeros in spread_perm
		int pieces_to_put_back = 8 - trailing_zeros; // 8 is the max number of pieces in a move representation
		perm >>= trailing_zeros; // remove trailing zeros from perm

		// TODO this can be done directly on the square stack!
		Piece stones[8];

		// pick up stones
		int current_square = start_square + square_offset * distance;
		bool is_last_square = true;

		for (int i = pieces_to_put_back - 1; i >= 0; i--) {
			Piece stone = top_stones[current_square];
			stones[i] = stone;

			stack_sizes[current_square]--;
			if (stack_sizes[current_square] > 0) {
				top_stones[current_square] = stacks[current_square][stack_sizes[current_square] - 1];
			}
			else {
				top_stones[current_square] = Piece::NONE;
			}
			if (perm & 0b1) {
				// undo flatten
				if (is_last_square && is_capstone(stone) && did_flatten[move_count]) {
					Piece top_stone = top_stones[current_square];
					if (top_stone == Piece::B_FLAT) {
						top_stones[current_square] = Piece::B_WALL;
					}
					else if (top_stone == Piece::W_FLAT) {
						top_stones[current_square] = Piece::W_WALL;
					}
				}
				// move to previous square
				current_square -= square_offset;
				is_last_square = false;
			}
			perm >>= 1; // shift to next stone
		}

		// put back stones
		stacks[start_square][stack_sizes[start_square]] = top_stones[start_square];
		for (int i = 0; i < pieces_to_put_back; i++) {
			stacks[start_square][stack_sizes[start_square] + i + 1] = stones[i];
		}
		stack_sizes[start_square] += pieces_to_put_back;
		// adjust top stone
		top_stones[start_square] = stacks[start_square][stack_sizes[start_square] - 1];
	}

	else { // undo placement
		// remove stone
		int square = m.square_idx();
		top_stones[square] = Piece::NONE;
		stack_sizes[square] = 0;

		// adjust reserves
		if (m.is_cap_placement()) {
			if (current_player == 1) // white
				w_cap_placed = false;
			else
				b_cap_placed = false;
		}
		else {
			if (current_player == 1) // white
				w_reserves++;
			else
				b_reserves++;
		}
	}
}

uint64_t TakBoard::get_bitmap(Piece type)
{
	// TODO internal board representation with bitmaps
	uint64_t result = 0;
	for (int i = 0; i < 36; i++) {
		if (top_stones[i] == type)
			result |= (uint64_t) 1 << i;
	}
	return result;
}

uint64_t TakBoard::get_bordered_bitmap(Piece type)
{
	// bordered layout: (1 = board, 0 = border)
	// 00000000
	// 11111100
	// 11111100
	// 11111100
	// 11111100
	// 11111100
	// 11111100
	// 00000000

	uint64_t result = 0;
	int shift = 10;
	for (int i = 0; i < 36; i+=6) {
		for (int j = i; j < i + 6; j++) {
			if(top_stones[j] == type)
				result |= (uint64_t) 1 << shift;
			shift += 1;
		}
		shift += 2;
	}
	return result;
}