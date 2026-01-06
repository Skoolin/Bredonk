#pragma once

#include "bitboard.h"
#include <array>

static std::array<magic_hash_t, 64> road_magics;

struct magic_hash_t {
	uint64_t magic_mul;
	uint64_t magic_mask;
};

class MagicRoad {
public:
private:
};