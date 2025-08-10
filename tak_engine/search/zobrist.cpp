#include "zobrist.h"

void hash_bucket_t::update(uint64_t zobrist, int move_num, int depth, move_t move, int16_t eval, bool alpha_cut, bool beta_cut) {
	hash_entry_t new_entry{};
	new_entry.eval = eval;
	new_entry.lower_zobrist = zobrist & 0xFFFFU;
	new_entry.move = move;
	new_entry.depth = depth;
	new_entry.age_flags = (move_num & 0b11111000U) | 0b00000100U | (alpha_cut << 1) | beta_cut;

	int replacement_idx = -1;
	int replacement_val = new_entry.replacement_value(move_num);
	int worst_val = replacement_val;
	for (int i = 0; i < ENTRIES_PER_BUCKET; i++) {

		auto entry = entries[i];
		int val = entry.replacement_value(move_num);

		if (entry.matches(zobrist)) {
			if (val < replacement_val) {
				entries[i] = new_entry;
			}
			return;
		}

		if (val < worst_val) {
			worst_val = val;
			replacement_idx = i;
		}
	}
	if (replacement_idx >= 0)
		entries[replacement_idx] = new_entry;
}
