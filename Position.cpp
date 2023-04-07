//
// Created by Joe Chrisman on 4/5/23.
//

#include "Position.h"

Position::Position(const std::string& fen)
{
    bool readingPieces = true;
    Square square = A8;
    for (const char letter : fen)
    {
        if (readingPieces)
        {
            if (letter == ' ')
            {
                readingPieces = false;
            }
            else
            {
                Piece piece = getPieceByChar(letter);

                if (piece != NULL_PIECE)
                {
                    pieces[square] = piece;
                    bitboards[piece] |= toBoard(square++);
                }
                else if (isdigit(letter))
                {
                    int digit = letter - '0';
                    square += digit;
                }
            }
        }
        else
        {
            // read position rights...
        }
    }
}

inline void Position::updateBitboards()
{
    whitePieces = pieces[WHITE_PAWN] |
                        pieces[WHITE_KNIGHT] |
                        pieces[WHITE_BISHOP] |
                        pieces[WHITE_ROOK] |
                        pieces[WHITE_QUEEN] |
                        pieces[WHITE_KING];

    blackPieces = pieces[BLACK_PAWN] |
                        pieces[BLACK_KNIGHT] |
                        pieces[BLACK_BISHOP] |
                        pieces[BLACK_ROOK] |
                        pieces[BLACK_QUEEN] |
                        pieces[BLACK_KING];

    occupiedSquares = whitePieces | blackPieces;
    emptySquares = ~occupiedSquares;
    whiteOrEmpty = whitePieces | emptySquares;
    blackOrEmpty = blackPieces | emptySquares;
}

void Position::makeMove(const Move move)
{
    Square from = getSquareFrom(move);
    Square to = getSquareTo(move);
    Piece moved = getPieceMoved(move);
    Piece captured = getPieceCaptured(move);

    pieces[from] = NULL_PIECE;
    pieces[to] = moved;

    bitboards[moved] ^= toBoard(from);
    if (captured != NULL_PIECE)
    {
        bitboards[captured] ^= toBoard(to);
    }
}

void Position::unMakeMove()
{

}

void Position::printPosition() const
{
    for (Square square = A8; square <= H1; square++)
    {
        if (toBoard(square) & FILE_MASKS[A_FILE])
        {
            std::cout << "\n";
        }
        Piece piece = pieces[square];
        if (piece != NULL_PIECE)
        {
            std::cout << " " << getUnicodePiece(piece) << " ";
        }
        else
        {
            std::cout << " . ";
        }
    }
    std::cout << "\n";
}

std::string Position::getUnicodePiece(const Piece piece) const
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

Piece Position::getPieceByChar(const char letter) const
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
