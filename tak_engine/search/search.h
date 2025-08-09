#pragma once

#include "../tak/tak_board.h"
#include "zobrist.h"
#include <iomanip>

class Searcher {
public:
Searcher();
	move_t search(TakBoard& board, int depth);

private:
	constexpr static uint64_t MAX_DEPTH = 64;
	constexpr static int16_t MAX_SCORE = 32000;
	int16_t alpha_beta(TakBoard& board, int depth, int16_t alpha, int16_t beta, bool root);
	HashTable table;
	move_t result_move;

	struct search_stats_t {
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

		void reset() {
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

		void print() const {
			std::cout.setf(std::ios::fixed);
			std::cout << std::setprecision(5);
			std::cout
				<< "Search Stats: " << std::endl
				<< "  Nodes: total " << node_count
					<< ", exact " << pv_count << " (" << (100.0 * (double) pv_count / (double) node_count)
					<< "%), alpha " << alpha_count << " (" << (100.0 * (double)alpha_count / (double)node_count)
					<< ", beta " << beta_count << " (" << (100.0 * (double)beta_count / (double)node_count) << "%)" << std::endl
				<< "  Spreads: exact " << ((100.0*(double)pv_spread) / (double)pv_count)
				<< "%, alpha " << ((100.0*(double)alpha_spread) / (double)alpha_count)
				<< "%, beta " << ((100.0*(double)beta_spread) / (double)beta_count) << "%" << std::endl
				<< "  Beta Cut Move IDX: " << std::endl
				;
			for (int i = 0; i < 63; i++)
				std::cout << std::setfill(' ') << std::setw(6) << i << ": " << beta_top[i] << " (" << ((100.0 * (double)beta_top[i]) / (double)beta_count) << "%)" << std::endl;
			std::cout << "   63+: " << beta_top[63] << " (" << ((100.0 * (double)beta_top[63]) / (double)beta_count) << "%)" << std::endl;
			std::cout << "  First Non-Alpha Move IDX:" << std::endl;
			for (int i = 0; i < 63; i++)
				std::cout << std::setfill(' ') << std::setw(6) << i << ": " << first_beats_alpha[i] << " (" << ((100.0 * (double)first_beats_alpha[i]) / (double)(beta_count+pv_count)) << "%)" << std::endl;
			std::cout << "   63+: " << beta_top[63] << " (" << ((100.0 * (double)beta_top[63]) / (double)(beta_count+pv_count)) << "%)" << std::endl;

			std::cout
				<< "TT Stats:" << std::endl
				<< "  TT hits: " << tt_count << " (" << ((100.0 * (double)tt_count) / (double)(node_count + tt_beta + tt_res)) << "%)" << std::endl
				<< "  TT eval: " << tt_res << " (" << ((100.0 * (double)tt_res) / (double)(tt_count)) << "%)" << std::endl
				<< "  TT beta: " << tt_beta << " " << ((100.0 * (double)tt_beta) / (double)tt_count) << "%)" << std::endl
				;
		}
	};

	search_stats_t stats;
};
