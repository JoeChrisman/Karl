//
// Created by Joe Chrisman on 4/11/23.
//

#ifndef KARL_MOVES_H
#define KARL_MOVES_H

#include "Defs.h"

/*
 * A 32 bit move is encoded like this:
 *
 * 0b
 * 11111 square from
 * 11111 square to
 * 1111  piece moved
 * 1111  piece captured
 * 111   piece promoted
 * 1     double pawn push flag
 * 1     en passant capture flag
 * 1     short castle flag
 * 1     long castle flag
 * 1111  unused
 */
typedef int Move;
inline constexpr Move NULL_MOVE = 0;

// numbers representing bits 24-27. they are move type flags
inline constexpr int DOUBLE_PAWN_PUSH = 0x01000000;
inline constexpr int EN_PASSANT       = 0x02000000;
inline constexpr int SHORT_CASTLE     = 0x04000000;
inline constexpr int LONG_CASTLE      = 0x08000000;

namespace
{
    // numbers we can use to decode information from a move
    constexpr int SQUARE_FROM      = 0x0000003f;
    constexpr int SQUARE_TO        = 0x00000fc0;
    constexpr int PIECE_MOVED      = 0x0000f000;
    constexpr int PIECE_CAPTURED   = 0x000f0000;
    constexpr int PIECE_PROMOTED   = 0x00f00000;

    // numbers we can use encode information into a move
    constexpr int SQUARE_FROM_SHIFT     = 0;
    constexpr int SQUARE_TO_SHIFT       = 6;
    constexpr int PIECE_MOVED_SHIFT     = 12;
    constexpr int PIECE_CAPTURED_SHIFT  = 16;
    constexpr int PIECE_PROMOTED_SHIFT  = 20;
}

inline Move createMove(
        const Square from,
        const Square to,
        const Piece moved,
        const Piece captured,
        const Piece promoted = NULL_PIECE)
{
    return (from << SQUARE_FROM_SHIFT) |
           (to << SQUARE_TO_SHIFT) |
           (moved << PIECE_MOVED_SHIFT) |
           (captured << PIECE_CAPTURED_SHIFT) |
           (promoted << PIECE_PROMOTED_SHIFT);
}

inline Square getFrom(const Move move)
{
    return (move & SQUARE_FROM) >> SQUARE_FROM_SHIFT;
}

inline Square getTo(const Move move)
{
    return (move & SQUARE_TO) >> SQUARE_TO_SHIFT;
}

inline Piece getMoved(const Move move)
{
    return (move & PIECE_MOVED) >> PIECE_MOVED_SHIFT;
}

inline Piece getCaptured(const Move move)
{
    return (move & PIECE_CAPTURED) >> PIECE_CAPTURED_SHIFT;
}

inline Piece getPromoted(const Move move)
{
    return (move & PIECE_PROMOTED) >> PIECE_PROMOTED_SHIFT;
}

#endif //KARL_MOVES_H
