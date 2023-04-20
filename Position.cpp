//
// Created by Joe Chrisman on 4/5/23.
//

#include "Position.h"
#include "Notation.h"
#include <sstream>

Hash Position::hash;

U64 Position::bitboards[13];
Piece Position::pieces[64];

U64 Position::emptySquares = EMPTY_BOARD;
U64 Position::occupiedSquares = EMPTY_BOARD;
U64 Position::whitePieces = EMPTY_BOARD;
U64 Position::blackPieces = EMPTY_BOARD;
U64 Position::whiteOrEmpty = EMPTY_BOARD;
U64 Position::blackOrEmpty = EMPTY_BOARD;

bool Position::isWhiteToMove = true;

Position::Irreversibles Position::irreversibles = {};
Score Position::materialScore = 0;
int Position::totalPlies = 0;

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


    template<bool isWhite>
    void doMove(const Move move)
    {
        Position::totalPlies++;
        Position::irreversibles.reversiblePlies++;

        const Square squareFrom = Moves::getFrom(move);
        const Square squareTo = Moves::getTo(move);
        const Piece moving = Moves::getMoved(move);
        const Piece captured = Moves::getCaptured(move);
        const Piece promoted = Moves::getPromoted(move);

        const U64 to = getBoard(squareTo);
        const U64 from = getBoard(squareFrom);

        // remove the piece
        Position::bitboards[moving] ^= from;
        Position::pieces[squareFrom] = NULL_PIECE;
        Position::hash ^= Zobrist::PIECES[squareFrom][moving];

        // remove castling rights
        Position::irreversibles.castlingFlags &= CASTLING_FLAGS[squareTo];
        Position::irreversibles.castlingFlags &= CASTLING_FLAGS[squareFrom];
        Position::hash ^= Zobrist::CASTLING[Position::irreversibles.castlingFlags];

        if (moving == isWhite ? WHITE_PAWN : BLACK_PAWN)
        {
            // pawn moves are irreversible moves
            Position::irreversibles.reversiblePlies = 0;
        }

        // if we promoted
        if (promoted != NULL_PIECE)
        {

            // put the promoted piece on the target square
            Position::pieces[squareTo] = promoted;
            Position::bitboards[promoted] |= to;
            Position::hash ^= Zobrist::PIECES[squareTo][promoted];

            Position::materialScore += Eval::PIECE_SCORES[promoted];
        }
        // if we did not promote
        else
        {
            // put the moving piece on the target square
            Position::pieces[squareTo] = moving;
            Position::bitboards[moving] |= to;
            Position::hash ^= Zobrist::PIECES[squareTo][moving];
        }

        // if we pushed a pawn two squares
        if (move & Moves::DOUBLE_PAWN_PUSH)
        {
            // enable en passant square
            int enPassantFile = getFile(squareTo);
            Position::irreversibles.enPassantFile = enPassantFile;
            Position::hash ^= Zobrist::EN_PASSANT[enPassantFile];

            // pawn pushes are irreversible moves
            Position::irreversibles.reversiblePlies = 0;
        }
        // if we did not enable an en passant move
        else
        {
            if (Position::irreversibles.enPassantFile > -1)
            {
                // disable en passant move from last time
                Position::hash ^= Zobrist::EN_PASSANT[Position::irreversibles.enPassantFile];
                Position::irreversibles.enPassantFile = -1;
            }
            // if we castled short
            if (move & Moves::SHORT_CASTLE)
            {
                static constexpr Piece eastRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
                static constexpr Square rookFrom = isWhite ? H1 : H8;
                static constexpr Square rookTo = isWhite ? F1 : F8;
                // move the east rook west of the king
                Position::pieces[rookFrom] = NULL_PIECE;
                Position::bitboards[eastRook] ^= getBoard(rookFrom);
                Position::hash ^= Zobrist::PIECES[rookFrom][eastRook];
                Position::pieces[rookTo] = eastRook;
                Position::bitboards[eastRook] |= getBoard(rookTo);
                Position::hash ^= Zobrist::PIECES[rookTo][eastRook];

            }
            // if we castled long
            else if (move & Moves::LONG_CASTLE)
            {
                static constexpr Piece westRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
                static constexpr Square rookFrom = isWhite ? A1 : A8;
                static constexpr Square rookTo = isWhite ? D1 : D8;
                // move the west rook east of the king
                Position::pieces[rookFrom] = NULL_PIECE;
                Position::bitboards[westRook] ^= getBoard(rookFrom);
                Position::hash ^= Zobrist::PIECES[rookFrom][westRook];
                Position::pieces[rookTo] = westRook;
                Position::bitboards[westRook] |= getBoard(rookTo);
                Position::hash ^= Zobrist::PIECES[rookTo][westRook];
            }
            // if we captured en passant
            else if (move & Moves::EN_PASSANT)
            {
                // perform en passant capture
                const Square enPassantCaptureSquare = isWhite ? south(squareTo) : north(squareTo);
                const U64 enPassantCapture = isWhite ? south(to) : north(to);
                Position::materialScore -= Eval::PIECE_SCORES[captured];
                Position::bitboards[captured] ^= enPassantCapture;
                Position::pieces[enPassantCaptureSquare] = NULL_PIECE;
                Position::hash ^= Zobrist::PIECES[enPassantCaptureSquare][captured];

                // en passant captures are irreversible moves
                Position::irreversibles.reversiblePlies = 0;
            }
            // if we captured normally
            else if (captured != NULL_PIECE)
            {
                Position::materialScore -= Eval::PIECE_SCORES[captured];
                // remove the captured piece
                Position::bitboards[captured] ^= to;
                Position::hash ^= Zobrist::PIECES[squareTo][captured];

                // captures are irreversible moves
                Position::irreversibles.reversiblePlies = 0;
            }
        }

        Position::isWhiteToMove = !Position::isWhiteToMove;
        Position::hash ^= Zobrist::WHITE_TO_MOVE;
        Position::updateBitboards();
    }

    template<bool isWhite>
    void undoMove(const Move move, const Position::Irreversibles& state)
    {
        const Square squareFrom = Moves::getFrom(move);
        const Square squareTo = Moves::getTo(move);
        const Piece moved = Moves::getMoved(move);
        const Piece captured = Moves::getCaptured(move);
        const Piece promoted = Moves::getPromoted(move);

        const U64 from = getBoard(squareFrom);
        const U64 to = getBoard(squareTo);

        // move the piece back
        Position::pieces[squareFrom] = moved;
        Position::bitboards[moved] |= from;
        Position::hash ^= Zobrist::PIECES[squareFrom][moved];

        // if we are un-promoting
        if (promoted != NULL_PIECE)
        {
            // remove the promoted piece
            Position::pieces[squareTo] = NULL_PIECE;
            Position::bitboards[promoted] ^= to;
            Position::hash ^= Zobrist::PIECES[squareTo][promoted];
            Position::materialScore -= Eval::PIECE_SCORES[promoted];
        }
        else
        {
            // erase the piece we moved
            Position::pieces[squareTo] = NULL_PIECE;
            Position::bitboards[moved] ^= to;
            Position::hash ^= Zobrist::PIECES[squareTo][moved];
        }

        // if we are un-enabling en passant
        if (move & Moves::DOUBLE_PAWN_PUSH)
        {
            Position::hash ^= Zobrist::EN_PASSANT[Position::irreversibles.enPassantFile];
        }
        // if we are re-enabling en passant
        else if (Position::irreversibles.enPassantFile != state.enPassantFile)
        {
            Position::hash ^= Zobrist::EN_PASSANT[state.enPassantFile];
        }

        // if we are un-capturing en passant
        if (move & Moves::EN_PASSANT)
        {
            // replace the captured pawn
            const Square enPassantCaptureSquare = isWhite ? south(squareTo) : north(squareTo);
            const U64 enPassantCapture = isWhite ? south(to) : north(to);
            Position::pieces[enPassantCaptureSquare] = captured;
            Position::bitboards[captured] |= enPassantCapture;
            Position::hash ^= Zobrist::PIECES[enPassantCaptureSquare][captured];
            Position::materialScore += Eval::PIECE_SCORES[captured];
        }
        // if we are un-capturing normally
        else if (captured != NULL_PIECE)
        {
            // replace the captured piece
            Position::pieces[squareTo] = captured;
            Position::bitboards[captured] |= to;
            Position::hash ^= Zobrist::PIECES[squareTo][captured];
            Position::materialScore += Eval::PIECE_SCORES[captured];
        }
        // if we are undoing kingside castling
        else if (move & Moves::SHORT_CASTLE)
        {
            static constexpr Piece eastRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
            static constexpr Square rookFrom = isWhite ? F1 : F8;
            static constexpr Square rookTo = isWhite ? H1 : H8;
            // move the castled rook east back to where it came from
            Position::pieces[rookFrom] = NULL_PIECE;
            Position::bitboards[eastRook] ^= getBoard(rookFrom);
            Position::hash ^= Zobrist::PIECES[rookFrom][eastRook];
            Position::pieces[rookTo] = eastRook;
            Position::bitboards[eastRook] |= getBoard(rookTo);
            Position::hash ^= Zobrist::PIECES[rookTo][eastRook];
        }
        // if we are undoing queenside castling
        else if (move & Moves::LONG_CASTLE)
        {
            static constexpr Piece westRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
            static constexpr Square rookFrom = isWhite ? D1 : D8;
            static constexpr Square rookTo = isWhite ? A1 : A8;
            // move the castled rook east back to where it came from
            Position::pieces[rookFrom] = NULL_PIECE;
            Position::bitboards[westRook] ^= getBoard(rookFrom);
            Position::hash ^= Zobrist::PIECES[rookFrom][westRook];
            Position::pieces[rookTo] = westRook;
            Position::bitboards[westRook] |= getBoard(rookTo);
            Position::hash ^= Zobrist::PIECES[rookTo][westRook];
        }

        Position::hash ^= Zobrist::CASTLING[Position::irreversibles.castlingFlags];
        Position::hash ^= Zobrist::WHITE_TO_MOVE;

        Position::totalPlies--;
        Position::isWhiteToMove = !Position::isWhiteToMove;
        Position::irreversibles = state;
        Position::updateBitboards();
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
    const std::string halfMoveClock = fenParts[4];
    const std::string fullMoveCounter = fenParts[5];

    Square square = A8;
    for (const char letter : position)
    {
        const Piece piece = Notation::charToPiece(letter);

        if (piece != NULL_PIECE)
        {
            pieces[square] = piece;
            bitboards[piece] |= getBoard(square);
            materialScore += Eval::PIECE_SCORES[piece];
            hash ^= Zobrist::PIECES[square][piece];
            square++;
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
    isWhiteToMove = playerToMove == "w";
    if (isWhiteToMove)
    {
        hash ^= Zobrist::WHITE_TO_MOVE;
    }

    int enPassantFile = enPassantSquare == "-" ? -1 : Notation::charToFile(enPassantSquare[0]);
    if (enPassantFile < -1 || enPassantFile > 8)
    {
        clear();
        return false;
    }
    irreversibles.enPassantFile = enPassantFile;
    if (enPassantFile > -1)
    {
        Position::hash ^= Zobrist::EN_PASSANT[enPassantFile];
    }

    if (castlingRights.find('K') != std::string::npos)
    {
        irreversibles.castlingFlags |= WHITE_CASTLE_SHORT;
    }
    if (castlingRights.find('Q') != std::string::npos)
    {
        irreversibles.castlingFlags |= WHITE_CASTLE_LONG;
    }
    if (castlingRights.find('k') != std::string::npos)
    {
        irreversibles.castlingFlags |= BLACK_CASTLE_SHORT;
    }
    if (castlingRights.find('q') != std::string::npos)
    {
        irreversibles.castlingFlags |= BLACK_CASTLE_LONG;
    }
    hash ^= Zobrist::CASTLING[irreversibles.castlingFlags];

    try
    {
        totalPlies = 2 * (std::stoi(fullMoveCounter) - 1);
        irreversibles.reversiblePlies = std::stoi(halfMoveClock);
        if (totalPlies < 0 || irreversibles.reversiblePlies < 0)
        {
            clear();
            return false;
        }
    }
    catch (const std::exception& exception)
    {
        clear();
        return false;
    }

    return true;
}

inline void Position::clear()
{
    std::memset(bitboards, EMPTY_BOARD, sizeof(bitboards));
    std::memset(pieces, NULL_PIECE, sizeof(pieces));
    updateBitboards();
    hash = 0;
    materialScore = 0;
    totalPlies = 0;
    isWhiteToMove = true;
    irreversibles = {};
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
    if (isWhiteToMove)
    {
        doMove<true>(move);
    }
    else
    {
        doMove<false>(move);
    }
}

void Position::unMakeMove(const Move move, const Irreversibles& state)
{
    if (!isWhiteToMove)
    {
        undoMove<true>(move, state);
    }
    else
    {
        undoMove<false>(move, state);
    }
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

