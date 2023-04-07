//
// Created by Joe Chrisman on 4/5/23.
//

#ifndef KARL_DEFINITIONS_H
#define KARL_DEFINITIONS_H

#include <string>
#include <iostream>
#include <vector>
#include <x86intrin.h>

typedef unsigned long long U64;
typedef int Piece;
typedef int Square;
typedef int Move;

const std::string VERSION = "0.0.1 <beta>";
const std::string INITIAL_WHITE_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const std::string INITIAL_BLACK_FEN = "RNBKQBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbkqbnr w - - 0 1";

const U64 EMPTY_BOARD = 0;
const U64 FULL_BOARD = ~0;

const Move NULL_MOVE = 0;

const U64 FILE_MASKS[8] = {
        0x0101010101010101,
        0x0202020202020202,
        0x0404040404040404,
        0x0808080808080808,
        0x1010101010101010,
        0x2020202020202020,
        0x4040404040404040,
        0x8080808080808080,
};

const U64 RANK_MASKS[8] = {
        0x00000000000000ff,
        0x000000000000ff00,
        0x0000000000ff0000,
        0x00000000ff000000,
        0x000000ff00000000,
        0x0000ff0000000000,
        0x00ff000000000000,
        0xff00000000000000
};

enum
{
    A_FILE,
    B_FILE,
    C_FILE,
    D_FILE,
    E_FILE,
    F_FILE,
    G_FILE,
    H_FILE
};

enum
{
    FIRST_RANK,
    SECOND_RANK,
    THIRD_RANK,
    FOURTH_RANK,
    FIFTH_RANK,
    SIXTH_RANK,
    SEVENTH_RANK,
    EIGHTH_RANK
};


enum
{
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,
    NULL_PIECE
};

enum
{
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1,
    NULL_SQUARE
};

enum MoveType
{
    NORMAL,
    CASTLE,
    EN_PASSANT,
    KNIGHT_PROMOTION,
    BISHOP_PROMOTION,
    ROOK_PROMOTION,
    QUEEN_PROMOTION
};

inline Move createMove(
        const MoveType moveType,
        const Piece moved,
        const Piece captured,
        const Square from,
        const Square to)
{
    return  moveType << 29
                | moved << 25
                | captured << 21
                | from << 15
                | to << 9;
}

inline MoveType getMoveType(const Move move)
{
    return (MoveType)((move & 0b11100000000000000000000000000000) >> 29);
}

inline Piece getPieceMoved(const Move move)
{
    return (Piece)((move & 0b00011110000000000000000000000000) >> 25);
}

inline Piece getPieceCaptured(const Move move)
{
    return (Piece)((move & 0b00000001111000000000000000000000) >> 21);
}

inline Square getSquareFrom(const Move move)
{
    return (Square)((move & 0b00000000000111111000000000000000) >> 15);
}

inline Square getSquareTo(const Move move)
{
    return (Square)((move & 0b00000000000000000111111000000000) >> 9);
}

inline U64 toBoard(const Square square)
{
    return (U64)1 << square;
}

inline int getRank(const Square square)
{
    return 7 - square / 8;
}

inline int getFile(const Square square)
{
    return square % 8;
}

inline int toRank(const char letter)
{
    return letter - '0' - 1;
}

inline int toFile(const char letter)
{
    return tolower(letter) - 'a';
}

inline Square toSquare(const int rank, const int file)
{
    return (7 - rank) * 8 + file;
}

inline Square toSquare(const U64 board)
{
    return (Square)_mm_tzcnt_64(board);
}

inline Square toSquare(const std::string& notation)
{
    int rank = toRank(notation[1]);
    int file = toFile(notation[0]);

    return toSquare(rank, file);
}

inline Square popFirstPiece(U64& board)
{
    Square piece = toSquare(board);
    board ^= toBoard(piece);
    return piece;
}

template<int distance>
U64 north(const U64 board)
{
    return board >> 8 * distance;
}

template<int distance>
U64 east(const U64 board)
{
    return board << distance;
}

template<int distance>
U64 south(const U64 board)
{
    return board << 8 * distance;
}

template<int distance>
U64 west(const U64 board)
{
    return board << distance;
}

template<int distance>
Square north(const Square square)
{
    return square - 8 * distance;
}

template<int distance>
Square east(const Square square)
{
    return square + distance;
}

template<int distance>
Square south(const Square square)
{
    return square + 8 * distance;
}

template<int distance>
Square west(const Square square)
{
    return square - distance;
}

inline void printBitboard(const U64 board)
{
    for (Square square = A8; square <= H1; square++)
    {
        if (FILE_MASKS[A_FILE] & toBoard(square))
        {
            std::cout << "\n";
        }
        if (board & toBoard(square))
        {
            std::cout << " 1 ";
        }
        else
        {
            std::cout << " . ";
        }
    }
    std::cout << "\n";
}

#endif //KARL_DEFINITIONS_H
