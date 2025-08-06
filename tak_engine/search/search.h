#pragma once

#include "../tak/tak_board.h"
#include "zobrist.h"

class Searcher {
public:
	Searcher();
	move_t search(TakBoard& board, int depth);

private:
	int16_t alpha_beta(TakBoard& board, int depth, int16_t alpha, int16_t beta, bool root);
	int16_t eval(TakBoard& board);
	constexpr static int16_t MAX_SCORE = 3000;
	HashTable table;
	uint64_t count;
	move_t result_move;
};
