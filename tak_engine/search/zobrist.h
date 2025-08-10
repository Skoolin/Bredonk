#pragma once
#include <cstdint>
#include "../tak/move.h"

constexpr unsigned int TABLE_SIZE = 1 << 20;
constexpr unsigned int ENTRIES_PER_BUCKET = 4;
constexpr unsigned int BUCKET_COUNT = TABLE_SIZE / ENTRIES_PER_BUCKET;
static_assert((TABLE_SIZE % ENTRIES_PER_BUCKET) == 0);

struct hash_entry_t {
	int16_t eval;
	int16_t lower_zobrist;
	move_t move;
	uint8_t depth;
	uint8_t age_flags;
	int replacement_value(int move_num) const {
		int age = (move_num >> 3) - (age_flags >> 3);
		return is_valid() ? depth - age : -1000; // TODO tune
	};
	constexpr bool is_valid() const {
		return age_flags != 0; // bit 3 = valid flag
	}
	constexpr bool exact() const {
		return !(age_flags & 0b011U);
	}
	constexpr bool alpha() const {
		return age_flags & 0b010U;
	}
	constexpr bool beta() const {
		return age_flags & 0b001U;
	}
	constexpr bool matches(uint64_t zobrist) const {
		return (zobrist & 0x0FFFFU) == lower_zobrist;
	}
};

struct hash_bucket_t {
	hash_entry_t entries[ENTRIES_PER_BUCKET];
	void update(uint64_t zobrist, int move_num, int depth, move_t move, int16_t eval, bool alpha_cut, bool beta_cut);
	constexpr hash_entry_t get(uint64_t zobrist) const {
		for (int i = 0; i < ENTRIES_PER_BUCKET; i++)
			if (entries[i].matches(zobrist)) return entries[i];
		return {};
	}
};

class HashTable {
public:
	hash_entry_t get(uint64_t zobrist) {
		return buckets[(zobrist >> 16) & (BUCKET_COUNT - 1)].get(zobrist);
	};
	void update(uint64_t zobrist, int move_num, int depth, move_t move, int16_t eval, bool alpha_cut, bool beta_cut) {
		buckets[(zobrist >> 16) & (BUCKET_COUNT - 1)].update(zobrist, move_num, depth, move, eval, alpha_cut, beta_cut);
	};
private:
	hash_bucket_t buckets[BUCKET_COUNT]{};
};
