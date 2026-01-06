#include "search.h"

Searcher::Searcher(bool verbose)
	: table(),
	stats(),
	result_move(move_t::ILLEGAL),
	command_stop(true)
{
	stats.verbose = false;
}

void Searcher::search(TakBoard& board, int depth)
{
	if (search_thread.joinable()) {
		command_stop = true;
		search_thread.join();
	}
	command_stop = false;
	search_thread = std::thread(&Searcher::iterative_deepening, this, std::ref(board), depth);
}

void Searcher::stop()
{
	command_stop = true;
	if (search_thread.joinable())
		search_thread.join();
}

void Searcher::iterative_deepening(TakBoard& board, int max_depth)
{
	// TODO aspiration windows
	stats.reset();
	for (int i = 2; i <= max_depth; i++) {
		auto result = alpha_beta(board, i, -MAX_SCORE, MAX_SCORE, true);
		stats.print(result);
		if (command_stop)
			break;
	}
	std::cout << "bestmove " << result_move.get_ptn() << std::endl;
}

int16_t Searcher::alpha_beta(TakBoard& board, int depth, int16_t alpha, int16_t beta, bool root = false)
{
	if (command_stop)
		return 0;
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
		else {
			tt_move = move_t::ILLEGAL;
		}
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

		// debug: compare zobrist and accums before and after

		auto debug_zobrist = board.get_hash();
		auto debug_accum = board.incremental.get_sample_accum();

		board.make_move(move);

		int16_t new_score = -alpha_beta(board, depth - 1, -beta, -alpha);

		board.undo_move(move);

		if (debug_zobrist != board.get_hash()) {
			std::cout << board.get_tps() << std::endl;
			std::cout << move.get_ptn() << std::endl;
			std::cout.flush();
			throw std::runtime_error("zobrist changed after move/unmove!!!");
		}
		if (debug_accum != board.incremental.get_sample_accum()) {
			std::cout << board.get_tps() << std::endl;
			std::cout << move.get_ptn() << std::endl;
			std::cout.flush();
			throw std::runtime_error("incremental accumulator inconsistency after move/unmove!!!");
		}

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
	if (root) {
		result_move = best_move;
		stats.result_move = result_move.get_ptn();
	}
	return best_score;
}

void Searcher::search_stats_t::reset() {
	start = std::chrono::system_clock::now();
	count = 0;
	node_count = 0;
	for (int i = 0; i < MAX_DEPTH; i++)
		depth_count[i] = 0;
	tt_count = 0;
	tt_res = 0;
	tt_beta = 0;
	tt_beats_alpha = 0;
	pv_count = 0;
	alpha_count = 0;
	beta_count = 0;
	for (int i = 0; i < 64; i++) {
		beta_top[i] = 0;
		first_beats_alpha[i] = 0;
	}
	pv_place = 0;
	pv_spread = 0;
	alpha_place = 0;
	alpha_spread = 0;
	beta_place = 0;
	beta_spread = 0;
}

void Searcher::search_stats_t::print(int16_t eval) const
{
	auto end_time = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start);
	auto nps = (count * 1000.0 / duration.count());

	if (verbose)
	{
		std::cout.setf(std::ios::fixed);
		std::cout << std::setprecision(5);
		std::cout
			<< "Search Stats: " << std::endl
			<< "  Nodes: total " << count << ", nps: " << std::fixed << nps << std::endl
			<< "  after tt: " << node_count
			<< ", exact " << pv_count << " (" << (100.0 * (double)pv_count / (double)node_count)
			<< "%), alpha " << alpha_count << " (" << (100.0 * (double)alpha_count / (double)node_count)
			<< "%), beta " << beta_count << " (" << (100.0 * (double)beta_count / (double)node_count) << "%)" << std::endl
			<< "  Spreads: exact " << ((100.0 * (double)pv_spread) / (double)pv_count)
			<< "%, alpha " << ((100.0 * (double)alpha_spread) / (double)alpha_count)
			<< "%, beta " << ((100.0 * (double)beta_spread) / (double)beta_count) << "%" << std::endl
			<< "  Beta Cut Move IDX: " << std::endl
			;
		for (int i = 0; i < 63; i++)
			std::cout << std::setfill(' ') << std::setw(6) << i << ": " << beta_top[i] << " (" << ((100.0 * (double)beta_top[i]) / (double)beta_count) << "%)" << std::endl;
		std::cout << "   63+: " << beta_top[63] << " (" << ((100.0 * (double)beta_top[63]) / (double)beta_count) << "%)" << std::endl;
		std::cout << "  First Non-Alpha Move IDX:" << std::endl;
		for (int i = 0; i < 63; i++)
			std::cout << std::setfill(' ') << std::setw(6) << i << ": " << first_beats_alpha[i] << " (" << ((100.0 * (double)first_beats_alpha[i]) / (double)(beta_count + pv_count)) << "%)" << std::endl;
		std::cout << "   63+: " << beta_top[63] << " (" << ((100.0 * (double)beta_top[63]) / (double)(beta_count + pv_count)) << "%)" << std::endl;

		std::cout
			<< "TT Stats:" << std::endl
			<< "  TT hits: " << tt_count << " (" << ((100.0 * (double)tt_count) / (double)(node_count + tt_beta + tt_res)) << "%)" << std::endl
			<< "  TT eval: " << tt_res << " (" << ((100.0 * (double)tt_res) / (double)(tt_count)) << "%)" << std::endl
			<< "  TT beta: " << tt_beta << " " << ((100.0 * (double)tt_beta) / (double)tt_count) << "%)" << std::endl
			;
	}

	std::cout << "info cp " << eval << " pv " << result_move << " time " << duration.count() << " nodes " << count << " nps " << ((uint64_t) nps) << std::endl;
}
