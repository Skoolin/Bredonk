#include "search.h"

Searcher::Searcher()
	: count(0), table()
{

}

move_t Searcher::search(TakBoard& board, int depth)
{
	count = 0;
	for (int i = 2; i <= depth; i++) {
		alpha_beta(board, i, -MAX_SCORE, MAX_SCORE, true);
	}
	std::cout << "found within " << count << " nodes" << std::endl;
	return result_move;
}

int16_t Searcher::alpha_beta(TakBoard& board, int depth, int16_t alpha, int16_t beta, bool root = false)
{
	count++;
	if (board.is_final()) {
		return board.current_player * board.get_result() * MAX_SCORE;
	}
	if (depth <= 0) {
		return board.current_player * eval(board);
	}

	int16_t best_score = -MAX_SCORE;
	move_t best_move;

	auto entry = table.get(board.get_hash());

	if (!root && entry.is_valid()) {
		if (entry.depth >= depth) {
			if (entry.exact()
				|| (entry.beta() && entry.eval > beta)
				)
				return entry.eval;
		}

		move_t move = entry.move;
		board.make_move(move);
		int16_t new_score = -alpha_beta(board, depth - 1, -beta, -alpha);
		board.undo_move(move);
		if (new_score > beta) return new_score;
		if (new_score > best_score) best_score = new_score;
		if (new_score > alpha) alpha = new_score;
	}

	auto moves = board.get_legal_moves();
	while (moves->has_next()) {
		move_t move = moves->next();
		board.make_move(move);
		int16_t new_score = -alpha_beta(board, depth - 1, -beta, -alpha);
		board.undo_move(move);
		if (new_score > beta) {
			table.update(board.zobrist, board.move_count, depth, move, new_score, false, true);
			return new_score;
		}
		if (new_score > best_score) {
			best_score = new_score;
			best_move = move;
		}
		if (new_score > alpha) alpha = new_score;
	}
	table.update(board.zobrist, board.move_count, depth, best_move, best_score, alpha > best_score, false);
	result_move = best_move;
	return best_score;
}

int16_t Searcher::eval(TakBoard& board)
{
	return board.bordered_bitboards[Piece::W_FLAT].count() - board.bordered_bitboards[Piece::B_FLAT].count();
}
