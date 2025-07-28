// tak_engine.cpp : Defines the entry point for the application.
//

#include "tak_engine.h"
#include "search/perft.h"
#include <chrono>

#include "tak/magic.h"

int main()
{
	Magic::init();

	TakBoard board;

	for (int depth = 1; depth < 8; depth++) {
		// Reset the board to the initial state
		board = TakBoard();

		// start timer
		auto start = std::chrono::high_resolution_clock::now();
		int nodes = perft(board, depth);
		// stop timer
		auto end = std::chrono::high_resolution_clock::now();
		// print perft result
		std::cout << "Perft at depth " << depth << ": " << nodes << " nodes." << std::endl;
		// print nodes per second
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::cout << "Time taken: " << duration.count() << " ms." << std::endl;
		std::cout << "Nodes per second: " << std::fixed << (nodes * 1000.0 / duration.count()) << std::endl;
	}

	return 0;
}
