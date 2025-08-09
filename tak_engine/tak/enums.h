#pragma once

#include <stdint.h>

enum player_t: bool {
	PLAYER_WHITE = 0,
	PLAYER_BLACK = 1
};

enum game_state_t: int32_t {
	STATE_ONGOING = 2,
	STATE_DRAW = 0,
	STATE_BLACK_WIN = -1,
	STATE_WHITE_WIN = 1,
	STATE_ILLEGAL = 3
};
