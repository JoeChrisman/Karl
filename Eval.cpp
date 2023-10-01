//
// Created by Joe Chrisman on 9/24/23.
//

#include <algorithm>
#include "Eval.h"

Evaluator::Evaluator(Position& position, MoveGen& moveGen) :
    position(position), moveGen(moveGen)
{
    for (PawnStructure& pawnStructure : pawnStructures)
    {
        pawnStructure = PawnStructure{
            false,
            0,
            EMPTY_BOARD,
            EMPTY_BOARD
        };
    }
}

Score Evaluator::evaluate()
{
    Score whiteAdvantage = position.materialScore + position.placementScore;

    const float openingWeight = getOpeningWeight();
    const float endgameWeight = 1.0f - openingWeight;

    const Square whiteKing = getSquare(position.bitboards[WHITE_KING]);
    const Square blackKing = getSquare(position.bitboards[BLACK_KING]);

    const Score whiteKingActivityAdvantage = static_cast<Score>(
            static_cast<float>(KING_ACTIVITY_SCORES[whiteKing] - KING_ACTIVITY_SCORES[blackKing]) * endgameWeight);

    const Score whiteKingSafetyAdvantage = static_cast<Score>(
            static_cast<float>(WHITE_KING_SAFETY_SCORES[whiteKing] - BLACK_KING_SAFETY_SCORES[blackKing]) * openingWeight);

    const U64 whitePawns = position.bitboards[WHITE_PAWN];
    const U64 blackPawns = position.bitboards[BLACK_PAWN];

    const Hash key = position.hash >> 51;
    PawnStructure& pawnStructure = pawnStructures[key];
    if (!pawnStructure.isValid ||
        pawnStructure.whitePawns != whitePawns ||
        pawnStructure.blackPawns != blackPawns)
    {
        const Score whitePawnStructureScore = getPawnStructureScore<true>(whitePawns, blackPawns);
        const Score blackPawnStructureScore = getPawnStructureScore<false>(blackPawns, whitePawns);
        pawnStructure.whiteAdvantage = whitePawnStructureScore - blackPawnStructureScore;
        pawnStructure.isValid = true;
        pawnStructure.whitePawns = whitePawns;
        pawnStructure.blackPawns = blackPawns;
    }

    return
        whiteAdvantage +
        whiteKingActivityAdvantage +
        whiteKingSafetyAdvantage +
        pawnStructure.whiteAdvantage;
}

template<bool isWhite>
Score Evaluator::getPawnStructureScore(const U64 friendlyPawns, const U64 enemyPawns)
{
    Score score = 0;

    U64 friendlies = friendlyPawns;
    while (friendlies)
    {
        const Square friendlyPawn = popFirstPiece(friendlies);
        const int rank = getRank(friendlyPawn);
        const int file = getFile(friendlyPawn);
        const U64 fileMask = FILES[file];

        const U64 adjacentFiles = fileMask << 1 & ~FILES[A_FILE] | fileMask >> 1 & ~FILES[H_FILE];
        // add a penalty if this pawn is isolated
        if (!(adjacentFiles & friendlyPawns))
        {
            score += ISOLATED_PAWN_PENALTY;
        }
        // add a penalty if this pawn is doubled
        if (fileMask & friendlies)
        {
            score += DOUBLED_PAWN_PENALTY;
        }
        const U64 passerMask = isWhite ? FULL_BOARD >> (rank + 1) * 8 : FULL_BOARD << (7 - rank) * 8;
        // if this pawn is a passed pawn
        if (!(passerMask & (adjacentFiles | fileMask) & enemyPawns))
        {
            // only the front pawn in a set of stacked pawns should be considered passed
            if (!(passerMask & fileMask & friendlyPawns))
            {
                score += isWhite ? WHITE_PASSED_PAWN_SCORES[friendlyPawn] : BLACK_PASSED_PAWN_SCORES[friendlyPawn];
            }
        }
    }

    return score;
}


float Evaluator::getOpeningWeight()
{
    int knights = 4 - getNumPieces(position.bitboards[WHITE_KNIGHT] | position.bitboards[BLACK_KNIGHT]);
    int bishops = 4 - getNumPieces(position.bitboards[WHITE_BISHOP] | position.bitboards[BLACK_BISHOP]);
    int rooks = 8 - getNumPieces(position.bitboards[WHITE_ROOK] | position.bitboards[BLACK_ROOK]) * 2;
    int queens = 16 - getNumPieces(position.bitboards[WHITE_QUEEN] | position.bitboards[BLACK_QUEEN]) * 8;
    return std::min(1.0f, static_cast<float>(32 - knights - bishops - rooks - queens) / 32.0f);
}



