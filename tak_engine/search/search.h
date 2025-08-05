#pragma once

#include "../tak/tak_board.h"

class Searcher {
public:
	Searcher();
	move_t search(TakBoard& board, int depth);

private:
	int16_t alpha_beta(TakBoard& board, int depth, int16_t alpha, int16_t beta);
	int16_t eval(TakBoard& board);
	constexpr static int16_t MAX_SCORE = 3000;
	uint64_t count;
};
