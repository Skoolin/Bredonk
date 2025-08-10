#pragma once

#include "move_iterator.h"
#include "magic.h"

constexpr int MoveList_MAX_MOVES = 108; // Maximum number of moves in a MoveList
constexpr int MoveList_MAX_SPREAD = 34;

// MoveList is a concrete implementation of MoveIterator that stores a list of moves.
class MoveList// : public MoveIterator
{
public:
	MoveList();
	~MoveList();

	// base class methods
	constexpr move_t next() {
		// assuming has_next() is called before this method
		if (placement_done) {
			move_t m = spreads[current_index].next();
			if (!spreads[current_index].has_next())
				current_index++;
			return m;
		}
		move_t m = moves[current_index++];
		if (current_index >= move_count) {
			placement_done = true;
			current_index = 0;
		}
		return m;
	};
	constexpr void clear() {
		move_count = 0;
		spread_count = 0;
		current_index = 0;
		placement_done = false;
	};
	constexpr void reset() {
		current_index = 0;
		placement_done = false;
	};
	constexpr bool is_empty() const {
		return move_count == 0 && spread_count == 0;
	};
	constexpr bool has_next() const {
		return placement_done ? (current_index < spread_count && spreads[current_index].has_next()) : current_index < move_count;
	};

	constexpr void add_move(move_t m) {
		moves[move_count++] = m;
	};
	inline void add_spread(SpreadIterator i) {
		spreads[spread_count++] = i;
	};
	constexpr uint32_t size() {
		auto count = move_count;
		for (int i = 0; i < spread_count; i++) {
			count += spreads[i].count();
		}
		return count;
	};
private:
	SpreadIterator spreads[MoveList_MAX_SPREAD];
	move_t moves[MoveList_MAX_MOVES];
	uint8_t move_count;
	uint8_t current_index;
	uint8_t spread_count;
	bool placement_done;
};
