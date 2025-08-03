#pragma once

#include "../tak/tak_board.h"
#include <chrono>

uint64_t perft(TakBoard *board, int depth, bool verbose = false) {

	// Base case: if depth is 0 or the board is in a final state, return 1 (leaf nodes)
	// check state first for more accurate nps
	if (depth <= 0)
		return 1;
	if (board->is_final())
		return 0; // to match tiltak, but should be 1 no?

	//	std::cout << "tps: " << board->get_tps() << std::endl;

	uint64_t nodes = 0;
	MoveList* moves = board->get_legal_moves();

	if (depth == 1) {
		return moves->size(); // If depth is 1, return the number of legal moves
	}

	if (verbose)
		std::cout << "tps: " << board->get_tps() << std::endl;

	while (moves->has_next()) {
//		auto tps_before_make = board->get_tps();
		move_t move = moves->next();
		board->make_move(move);
//		auto tps_before_perft = board->get_tps();
		auto perft_nodes = perft(board, depth - 1);
//		auto tps_after_perft = board->get_tps();
		nodes += perft_nodes;
		board->undo_move(move);
//		auto tps_after_undo = board->get_tps();
//		if (tps_before_make != tps_after_undo) {
//			std::cout << "ERROR: make/undo move changed tps!!" << std::endl;
//			std::cout << ">> tps before make:  " << tps_before_make << std::endl;
//			std::cout << ">> tps before perft: " << tps_before_perft << std::endl;
//			std::cout << ">> tps after perft:  " << tps_after_perft << std::endl;
//			std::cout << ">> tps after undo:   " << tps_after_undo << std::endl;
//			std::cout << ">> move: " << move.get_ptn() << ", depth: " << depth << std::endl;
//			std::cout << ">> dir: " << move.spread_direction() << ", perm: " << (unsigned int)move.spread_perm << std::endl;;
//			break;
//		}
		if (verbose) {
			std::cout << "move " << move.get_ptn() << ": " << perft_nodes << std::endl;
		}
	}

	return nodes;
}

void run_perft(int max_depth) {
	std::cout << "perft for position " << TakBoard().get_tps() << std::endl;
	for (int depth = 0; depth <= max_depth; depth++) {
		TakBoard board = TakBoard();
		auto start_time = std::chrono::high_resolution_clock::now();
		auto nodes = perft(&board, depth);
		auto end_time = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
		std::cout << "depth " << depth << ": " << nodes << " (" << std::fixed << (nodes * 1000.0 / duration.count()) << " nps)" << std::endl;
		board.~TakBoard();
	}
}
