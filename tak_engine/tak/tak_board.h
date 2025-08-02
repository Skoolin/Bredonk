#include <inttypes.h>
#include <bit>

#include "../params.h"
#include "move.h"
#include "move_iterator.h"
#include "move_list.h"
#include "piece.h"
#include "bitboard.h"

#pragma once

/*
* Board representation for the game of Tak. Current implementation for 6x6 only.
* It maintains the current state of the game and allows moves to be made, undone, and checked for legality.
* It also provides functionality to check if the game is in a final state and to get the result of the game.
*/
class TakBoard {
public:
	TakBoard();
	~TakBoard();

	std::string get_tps() const;

	bool is_final() const;
	int32_t get_result() const; // 0 if draw, 1 if white wins, -1 if black wins

	uint64_t get_hash();

	void make_move(move_t m);
	void undo_move(move_t m);
	bool is_legal(move_t m);
	MoveList* get_legal_moves();
	bool is_swap() const;

	bitboard_t get_bordered_bitboard(Piece type) const;

	static constexpr bitboard_t BORDER_MASK = 0x00FDFDFDFDFDFD00ULL; // bordered layout: (1 = board, 0 = border)
	static constexpr int offsets[4] = {
		+8, // up
		+1, // right
		-8, // down
		-1, // left
	};

private:
	void generate_moves(MoveList* move_list);

	int32_t state;
	int32_t current_player; // white = 1, black = -1
	uint32_t w_reserves;
	uint32_t b_reserves;
	bool w_cap_placed;
	bool b_cap_placed;

	int32_t move_count;
	move_t previous_moves[MAX_GAME_LENGTH];
	MoveList* move_lists[MAX_GAME_LENGTH];
	bool did_flatten[MAX_GAME_LENGTH];

	uint32_t zobrist;
	Piece top_stones[64];
	uint8_t stack_sizes[64];
	Piece stacks[64][MAX_STACK_HEIGHT];
	bitboard_t bordered_bitboards[8]; // NONE, W_FLAT, W_WALL, W_CAP, __ILLEGAL__, B_FLAT, B_WALL, B_CAP
};
