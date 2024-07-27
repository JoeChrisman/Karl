//
// Created by Joe Chrisman on 4/5/23.
//

#ifndef KARL_POSITION_H
#define KARL_POSITION_H

#include "Eval.h"
#include "Moves.h"
#include "Zobrist.h"

inline constexpr int WHITE_CASTLE_SHORT = 0x1;
inline constexpr int WHITE_CASTLE_LONG = 0x2;
inline constexpr int BLACK_CASTLE_SHORT = 0x4;
inline constexpr int BLACK_CASTLE_LONG = 0x8;

inline constexpr int WHITE_CASTLE = WHITE_CASTLE_LONG | WHITE_CASTLE_SHORT;
inline constexpr int BLACK_CASTLE = BLACK_CASTLE_LONG | BLACK_CASTLE_SHORT;

inline constexpr int CASTLING_FLAGS[64] = {
        ~BLACK_CASTLE_LONG, 15, 15, 15, ~BLACK_CASTLE,  15, 15, ~BLACK_CASTLE_SHORT,
        15, 15, 15, 15,            15,  15, 15,                  15,
        15, 15, 15, 15,            15,  15, 15,                  15,
        15, 15, 15, 15,            15,  15, 15,                  15,
        15, 15, 15, 15,            15,  15, 15,                  15,
        15, 15, 15, 15,            15,  15, 15,                  15,
        15, 15, 15, 15,            15,  15, 15,                  15,
        ~WHITE_CASTLE_LONG, 15, 15, 15, ~WHITE_CASTLE,  15, 15, ~WHITE_CASTLE_SHORT
};

class Position
{
public:
    Position(const Zobrist& zobrist);
    bool loadFen(const std::string& fen);

    struct Irreversibles
    {
        int castlingFlags;
        int enPassantFile;
        int reversiblePlies;
    };

    Hash hash;
    Hash history[MAX_MOVES];

    U64 bitboards[13];
    Piece pieces[64];

    U64 emptySquares;
    U64 occupiedSquares;
    U64 whitePieces;
    U64 blackPieces;
    U64 whiteOrEmpty;
    U64 blackOrEmpty;

    int placementScore;
    int materialScore;

    short totalPlies;
    bool isWhiteToMove;
    Irreversibles irreversibles{};

    void print(const bool isWhiteOnBottom);

    void makeMove(const Move move);
    void unMakeMove(const Move move, const Irreversibles& state);

    bool isZugzwang();
    void makeNullMove();
    void unMakeNullMove(const int enPassantBefore);

    void updateBitboards();
private:
    const Zobrist& zobrist;

    template<bool isWhite>
    void doMove(const Move move);

    template<bool isWhite>
    void undoMove(const Move move, const Irreversibles& state);

    void clear();

};


#endif //KARL_POSITION_H
