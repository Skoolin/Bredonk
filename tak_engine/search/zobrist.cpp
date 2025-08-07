#include "zobrist.h"

hash_entry_t hash_bucket_t::get(uint64_t zobrist) const {
	for (int i = 0; i < ENTRIES_PER_BUCKET; i++)
		if (false && entries[i].lower_zobrist == (zobrist & 0xFFFFU)) return entries[i];
	return {};
}

void hash_bucket_t::update(uint64_t zobrist, int move_num, int depth, move_t move, int16_t eval, bool alpha_cut, bool beta_cut) {
	hash_entry_t entry{};
	entry.eval = eval;
	entry.lower_zobrist = zobrist & 0xFFFFU;
	entry.move = move;
	entry.depth = depth;
	entry.age_flags = (move_num & 0b11111000U) | 0b00000100U | (alpha_cut << 1) | beta_cut;

	int replacement_idx = -1;
	int worst_val = entry.replacement_value(move_num);
	for (int i = 0; i < ENTRIES_PER_BUCKET; i++) {
		int val = entries[i].replacement_value(move_num);
		if (val < worst_val) {
			worst_val = val;
			replacement_idx = i;
		}
	}
	if (replacement_idx >= 0)
		entries[replacement_idx] = entry;
}
