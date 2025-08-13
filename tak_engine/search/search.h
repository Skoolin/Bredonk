#pragma once

#include "../tak/tak_board.h"
#include "zobrist.h"
#include <iomanip>
#include <chrono>
#include <thread>

class Searcher {
public:
	Searcher(bool verbose = false);
	void search(TakBoard& board, int depth);
	void stop();

private:
	bool command_stop;
	constexpr static uint64_t MAX_DEPTH = 64;
	constexpr static int16_t MAX_SCORE = 32000;
	void iterative_deepening(TakBoard& board, int max_depth);
	int16_t alpha_beta(TakBoard& board, int depth, int16_t alpha, int16_t beta, bool root);
	HashTable table;
	move_t result_move;

	struct search_stats_t {
		std::chrono::system_clock::time_point start;
		std::string result_move;
		uint64_t count;
		uint64_t node_count;
		uint64_t depth_count[MAX_DEPTH];
		uint64_t tt_count;
		uint64_t tt_res;
		uint64_t tt_beta;
		uint64_t tt_beats_alpha;
		uint64_t pv_count;
		uint64_t alpha_count;
		uint64_t beta_count;
		uint64_t first_beats_alpha[64];
		uint64_t pv_place;
		uint64_t pv_spread;
		uint64_t alpha_place;
		uint64_t alpha_spread;
		uint64_t beta_top[64];
		uint64_t beta_place;
		uint64_t beta_spread;

		bool verbose = false;

		void reset();

		void print(int16_t eval = 0) const;
	};

	search_stats_t stats;
	std::thread search_thread;
};
