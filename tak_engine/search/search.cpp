#include "search.h"

Searcher::Searcher()
	: table(),
	stats()
{
}

move_t Searcher::search(TakBoard& board, int depth)
{
	stats.reset();
	for (int i = 2; i <= depth; i++) {
		alpha_beta(board, i, -MAX_SCORE, MAX_SCORE, true);
	}

	stats.print();
	return result_move;
}

int16_t Searcher::alpha_beta(TakBoard& board, int depth, int16_t alpha, int16_t beta, bool root = false)
{
	stats.count++;
	if (board.is_final()) {
		auto result = board.get_result() * MAX_SCORE;
		return result - 2 * result * board.current_player;
	}
	if (depth <= 0) {
		auto result = board.get_eval();
		return result - 2 * result * board.current_player;
	}

	int16_t best_score = -MAX_SCORE;
	move_t best_move{};

	auto entry = table.get(board.get_hash());
	auto tt_move = move_t::ILLEGAL;

	if (!root && entry.is_valid()) {
		stats.tt_count++;
		if (entry.depth >= depth) {
			if (entry.exact()
				|| (entry.beta() && entry.eval > beta)
				|| (entry.alpha() && entry.eval <= alpha)
				) {
				stats.tt_res++;
				return entry.eval;
			}
		}

		/*
		// search tt move first
		tt_move = entry.move;
		if (board.is_legal(tt_move)) {
			board.make_move(tt_move);
			int16_t new_score = -alpha_beta(board, depth - 1, -beta, -alpha);
			board.undo_move(tt_move);
			if (new_score > beta) {
				stats.tt_beta++;
				return new_score;
			}
			if (new_score > best_score) {
				best_score = new_score;
			}
			if (new_score > alpha) {
				stats.tt_beats_alpha++;
				alpha = new_score;
			}
		}
		*/

	}

	stats.node_count++;

	bool raised_alpha = false;

	auto moves = board.get_legal_moves();
	int move_idx = -1;
	while (moves->has_next()) {
		move_t move = moves->next();
		move_idx++;

		// don't search tt move again!
		if (move == tt_move)
			continue;

		board.make_move(move);
		int16_t new_score = -alpha_beta(board, depth - 1, -beta, -alpha);
		board.undo_move(move);
		if (new_score > beta) {
			stats.beta_count++;
			stats.beta_top[move_idx > 63 ? 63 : move_idx]++;
			if (alpha >= best_score)
				stats.first_beats_alpha[move_idx > 63 ? 63 : move_idx]++;
			table.update(board.zobrist, board.move_count, depth, move, new_score, false, true);
			return new_score;
		}
		if (new_score > alpha) {
			raised_alpha = true;
			if (alpha >= best_score)
				stats.first_beats_alpha[move_idx > 63 ? 63 : move_idx]++;
			alpha = new_score;
		}
		if (new_score > best_score) {
			best_score = new_score;
			best_move = move;
		}
	}
	stats.pv_count += raised_alpha;
	stats.alpha_count += !raised_alpha;
	table.update(board.zobrist, board.move_count, depth, best_move, best_score, !raised_alpha, false);
	result_move = best_move;
	return best_score;
}
