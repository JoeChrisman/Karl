//
// Created by Joe Chrisman on 4/5/23.
//

#include <sstream>
#include "Position.h"
#include "Notation.h"

std::vector<U64> Position::bitboards = std::vector<U64>(12, EMPTY_BOARD);
std::vector<Piece> Position::pieces = std::vector<Piece>(64, NULL_PIECE);

U64 Position::emptySquares = EMPTY_BOARD;
U64 Position::occupiedSquares = EMPTY_BOARD;
U64 Position::whitePieces = EMPTY_BOARD;
U64 Position::blackPieces = EMPTY_BOARD;
U64 Position::whiteOrEmpty = EMPTY_BOARD;
U64 Position::blackOrEmpty = EMPTY_BOARD;

Position::Rights Position::rights = {};

namespace
{
    std::string getUnicodePiece(const Piece piece)
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
    Piece getPieceByChar(const char letter)
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

    void clear()
    {
        Position::bitboards = std::vector<U64>(12, EMPTY_BOARD);
        Position::pieces = std::vector<Piece>(64, NULL_PIECE);
        Position::updateBitboards();
        Position::rights = {};
    }
}

bool Position::init(const std::string& fen)
{
    clear();

    std::vector<std::string> fenParts;
    std::stringstream stream(fen);
    std::string part;
    while (stream >> part)
    {
        fenParts.push_back(part);
    }
    if (fenParts.size() != 6)
    {
        return false;
    }

    const std::string position = fenParts[0];
    const std::string playerToMove = fenParts[1];
    const std::string castlingRights = fenParts[2];
    const std::string enPassantSquare = fenParts[3];
    const std::string halfMoves = fenParts[4];
    const std::string fullMoves = fenParts[5];

    Square square = A8;
    for (const char letter : position)
    {
        const Piece piece = getPieceByChar(letter);

        if (piece != NULL_PIECE)
        {
            pieces[square] = piece;
            bitboards[piece] |= getBoard(square++);
        }
        else if (isdigit(letter))
        {
            const int digit = letter - '0';
            if (digit < 1 || digit > 8)
            {
                clear();
                return false;
            }
            square += digit;
        }
        else if (letter != '/')
        {
            clear();
            return false;
        }
    }
    updateBitboards();

    if (playerToMove != "w" && playerToMove != "b")
    {
        clear();
        return false;
    }

    int enPassantFile = enPassantSquare == "-" ? -1 : Notation::charToFile(enPassantSquare[0]);
    if (enPassantFile < -1 || enPassantFile > 8)
    {
        clear();
        return false;
    }

    rights.enPassantFile = enPassantFile;
    rights.isWhiteToMove = playerToMove == "w";

    return true;
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
    if (rights.isWhiteToMove)
    {
        makeMove<true>(move);
    }
    else
    {
        makeMove<false>(move);
    }
}

void Position::unMakeMove(const Move move, const Rights& previousRights)
{
    if (!rights.isWhiteToMove)
    {
        unMakeMove<true>(move, previousRights);
    }
    else
    {
        unMakeMove<false>(move, previousRights);
    }
}

template<bool isWhite>
void Position::makeMove(const Move move)
{
    const Square from = Moves::getFrom(move);
    const Square to = Moves::getTo(move);
    const Piece moving = Moves::getMoved(move);
    const Piece captured = Moves::getCaptured(move);

    assert(moving >= WHITE_PAWN && moving <= NULL_PIECE);
    assert(captured >= WHITE_PAWN && captured <= NULL_PIECE);

    // move the piece
    bitboards[moving] ^= getBoard(from);
    pieces[from] = NULL_PIECE;
    bitboards[moving] |= getBoard(to);
    pieces[to] = moving;

    if (move & Moves::DOUBLE_PAWN_PUSH)
    {
        // enable en passant square
        rights.enPassantFile = getFile(to);
    }
    else
    {
        rights.enPassantFile = -1;
    }


    if (move & Moves::EN_PASSANT)
    {
        //assert(rights.enPassantFile >= 0 && rights.enPassantFile <= 8);
        /*if (rights.enPassantFile >= 0 && rights.enPassantFile <= 8)
        {
            int a = 1;
        }*/



        // perform en passant capture
        const Square enPassantCapture = isWhite ? south(to) : north(to);
        bitboards[captured] ^= getBoard(enPassantCapture);
        pieces[enPassantCapture] = NULL_PIECE;
    }
    else if (captured != NULL_PIECE)
    {
        bitboards[captured] ^= getBoard(to);
    }

    rights.isWhiteToMove = !rights.isWhiteToMove;
    updateBitboards();

}

template<bool isWhite>
void Position::unMakeMove(const Move move, const Rights& previousRights)
{
    rights = previousRights;

    const Square from = Moves::getFrom(move);
    const Square to = Moves::getTo(move);
    const Piece moved = Moves::getMoved(move);
    const Piece captured = Moves::getCaptured(move);

    assert(moved >= WHITE_PAWN && moved <= NULL_PIECE);
    assert(captured >= WHITE_PAWN && captured <= NULL_PIECE);

    // move the piece back
    pieces[from] = moved;
    pieces[to] = NULL_PIECE;
    bitboards[moved] ^= getBoard(to);
    bitboards[moved] |= getBoard(from);

    // if we are un-capturing en passant
    if (move & Moves::EN_PASSANT)
    {
        assert(captured == WHITE_PAWN || captured == BLACK_PAWN);
        const Square enPassantCapture = isWhite ? south(to) : north(to);
        pieces[enPassantCapture] = captured;
        bitboards[captured] |= getBoard(enPassantCapture);
    }
    else if (captured != NULL_PIECE)
    {
        pieces[to] = captured;
        bitboards[captured] |= getBoard(to);
    }

    updateBitboards();
}


void Position::print(bool isWhiteOnBottom)
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
        Piece piece = Position::pieces[square];
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

