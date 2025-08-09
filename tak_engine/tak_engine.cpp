// tak_engine.cpp : Defines the entry point for the application.
//

#include "tak_engine.h"
#include "search/perft.h"
#include "search/search.h"
#include "tak/eval.h"
#include <chrono>

#include "tak/magic.h"

int start()
{
	run_perft(5);
	return 0;
}

int from_pos()
{
	int depth = 2;
	TakBoard* board;

//	move_t moves[] = { move_t::from_ptn("a1"), move_t::from_ptn("f6"), { (uint8_t)((2U << 6) | 45U), 0b00000001U }};
	move_t moves[] = { move_t::from_ptn("b3"), move_t::from_ptn("a2"), move_t::from_ptn("Ca1") };

	// Reset the board to the initial state
	board = new TakBoard();

	for (move_t move : moves) {
		board->make_move(move);
	}

	// start timer
	auto start = std::chrono::high_resolution_clock::now();
	uint64_t nodes = perft(board, depth, true);
	// stop timer
	auto end = std::chrono::high_resolution_clock::now();
	// print perft result
	std::cout << "Perft at depth " << depth << ": " << nodes << " nodes." << std::endl;
	// print nodes per second
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Time taken: " << duration.count() << " ms." << std::endl;
	std::cout << "Nodes per second: " << std::fixed << (nodes * 1000.0 / duration.count()) << std::endl;

	delete board;

	return 0;
}

void init() {
	Eval::init("nnue.bin");
}

int main() {
	init();

	Searcher* s = new Searcher();
	TakBoard b = TakBoard();
	auto res = s->search(b, 6);
	std::cout << res.get_ptn() << std::endl;
}
