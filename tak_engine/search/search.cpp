#include "search.h"

Searcher::Searcher()
{
}

move_t Searcher::search(TakBoard& board, int depth)
{
	auto moves = board.get_legal_moves();

	int16_t best_score = -MAX_SCORE;
	move_t best_move = {};

	count = 0;

	while (moves->has_next()) {
		move_t move = moves->next();
		board.make_move(move);
		int16_t new_score = -alpha_beta(board, depth - 1, -MAX_SCORE, -best_score);
		board.undo_move(move);
		if (new_score > best_score) {
			best_score = new_score;
			best_move = move;
		}
	}

	std::cout << "found within " << count << " nodes" << std::endl;

	return best_move;
}

int16_t Searcher::alpha_beta(TakBoard& board, int depth, int16_t alpha, int16_t beta)
{
	count++;
	if (board.is_final()) {
		return board.current_player * board.get_result() * MAX_SCORE;
	}
	if (depth <= 0) {
		return board.current_player * eval(board);
	}

	auto moves = board.get_legal_moves();
	int16_t best_score = -MAX_SCORE;
	while (moves->has_next()) {
		move_t move = moves->next();
		board.make_move(move);
		int16_t new_score = -alpha_beta(board, depth - 1, -beta, -alpha);
		board.undo_move(move);
		if (new_score > beta) return new_score;
		if (new_score > best_score) best_score = new_score;
		if (new_score > alpha) alpha = new_score;
	}
	return best_score;
}

int16_t Searcher::eval(TakBoard& board)
{
	return board.bordered_bitboards[Piece::W_FLAT].count() - board.bordered_bitboards[Piece::B_FLAT].count();
}
