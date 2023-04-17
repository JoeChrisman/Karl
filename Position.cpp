//
// Created by Joe Chrisman on 4/5/23.
//

#include <sstream>
#include "Position.h"
#include "Notation.h"

U64 Position::bitboards[13];
Piece Position::pieces[64];

U64 Position::emptySquares = EMPTY_BOARD;
U64 Position::occupiedSquares = EMPTY_BOARD;
U64 Position::whitePieces = EMPTY_BOARD;
U64 Position::blackPieces = EMPTY_BOARD;
U64 Position::whiteOrEmpty = EMPTY_BOARD;
U64 Position::blackOrEmpty = EMPTY_BOARD;

Position::Rights Position::rights = {};
Score Position::materialScore = 0;

namespace
{
    constexpr int WHITE_CASTLE = Position::WHITE_CASTLE_LONG | Position::WHITE_CASTLE_SHORT;
    constexpr int BLACK_CASTLE = Position::BLACK_CASTLE_LONG | Position::BLACK_CASTLE_SHORT;
    constexpr int CASTLING_FLAGS[64] = {
            ~Position::BLACK_CASTLE_LONG, 15, 15, 15, ~BLACK_CASTLE,  15, 15, ~Position::BLACK_CASTLE_SHORT,
            15,                           15, 15, 15,       15,       15, 15,                            15,
            15,                           15, 15, 15,       15,       15, 15,                            15,
            15,                           15, 15, 15,       15,       15, 15,                            15,
            15,                           15, 15, 15,       15,       15, 15,                            15,
            15,                           15, 15, 15,       15,       15, 15,                            15,
            15,                           15, 15, 15,       15,       15, 15,                            15,
            ~Position::WHITE_CASTLE_LONG, 15, 15, 15, ~WHITE_CASTLE,  15, 15, ~Position::WHITE_CASTLE_SHORT,
    };
}

