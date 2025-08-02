#pragma once

#include "../tak/tak_board.h"

uint64_t perft(TakBoard* board, int depth, bool verbose = false) {
	// Base case: if depth is 0 or the board is in a final state, return 1 (leaf nodes)
	// check state first for more accurate nps
	if (depth <= 0 || board->is_final()) {
		return 1;
	}

	uint64_t nodes = 0;
	MoveList* moves = board->get_legal_moves();

	if (depth == 1) {
//		return moves->size(); // If depth is 1, return the number of legal moves
	}

	if (verbose)
		std::cout << "tps: " << board->get_tps() << std::endl;

	while (moves->has_next()) {
		move_t move = moves->next();
		board->make_move(move);
		auto perft_nodes = perft(board, depth - 1);
		nodes += perft_nodes;
		board->undo_move(move);
		if (verbose) {
			std::cout << "move " << move.get_ptn() << ": " << perft_nodes << std::endl;
		}
	}

	return nodes;
}
