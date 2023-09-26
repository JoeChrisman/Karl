//
// Created by Joe Chrisman on 9/24/23.
//

#include <algorithm>
#include "Eval.h"

inline constexpr Score MAX_MATERIAL = 8240;
inline constexpr int KING_ACTIVITY_WEIGHT = 100;

Evaluator::Evaluator(Position& position, MoveGen& moveGen) :
    position(position), moveGen(moveGen)
{
}

Score Evaluator::evaluate()
{
    const Score whiteMaterial = countMaterial<true>();
    const Score blackMaterial = countMaterial<false>();

    const Score advantage = whiteMaterial - blackMaterial + position.midgamePlacementScore;

    const Square whiteKing = getSquare(position.bitboards[WHITE_KING]);
    const Square blackKing = getSquare(position.bitboards[BLACK_KING]);
    const int whiteKingRank = getRank(whiteKing);
    const int whiteKingFile = getFile(whiteKing);
    const int blackKingRank = getRank(blackKing);
    const int blackKingFile = getFile(blackKing);

    const int whiteKingActivityBonus = getKingActivityBonus(whiteKingRank, whiteKingFile);
    const int blackKingActivityBonus = getKingActivityBonus(blackKingRank, blackKingFile);

    const int kingActivityBonus = (whiteKingActivityBonus - blackKingActivityBonus) * KING_ACTIVITY_WEIGHT;

    const Score totalMaterial = whiteMaterial + blackMaterial;

    const float materialLost = std::max(0.0f, static_cast<float>(MAX_MATERIAL - totalMaterial));
    const float percentageLost = materialLost / MAX_MATERIAL;
    const float endgameWeight = percentageLost * percentageLost;

    const Score endgameScore = static_cast<Score>(static_cast<float>(kingActivityBonus) * endgameWeight);
    return advantage + endgameScore;
}

template<bool isWhite>
Score Evaluator::countMaterial()
{
    Score material = 0;

    Piece piece = isWhite ? WHITE_PAWN : BLACK_PAWN;
    const Piece highPiece = isWhite ? WHITE_QUEEN : BLACK_QUEEN;
    for (; piece <= highPiece; ++piece)
    {
        material += getNumPieces(position.bitboards[piece]) * PIECE_SCORES[piece];
    }
    return std::abs(material);
}

inline int Evaluator::getKingActivityBonus(const int kingRank, const int kingFile)
{
    const int centerRankDistance = std::max(3 - kingRank, kingRank - 4);
    const int centerFileDistance = std::max(3 - kingFile, kingFile - 4);
    return 6 - (centerRankDistance + centerFileDistance);
}

