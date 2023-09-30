//
// Created by Joe Chrisman on 9/24/23.
//

#include <algorithm>
#include "Eval.h"

Evaluator::Evaluator(Position& position, MoveGen& moveGen) :
    position(position), moveGen(moveGen)
{
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

    return whiteAdvantage + whiteKingActivityAdvantage + whiteKingSafetyAdvantage;
}

float Evaluator::getOpeningWeight()
{
    int knights = 4 - getNumPieces(position.bitboards[WHITE_KNIGHT] | position.bitboards[BLACK_KNIGHT]);
    int bishops = 4 - getNumPieces(position.bitboards[WHITE_BISHOP] | position.bitboards[BLACK_BISHOP]);
    int rooks = 8 - getNumPieces(position.bitboards[WHITE_ROOK] | position.bitboards[BLACK_ROOK]) * 2;
    int queens = 16 - getNumPieces(position.bitboards[WHITE_QUEEN] | position.bitboards[BLACK_QUEEN]) * 8;
    return std::min(1.0f, static_cast<float>(32 - knights - bishops - rooks - queens) / 32.0f);
}



