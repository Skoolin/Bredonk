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
    constexpr bool is_spread() const {
        return spread_perm != 0;
    }

    // returns the index of the starting/placement square  
    constexpr uint8_t square_idx() const {
        return square_and_type & 0b00111111U;
    }

    // returns if the piece placed is a capstone  
    constexpr bool is_cap_placement() const {
        //return Piece(static_cast<Piece::Value>(square_and_type >> 6)).is_capstone();
        return (square_and_type & 0b11000000U) == 0b11000000U;
    }

    // returns the integer representation of the piece type  
    constexpr Piece piece_type(bool player) const {
        uint8_t piece_as_int = ((uint8_t)(square_and_type >> 6)) + player * Piece::B_START;
        return Piece(static_cast<Piece::Value>(piece_as_int));
    }

    // returns the integer representation of the direction  
    constexpr int spread_direction() const {
        return square_and_type >> 6;
    }

    constexpr int spread_distance() const {
        return std::popcount(spread_perm);
    }

    struct _square_pos_t {
        uint8_t row;
        uint8_t col;
    };

    constexpr _square_pos_t row_col() const {
        uint8_t idx = square_and_type & 0b00111111U;
        _square_pos_t t;
        t.row = idx >> 3;
        t.col = idx & 0b111U;
        return t;
    }

    std::string get_ptn() const {
        auto rc = row_col();
        std::string square_name = "";
        square_name.push_back('a' + rc.col);
        square_name.push_back('1' + rc.row);
        if (is_spread()) {
            // TODO
            char dirs[4] = { '+' , '>', '-', '<' };
            square_name.push_back(dirs[spread_direction()]);
            return square_name;
        }
        else {
            Piece type = piece_type(PLAYER_WHITE);
            std::string s = "";
            switch (type) {
            case Piece::W_CAP: s = "C"; break;
            case Piece::W_WALL: s = "S"; break;
            default: break;
            }
            return s + square_name;
        }
    }

    static move_t from_ptn(std::string ptn) {
        if (ptn[0] == 'C' || ptn[0] == 'S') {
            char c = ptn[0];
            ptn[0] = ptn[1];
            ptn[1] = ptn[2];
            ptn[2] = c;
        }
        uint8_t col = ptn[0] - 'a';
        uint8_t row = ptn[1] - '1';
        uint8_t spread_perm = 0;
        uint8_t square_idx = 8U * row + col;
        if (ptn.length() == 2) {
            uint8_t square_and_type = (0b01U << 6) | square_idx;
            return { square_and_type, spread_perm };
        }
        if (ptn[2] == 'S') {
            uint8_t square_and_type = (0b10U << 6) | square_idx;
            return { square_and_type, spread_perm };
        }
        if (ptn[2] == 'C') {
            uint8_t square_and_type = (0b11U << 6) | square_idx;
            return { square_and_type, spread_perm };
        }
        // TODO implement spread
        spread_perm = 0b11111111U;
        return { square_idx, spread_perm };
    }
};