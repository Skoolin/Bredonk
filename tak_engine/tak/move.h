#pragma once

#include <bit>
#include <iostream>

#include "../tak/piece.h"
#include "enums.h"

struct move_t {
   uint8_t square_and_type; // 2 bit type/direction, 6 bit square index  
   uint8_t spread_perm;     // encoding of the spread. if 0, this is placement type else spread.

   // Equality operator to allow comparison with move_t::INVALID_MOVE  
   constexpr bool operator==(const move_t& other) const {  
       return square_and_type == other.square_and_type && spread_perm == other.spread_perm;  
   }

   constexpr bool operator!=(const move_t& other) const {  
       return !(*this == other);
   }

   // returns true if this is a spread and false if this is a placement.  
   inline bool is_spread() const {  
       return spread_perm != 0;  
   }

   // returns the index of the starting/placement square  
   inline int square_idx() const {  
       return square_and_type & 0b00111111;  
   }

   // returns if the piece placed is a capstone  
   inline bool is_cap_placement() const {
	   return Piece(static_cast<Piece::Value>(square_and_type >> 6)).is_capstone();
   }

   // returns the integer representation of the piece type  
   inline Piece piece_type(int player) const {
       uint8_t piece_as_int = ((uint8_t) (square_and_type >> 6))
           + (player == PLAYER_BLACK ? Piece::B_START : 0);
       return Piece(static_cast<Piece::Value>(piece_as_int));  
   }

   // returns the integer representation of the direction  
   inline int spread_direction() const {  
       return square_and_type >> 6;
   }

   inline int spread_distance() const {  
	   return std::popcount(spread_perm);
   }
};
