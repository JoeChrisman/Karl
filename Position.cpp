//
// Created by Joe Chrisman on 4/5/23.
//

#include "Position.h"
#include "Notation.h"
#include <sstream>

Position::Position(const Zobrist& zobrist)
: zobrist(zobrist), bitboards{0}, pieces{0}, history{0}
{
    hash = 0;

    whitePieces = EMPTY_BOARD;
    blackPieces = EMPTY_BOARD;
    emptySquares = EMPTY_BOARD;
    occupiedSquares = EMPTY_BOARD;
    blackOrEmpty = EMPTY_BOARD;
    whiteOrEmpty = EMPTY_BOARD;

    materialScore = 0;
    midgamePlacementScore = 0;
    endgamePlacementScore = 0;
    irreversibles = {};
}

bool Position::loadFen(const std::string& fen)
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
        throw std::runtime_error("invalid FEN");
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
        const Piece piece = charToPiece(letter);

        if (piece != NULL_PIECE)
        {
            pieces[square] = piece;
            bitboards[piece] |= getBoard(square);
            materialScore += PIECE_SCORES[piece];
            hash ^= zobrist.PIECES[square][piece];
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
        hash ^= zobrist.WHITE_TO_MOVE;
    }

    int enPassantFile = enPassantSquare == "-" ? -1 : charToFile(enPassantSquare[0]);
    if (enPassantFile < -1 || enPassantFile > 8)
    {
        clear();
        return false;
    }
    irreversibles.enPassantFile = enPassantFile;
    if (enPassantFile > -1)
    {
        hash ^= zobrist.EN_PASSANT[enPassantFile];
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
    hash ^= zobrist.CASTLING[irreversibles.castlingFlags];

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

void Position::print(const bool isWhiteOnBottom)
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
            std::cout << " " << pieceToUnicode(piece) << " ";
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

template<bool isWhite>
void Position::doMove(const Move move)
{
    totalPlies++;
    irreversibles.reversiblePlies++;

    const Square squareFrom = getFrom(move);
    const Square squareTo = getTo(move);
    const Piece moving = getMoved(move);
    const Piece captured = getCaptured(move);
    const Piece promoted = getPromoted(move);

    const U64 to = getBoard(squareTo);
    const U64 from = getBoard(squareFrom);

    // remove the piece
    bitboards[moving] ^= from;
    pieces[squareFrom] = NULL_PIECE;
    hash ^= zobrist.PIECES[squareFrom][moving];

    midgamePlacementScore -= MIDGAME_PLACEMENT_SCORES[moving][squareFrom];

    // remove castling rights
    irreversibles.castlingFlags &= CASTLING_FLAGS[squareTo];
    irreversibles.castlingFlags &= CASTLING_FLAGS[squareFrom];
    hash ^= zobrist.CASTLING[Position::irreversibles.castlingFlags];

    if (moving == (isWhite ? WHITE_PAWN : BLACK_PAWN))
    {
        // pawn moves are irreversible moves
        irreversibles.reversiblePlies = 0;
    }

    // if we promoted
    if (promoted != NULL_PIECE)
    {
        // put the promoted piece on the target square
        pieces[squareTo] = promoted;
        bitboards[promoted] |= to;
        hash ^= zobrist.PIECES[squareTo][promoted];

        materialScore += PIECE_SCORES[promoted];
        midgamePlacementScore += MIDGAME_PLACEMENT_SCORES[promoted][squareTo];
    }
    // if we did not promote
    else
    {
        // put the moving piece on the target square
        pieces[squareTo] = moving;
        bitboards[moving] |= to;
        hash ^= zobrist.PIECES[squareTo][moving];

        midgamePlacementScore += MIDGAME_PLACEMENT_SCORES[moving][squareTo];
    }

    // if we pushed a pawn two squares
    if (move & DOUBLE_PAWN_PUSH)
    {
        // enable en passant square
        int enPassantFile = getFile(squareTo);
        irreversibles.enPassantFile = enPassantFile;
        hash ^= zobrist.EN_PASSANT[enPassantFile];
    }
    // if we did not enable an en passant move
    else
    {
        if (irreversibles.enPassantFile > -1)
        {
            // disable en passant move from last time
            hash ^= zobrist.EN_PASSANT[Position::irreversibles.enPassantFile];
            irreversibles.enPassantFile = -1;
        }
        // if we castled short
        if (move & SHORT_CASTLE)
        {
            static constexpr Piece eastRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
            static constexpr Square rookFrom = isWhite ? H1 : H8;
            static constexpr Square rookTo = isWhite ? F1 : F8;
            // move the east rook west of the king
            pieces[rookFrom] = NULL_PIECE;
            bitboards[eastRook] ^= getBoard(rookFrom);
            hash ^= zobrist.PIECES[rookFrom][eastRook];
            pieces[rookTo] = eastRook;
            bitboards[eastRook] |= getBoard(rookTo);
            hash ^= zobrist.PIECES[rookTo][eastRook];

            midgamePlacementScore -= MIDGAME_PLACEMENT_SCORES[eastRook][rookFrom];
            midgamePlacementScore += MIDGAME_PLACEMENT_SCORES[eastRook][rookTo];
        }
        // if we castled long
        else if (move & LONG_CASTLE)
        {
            static constexpr Piece westRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
            static constexpr Square rookFrom = isWhite ? A1 : A8;
            static constexpr Square rookTo = isWhite ? D1 : D8;
            // move the west rook east of the king
            pieces[rookFrom] = NULL_PIECE;
            bitboards[westRook] ^= getBoard(rookFrom);
            hash ^= zobrist.PIECES[rookFrom][westRook];
            pieces[rookTo] = westRook;
            bitboards[westRook] |= getBoard(rookTo);
            hash ^= zobrist.PIECES[rookTo][westRook];

            midgamePlacementScore -= MIDGAME_PLACEMENT_SCORES[westRook][rookFrom];
            midgamePlacementScore += MIDGAME_PLACEMENT_SCORES[westRook][rookTo];
        }
        // if we captured en passant
        else if (move & EN_PASSANT)
        {
            // perform en passant capture
            const Square enPassantCaptureSquare = isWhite ? south(squareTo) : north(squareTo);
            const U64 enPassantCapture = isWhite ? south(to) : north(to);
            bitboards[captured] ^= enPassantCapture;
            pieces[enPassantCaptureSquare] = NULL_PIECE;
            hash ^= zobrist.PIECES[enPassantCaptureSquare][captured];

            materialScore -= PIECE_SCORES[captured];
            midgamePlacementScore -= MIDGAME_PLACEMENT_SCORES[captured][enPassantCaptureSquare];
        }
        // if we captured normally
        else if (captured != NULL_PIECE)
        {
            // remove the captured piece
            bitboards[captured] ^= to;
            hash ^= zobrist.PIECES[squareTo][captured];

            // captures are irreversible moves
            irreversibles.reversiblePlies = 0;

            materialScore -= PIECE_SCORES[captured];
            midgamePlacementScore -= MIDGAME_PLACEMENT_SCORES[captured][squareTo];
        }
    }

    isWhiteToMove = !isWhiteToMove;
    hash ^= zobrist.WHITE_TO_MOVE;
    updateBitboards();

    history[Position::totalPlies] = hash;
}

template<bool isWhite>
void Position::undoMove(const Move move, const Irreversibles& state)
{
    const Square squareFrom = getFrom(move);
    const Square squareTo = getTo(move);
    const Piece moved = getMoved(move);
    const Piece captured = getCaptured(move);
    const Piece promoted = getPromoted(move);

    const U64 from = getBoard(squareFrom);
    const U64 to = getBoard(squareTo);

    // copy the piece back to where it came from
    pieces[squareFrom] = moved;
    bitboards[moved] |= from;
    hash ^= zobrist.PIECES[squareFrom][moved];
    midgamePlacementScore += MIDGAME_PLACEMENT_SCORES[moved][squareFrom];

    // if we are un-promoting
    if (promoted != NULL_PIECE)
    {
        // remove the promoted piece
        pieces[squareTo] = NULL_PIECE;
        bitboards[promoted] ^= to;
        hash ^= zobrist.PIECES[squareTo][promoted];
        materialScore -= PIECE_SCORES[promoted];
        midgamePlacementScore -= MIDGAME_PLACEMENT_SCORES[promoted][squareTo];
    }
    else
    {
        // erase the piece we moved
        pieces[squareTo] = NULL_PIECE;
        bitboards[moved] ^= to;
        hash ^= zobrist.PIECES[squareTo][moved];
        midgamePlacementScore -= MIDGAME_PLACEMENT_SCORES[moved][squareTo];
    }

    // if we are un-enabling en passant
    if (move & DOUBLE_PAWN_PUSH)
    {
        Position::hash ^= zobrist.EN_PASSANT[Position::irreversibles.enPassantFile];
    }
    // if we are re-enabling en passant
    else if (Position::irreversibles.enPassantFile != state.enPassantFile)
    {
        Position::hash ^= zobrist.EN_PASSANT[state.enPassantFile];
    }

    // if we are un-capturing en passant
    if (move & EN_PASSANT)
    {
        // replace the captured pawn
        const Square enPassantCaptureSquare = isWhite ? south(squareTo) : north(squareTo);
        const U64 enPassantCapture = isWhite ? south(to) : north(to);
        pieces[enPassantCaptureSquare] = captured;
        bitboards[captured] |= enPassantCapture;
        hash ^= zobrist.PIECES[enPassantCaptureSquare][captured];
        materialScore += PIECE_SCORES[captured];
        midgamePlacementScore += MIDGAME_PLACEMENT_SCORES[captured][enPassantCaptureSquare];
    }
    // if we are un-capturing normally
    else if (captured != NULL_PIECE)
    {
        // replace the captured piece
        pieces[squareTo] = captured;
        bitboards[captured] |= to;
        hash ^= zobrist.PIECES[squareTo][captured];
        materialScore += PIECE_SCORES[captured];
        midgamePlacementScore += MIDGAME_PLACEMENT_SCORES[captured][squareTo];
    }
    // if we are undoing kingside castling
    else if (move & SHORT_CASTLE)
    {
        static constexpr Piece eastRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
        static constexpr Square rookFrom = isWhite ? F1 : F8;
        static constexpr Square rookTo = isWhite ? H1 : H8;
        // move the castled rook east back to where it came from
        pieces[rookFrom] = NULL_PIECE;
        bitboards[eastRook] ^= getBoard(rookFrom);
        hash ^= zobrist.PIECES[rookFrom][eastRook];
        pieces[rookTo] = eastRook;
        bitboards[eastRook] |= getBoard(rookTo);
        hash ^= zobrist.PIECES[rookTo][eastRook];

        midgamePlacementScore -= MIDGAME_PLACEMENT_SCORES[eastRook][rookFrom];
        midgamePlacementScore += MIDGAME_PLACEMENT_SCORES[eastRook][rookTo];
    }
    // if we are undoing queenside castling
    else if (move & LONG_CASTLE)
    {
        static constexpr Piece westRook = isWhite ? WHITE_ROOK : BLACK_ROOK;
        static constexpr Square rookFrom = isWhite ? D1 : D8;
        static constexpr Square rookTo = isWhite ? A1 : A8;
        // move the castled rook east back to where it came from
        pieces[rookFrom] = NULL_PIECE;
        bitboards[westRook] ^= getBoard(rookFrom);
        hash ^= zobrist.PIECES[rookFrom][westRook];
        pieces[rookTo] = westRook;
        bitboards[westRook] |= getBoard(rookTo);
        hash ^= zobrist.PIECES[rookTo][westRook];

        midgamePlacementScore -= MIDGAME_PLACEMENT_SCORES[westRook][rookFrom];
        midgamePlacementScore += MIDGAME_PLACEMENT_SCORES[westRook][rookTo];
    }

    hash ^= zobrist.CASTLING[Position::irreversibles.castlingFlags];
    hash ^= zobrist.WHITE_TO_MOVE;

    totalPlies--;
    isWhiteToMove = !Position::isWhiteToMove;
    irreversibles = state;
    updateBitboards();
}


void Position::clear()
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
