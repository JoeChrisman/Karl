//
// Created by Joe Chrisman on 4/11/23.
//

#ifndef KARL_MOVES_H
#define KARL_MOVES_H

#include "Defs.h"

/*
 * A 32 bit move is encoded as such:
 *
 * 0->5: square from (0->63)
 * 6->11: square to (0->63)
 * 12->15: piece moved (0->11)
 * 16->19: piece captured (0->11)
 * 20->23: piece promoted (0->11)
 * 24: is double pawn push (0->1)
 * 25: is en passant capture (0->1)
 * 26: is short castle (0->1)
 * 27: is long castle (0->1)
 */
typedef int Move;

namespace Moves
{
    const Move NULL_MOVE = 0;

    namespace
    {
        constexpr int SQUARE_FROM      = 0x0000003f;
        constexpr int SQUARE_TO        = 0x00000fc0;
        constexpr int PIECE_MOVED      = 0x0000f000;
        constexpr int PIECE_CAPTURED   = 0x000f0000;
        constexpr int PIECE_PROMOTED   = 0x00f00000;

        constexpr int SQUARE_FROM_SHIFT     = 0;
        constexpr int SQUARE_TO_SHIFT       = 6;
        constexpr int PIECE_MOVED_SHIFT     = 12;
        constexpr int PIECE_CAPTURED_SHIFT  = 16;
        constexpr int PIECE_PROMOTED_SHIFT  = 20;
    }

    // flags
    extern const int DOUBLE_PAWN_PUSH;
    extern const int EN_PASSANT;
    extern const int SHORT_CASTLE;
    extern const int LONG_CASTLE;

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
};


#endif //KARL_MOVES_H
