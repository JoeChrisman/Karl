//
// Created by Joe Chrisman on 4/5/23.
//

#include "Position.h"

Position::Position(const std::string& fen)
{
    bitboards = std::vector<U64>(12, EMPTY_BOARD);
    pieces = std::vector<Piece>(64, NULL_PIECE);

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
                    U64 board = getBoard(square++);
                    bitboards[piece] |= board;
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
    updateBitboards();
}

inline void Position::updateBitboards()
{
    whitePieces = bitboards[WHITE_PAWN] |
                  bitboards[WHITE_KNIGHT] |
                  bitboards[WHITE_BISHOP] |
                  bitboards[WHITE_ROOK] |
                  bitboards[WHITE_QUEEN] |
                  bitboards[WHITE_KING];

    blackPieces = bitboards[BLACK_PAWN] |
                  bitboards[BLACK_KNIGHT] |
                  bitboards[BLACK_BISHOP] |
                  bitboards[BLACK_ROOK] |
                  bitboards[BLACK_QUEEN] |
                  bitboards[BLACK_KING];

    occupiedSquares = whitePieces | blackPieces;
    emptySquares = ~occupiedSquares;
    whiteOrEmpty = whitePieces | emptySquares;
    blackOrEmpty = blackPieces | emptySquares;
}

void Position::makeMove(const Move move)
{
    const Square from = getSquareFrom(move);
    const Square to = getSquareTo(move);
    const Piece moved = getPieceMoved(move);
    const Piece captured = getPieceCaptured(move);

    pieces[from] = NULL_PIECE;
    pieces[to] = moved;

    bitboards[moved] ^= getBoard(from);
    bitboards[moved] |= getBoard(to);
    if (captured != NULL_PIECE)
    {
        bitboards[captured] ^= getBoard(to);
    }

    isWhiteToMove = !isWhiteToMove;
    updateBitboards();
}

void Position::unMakeMove(const Move move)
{
    const Square from = getSquareFrom(move);
    const Square to = getSquareTo(move);
    const Piece moved = getPieceMoved(move);
    const Piece captured = getPieceCaptured(move);

    assert(from >= A8 && from <= H1);
    assert(to >= A8 && to <= H1);
    assert(moved != NULL_PIECE);


    pieces[from] = moved;
    pieces[to] = captured;

    bitboards[moved] ^= getBoard(to);
    bitboards[moved] |= getBoard(from);

    if (captured != NULL_PIECE)
    {
        bitboards[captured] ^= getBoard(to);
    }

    isWhiteToMove = !isWhiteToMove;
    updateBitboards();
}

void Position::printPosition(bool isWhiteOnBottom) const
{
    char rank = isWhiteOnBottom ? '8' : '1';
    for (Square square = isWhiteOnBottom ? A8 : H1;
            square >= A8 && square <= H1;
            isWhiteOnBottom ? square++ : square--)
    {
        if (getBoard(square) & FILE_MASKS[isWhiteOnBottom ? A_FILE : H_FILE])
        {
            std::cout << "\n" << rank << "   ";
            rank += isWhiteOnBottom ? -1 : 1;
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
    std::cout << "\n\n    ";
    for (char file = isWhiteOnBottom ? 'a' : 'h';
        file >= 'a' && file <= 'h';
        isWhiteOnBottom ? file++ : file--)
    {
        std::cout << " " << file << " ";
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
