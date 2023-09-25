//
// Created by Joe Chrisman on 4/5/23.
//

#ifndef KARL_DEFS_H
#define KARL_DEFS_H

#include <string>
#include <iostream>
#include <vector>
#include <x86intrin.h>

typedef unsigned long long U64;
typedef int Piece;
typedef int Square;

constexpr U64 EMPTY_BOARD = 0;
constexpr U64 FULL_BOARD = ~0;

constexpr int MAX_MOVES = 1024;

constexpr U64 FILE_MASKS[8] = {
        0x0101010101010101,
        0x0202020202020202,
        0x0404040404040404,
        0x0808080808080808,
        0x1010101010101010,
        0x2020202020202020,
        0x4040404040404040,
        0x8080808080808080,
};

constexpr U64 RANK_MASKS[8] = {
        0xff00000000000000,
        0x00ff000000000000,
        0x0000ff0000000000,
        0x000000ff00000000,
        0x00000000ff000000,
        0x0000000000ff0000,
        0x000000000000ff00,
        0x00000000000000ff
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
    NULL_PIECE,
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
    BLACK_KING
};

enum : Square
{
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1
};

inline U64 getBoard(const Square square)
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

inline Square getSquare(const int rank, const int file)
{
    return (7 - rank) * 8 + file;
}

inline Square getSquare(const U64 board)
{
    return (Square)_mm_tzcnt_64(board);
}

inline Square popFirstPiece(U64& board)
{
    Square piece = getSquare(board);
    board ^= getBoard(piece);
    return piece;
}

inline int getNumPieces(const U64 board)
{
    return __builtin_popcountll(board);
}

template<int distance = 1>
U64 north(const U64 board)
{
    return board >> 8 * distance;
}

template<int distance = 1>
U64 east(const U64 board)
{
    return board << distance;
}

template<int distance = 1>
U64 south(const U64 board)
{
    return board << 8 * distance;
}

template<int distance = 1>
U64 west(const U64 board)
{
    return board >> distance;
}

template<int distance = 1>
U64 northEast(const U64 board)
{
    return board >> 7 * distance;
}

template<int distance = 1>
U64 northWest(const U64 board)
{
    return board >> 9 * distance;
}

template<int distance = 1>
U64 southEast(const U64 board)
{
    return board << 9 * distance;
}

template<int distance = 1>
U64 southWest(const U64 board)
{
    return board << 7 * distance;
}

template<int distance = 1>
Square north(const Square square)
{
    return square - 8 * distance;
}

template<int distance = 1>
Square east(const Square square)
{
    return square + distance;
}

template<int distance = 1>
Square south(const Square square)
{
    return square + 8 * distance;
}

template<int distance = 1>
Square west(const Square square)
{
    return square - distance;
}

template<int distance = 1>
Square northEast(const Square square)
{
    return square - 7 * distance;
}

template<int distance = 1>
Square southEast(const Square square)
{
    return square + 9 * distance;
}


template<int distance = 1>
Square southWest(const Square square)
{
    return square + 7 * distance;
}

template<int distance = 1>
Square northWest(const Square square)
{
    return square - 9 * distance;
}

inline long getEpochMillis()
{
    return duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
}

// some debug code will live here temporarily

inline void printBitboard(const U64 board)
{
    for (Square square = A8; square <= H1; square++)
    {
        if (FILE_MASKS[A_FILE] & getBoard(square))
        {
            std::cout << "\n";
        }
        if (board & getBoard(square))
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

#endif //KARL_DEFS_H
