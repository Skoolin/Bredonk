#pragma once

#include "../tak/tak_board.h"

uint64_t perft(TakBoard* board, int depth) {
	// Base case: if depth is 0 or the board is in a final state, return 1 (leaf nodes)
	if (depth == 0 || board->is_final()) {
		return 1;
	}

	uint64_t nodes = 0;
	MoveList* moves = board->get_legal_moves();

	if (depth == 1) {
//		return moves->size(); // If depth is 1, return the number of legal moves
	}

	while (moves->has_next()) {
		move_t move = moves->next();
		board->make_move(move);
		nodes += perft(board, depth - 1);
		board->undo_move(move);
	}

	return nodes;
}