bool Position::init(const std::string& fen)
{
    Position::rights.currentPly++;

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
        const Piece piece = Notation::charToPiece(letter);

        if (piece != NULL_PIECE)
        {
            pieces[square] = piece;
            bitboards[piece] |= getBoard(square++);
            materialScore += Eval::PIECE_SCORES[piece];
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

    if (castlingRights.find('K') != std::string::npos)
    {
        rights.castlingFlags |= WHITE_CASTLE_SHORT;
    }
    if (castlingRights.find('Q') != std::string::npos)
    {
        rights.castlingFlags |= WHITE_CASTLE_LONG;
    }
    if (castlingRights.find('k') != std::string::npos)
    {
        rights.castlingFlags |= BLACK_CASTLE_SHORT;
    }
    if (castlingRights.find('q') != std::string::npos)
    {
        rights.castlingFlags |= BLACK_CASTLE_LONG;
    }

    return true;
}

inline void Position::clear()
{
    std::memset(bitboards, EMPTY_BOARD, sizeof(bitboards));
    std::memset(pieces, NULL_PIECE, sizeof(pieces));
    updateBitboards();
    rights = {};
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
    const Square squareFrom = Moves::getFrom(move);
    const Square squareTo = Moves::getTo(move);
    const Piece moving = Moves::getMoved(move);
    const Piece captured = Moves::getCaptured(move);
    const Piece promoted = Moves::getPromoted(move);

    const U64 to = getBoard(squareTo);
    const U64 from = getBoard(squareFrom);

    // remove the piece
    bitboards[moving] ^= from;
    pieces[squareFrom] = NULL_PIECE;

    // remove castling rights
    rights.castlingFlags &= CASTLING_FLAGS[squareTo];
    rights.castlingFlags &= CASTLING_FLAGS[squareFrom];

    // if we promoted
    if (promoted != NULL_PIECE)
    {
        // put the promoted piece on the target square
        pieces[squareTo] = promoted;
        bitboards[promoted] |= to;

        materialScore += Eval::PIECE_SCORES[promoted];
    }
    // if we did not promote
    else
    {
        // put the moving piece on the target square
        pieces[squareTo] = moving;
        bitboards[moving] |= to;
    }

    // if we pushed a pawn two squares
    if (move & Moves::DOUBLE_PAWN_PUSH)
    {
        // enable en passant square
        rights.enPassantFile = getFile(squareTo);
    }
    // if we did not enable an en passant move
    else
    {
        // disable en passant move from last time
        rights.enPassantFile = -1;
        // if we castled short
        if (move & Moves::SHORT_CASTLE)
        {
            static constexpr Piece eastRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
            static constexpr Square rookFrom = isWhite ? H1 : H8;
            static constexpr Square rookTo = isWhite ? F1 : F8;
            // move the east rook west of the king
            pieces[rookFrom] = NULL_PIECE;
            bitboards[eastRook] ^= getBoard(rookFrom);
            pieces[rookTo] = eastRook;
            bitboards[eastRook] |= getBoard(rookTo);
        }
            // if we castled long
        else if (move & Moves::LONG_CASTLE)
        {
            static constexpr Piece westRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
            static constexpr Square rookFrom = isWhite ? A1 : A8;
            static constexpr Square rookTo = isWhite ? D1 : D8;
            // move the west rook east of the king
            pieces[rookFrom] = NULL_PIECE;
            bitboards[westRook] ^= getBoard(rookFrom);
            pieces[rookTo] = westRook;
            bitboards[westRook] |= getBoard(rookTo);
        }
        // if we captured en passant
        else if (move & Moves::EN_PASSANT)
        {
            // perform en passant capture
            const Square enPassantCaptureSquare = isWhite ? south(squareTo) : north(squareTo);
            const U64 enPassantCapture = isWhite ? south(to) : north(to);

            materialScore -= Eval::PIECE_SCORES[isWhite ? BLACK_PAWN : WHITE_PAWN];
            bitboards[captured] ^= enPassantCapture;
            pieces[enPassantCaptureSquare] = NULL_PIECE;
        }
        // if we captured normally
        else if (captured != NULL_PIECE)
        {
            materialScore -= Eval::PIECE_SCORES[captured];
            // remove the captured piece
            bitboards[captured] ^= to;
        }
    }

    rights.isWhiteToMove = !rights.isWhiteToMove;
    updateBitboards();
}

template<bool isWhite>
void Position::unMakeMove(const Move move, const Rights& previousRights)
{
    rights = previousRights;

    const Square squareFrom = Moves::getFrom(move);
    const Square squareTo = Moves::getTo(move);
    const Piece moved = Moves::getMoved(move);
    const Piece captured = Moves::getCaptured(move);
    const Piece promoted = Moves::getPromoted(move);

    const U64 from = getBoard(squareFrom);
    const U64 to = getBoard(squareTo);

    // move the piece back
    pieces[squareFrom] = moved;
    pieces[squareTo] = NULL_PIECE;
    bitboards[moved] |= from;
    bitboards[moved] &= ~to;

    // if we are un-promoting
    if (promoted != NULL_PIECE)
    {
        // remove the promoted piece
        bitboards[promoted] ^= to;
        // replace a captured piece
        pieces[squareTo] = captured;
        bitboards[captured] |= to;

        materialScore += Eval::PIECE_SCORES[captured];
        materialScore -= Eval::PIECE_SCORES[promoted];
    }
    // if we are undoing kingside castling
    else if (move & Moves::SHORT_CASTLE)
    {
        static constexpr Piece eastRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
        static constexpr Square rookFrom = isWhite ? F1 : F8;
        static constexpr Square rookTo = isWhite ? H1 : H8;
        // move the castled rook east back to where it came from
        pieces[rookFrom] = NULL_PIECE;
        bitboards[eastRook] ^= getBoard(rookFrom);
        pieces[rookTo] = eastRook;
        bitboards[eastRook] |= getBoard(rookTo);
    }
    // if we are undoing queenside castling
    else if (move & Moves::LONG_CASTLE)
    {
        static constexpr Piece westRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
        static constexpr Square rookFrom = isWhite ? D1 : D8;
        static constexpr Square rookTo = isWhite ? A1 : A8;
        // move the castled rook east back to where it came from
        pieces[rookFrom] = NULL_PIECE;
        bitboards[westRook] ^= getBoard(rookFrom);
        pieces[rookTo] = westRook;
        bitboards[westRook] |= getBoard(rookTo);
    }
    // if we are un-capturing en passant
    else if (move & Moves::EN_PASSANT)
    {
        const Square enPassantCaptureSquare = isWhite ? south(squareTo) : north(squareTo);
        const U64 enPassantCapture = isWhite ? south(to) : north(to);
        pieces[enPassantCaptureSquare] = captured;
        bitboards[captured] |= enPassantCapture;
        materialScore += Eval::PIECE_SCORES[isWhite ? BLACK_PAWN : WHITE_PAWN];
    }
    else if (captured != NULL_PIECE)
    {
        pieces[squareTo] = captured;
        bitboards[captured] |= to;
        materialScore += Eval::PIECE_SCORES[captured];
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
            std::cout << " " << Notation::pieceToUnicode(piece) << " ";
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

