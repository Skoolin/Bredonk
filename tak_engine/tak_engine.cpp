// tak_engine.cpp : Defines the entry point for the application.
//

#include "tak_engine.h"
#include "search/perft.h"
#include <chrono>

#include "tak/magic.h"

int main()
{
	int depth = 4;
	TakBoard* board;

	move_t moves[] = { move_t::from_ptn("a1"), move_t::from_ptn("f6") };

	// Reset the board to the initial state
	board = new TakBoard();

	for (move_t move : moves) {
		board->make_move(move);
		// start timer
		auto start = std::chrono::high_resolution_clock::now();
//		uint64_t nodes = perft(board, depth, true);
		// stop timer
		auto end = std::chrono::high_resolution_clock::now();
		// print perft result
//		std::cout << "Perft at depth " << depth << ": " << nodes << " nodes." << std::endl;
		// print nodes per second
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::cout << "Time taken: " << duration.count() << " ms." << std::endl;
//		std::cout << "Nodes per second: " << std::fixed << (nodes * 1000.0 / duration.count()) << std::endl;
	}

	// start timer
	auto start = std::chrono::high_resolution_clock::now();
	uint64_t nodes = perft(board, 4, true);
	nodes = perft(board, 4, true);
	// stop timer
	auto end = std::chrono::high_resolution_clock::now();
	// print perft result
	std::cout << "Perft at depth " << 5 << ": " << nodes << " nodes." << std::endl;
	// print nodes per second
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Time taken: " << duration.count() << " ms." << std::endl;
	std::cout << "Nodes per second: " << std::fixed << (nodes * 1000.0 / duration.count()) << std::endl;

	delete board;

	return 0;
}
