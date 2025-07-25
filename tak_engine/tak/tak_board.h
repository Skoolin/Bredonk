#include <inttypes.h>
#include <bit>

#include "../params.h"
#include "move.h"
#include "move_iterator.h"
#include "piece.h"

#pragma once
class TakBoard {
public:
	TakBoard();
	bool is_final();
	int32_t get_result(); // 0 if draw, 1 if white wins, -1 if black wins

	uint32_t get_hash();

	void make_move(move_t m);
	void undo_move();
	bool is_legal(move_t m);
	MoveIterator get_legal_moves();
	
	static const int32_t PLAYER_WHITE = 1;
	static const int32_t PLAYER_BLACK = -1;

	static const int32_t STATE_ONGOING = 2;
	static const int32_t STATE_DRAW = 0;
	static const int32_t STATE_BLACK_WIN = -1;
	static const int32_t STATE_WHITE_WIN = 1;
	static const int32_t STATE_ILLEGAL = 3;

	uint64_t get_bitmap(Piece type);
	uint64_t get_bordered_bitmap(Piece type);

private:
	int32_t current_player; // white = 1, black = -1
	int32_t move_count;
	uint32_t zobrist;
	Piece top_stones[36];
	uint8_t stack_sizes[36];
	Piece stacks[36][MAX_STACK_HEIGHT];
	move_t previous_moves[MAX_GAME_LENGTH];
	MoveIterator move_lists[MAX_GAME_LENGTH];
	int32_t state;
	uint32_t w_reserves;
	uint32_t b_reserves;
	bool w_cap_placed;
	bool b_cap_placed;

};