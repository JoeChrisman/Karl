//
// Created by Joe Chrisman on 4/12/23.
//

#include <string>
#include "Notation.h"

std::string Notation::pieceToUnicode(const Piece piece)
{
    switch (piece)
    {
        case WHITE_PAWN: return "\u265F";
        case WHITE_KNIGHT: return "\u265E";
        case WHITE_BISHOP: return "\u265D";
        case WHITE_ROOK: return "\u265C";
        case WHITE_QUEEN: return "\u265B";
        case WHITE_KING: return "\u265A";
        case BLACK_PAWN: return "\u2659";
        case BLACK_KNIGHT: return "\u2658";
        case BLACK_BISHOP: return "\u2657";
        case BLACK_ROOK: return "\u2656";
        case BLACK_QUEEN: return "\u2655";
        case BLACK_KING: return "\u2654";
        default: return "";
    }
}

Piece Notation::charToPiece(const char letter)
{
    switch (letter)
    {
        case 'P': return WHITE_PAWN;
        case 'N': return WHITE_KNIGHT;
        case 'B': return WHITE_BISHOP;
        case 'R': return WHITE_ROOK;
        case 'Q': return WHITE_QUEEN;
        case 'K': return WHITE_KING;
        case 'p': return BLACK_PAWN;
        case 'n': return BLACK_KNIGHT;
        case 'b': return BLACK_BISHOP;
        case 'r': return BLACK_ROOK;
        case 'q': return BLACK_QUEEN;
        case 'k': return BLACK_KING;
        default: return NULL_PIECE;
    }
}

char Notation::pieceToChar(const Piece piece)
{
    switch (piece)
    {
        case WHITE_PAWN: return 'W';
        case WHITE_KNIGHT: return 'N';
        case WHITE_BISHOP: return 'B';
        case WHITE_ROOK: return 'R';
        case WHITE_QUEEN: return 'Q';
        case WHITE_KING: return 'K';
        case BLACK_PAWN: return 'p';
        case BLACK_KNIGHT: return 'n';
        case BLACK_BISHOP: return 'b';
        case BLACK_ROOK: return 'r';
        case BLACK_QUEEN: return 'q';
        case BLACK_KING: return 'k';
        default: return ' ';
    }
}

int Notation::charToFile(const char fileChar)
{
    return (int)(fileChar - 'a');
}

std::string Notation::fileToStr(const int file)
{
    return std::string{char(file + 'a')};
}

std::string Notation::rankToStr(const int rank)
{
    return std::string{char('1' + rank)};
}

std::string Notation::squareToStr(const Square square)
{
    return fileToStr(getFile(square)) + rankToStr((getRank(square)));
}

std::string Notation::moveToStr(const Move move)
{
    if (move == Moves::NULL_MOVE)
    {
        return "NULL";
    }
    std::string str = squareToStr(Moves::getFrom(move)) + squareToStr(Moves::getTo(move));
    const Piece promoted = Moves::getPromoted(move);
    if (promoted != NULL_PIECE)
    {
        str += (char)tolower(pieceToChar(promoted));
    }
    return str;
}