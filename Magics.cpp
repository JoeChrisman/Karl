//
// Created by Joe Chrisman on 4/8/23.
//

#include "Magics.h"

Magics::Magics()
: ordinalMagics{0}, cardinalMagics{0}, ordinalAttacks{0}, cardinalAttacks{0}
{
    for (Square square = A8; square <= H1; square++)
    {
        cardinalMagics[square].blockers = getRookBlockers(square);
        ordinalMagics[square].blockers = getBishopBlockers(square);

        cardinalMagics[square].magic = getMagicNumber(square, true);
        ordinalMagics[square].magic = getMagicNumber(square, false);
    }
}


U64 Magics::random64()
{
    return ((U64)(rand()) << 32) | rand();
}

U64 Magics::getRookAttacks(Square from, U64 blockers, bool captures)
{
    U64 attacks = EMPTY_BOARD;

    int rank = getRank(from);
    int file = getFile(from);
    int targetRank = rank;
    while (targetRank++ < EIGHTH_RANK)
    {
        U64 attack = getBoard(getSquare(targetRank, file));
        if (blockers & attack)
        {
            if (captures)
            {
                attacks |= attack;
            }
            break;
        }
        attacks |= attack;
    }
    int targetFile = file;
    while (targetFile++ < H_FILE)
    {
        U64 attack = getBoard(getSquare(rank, targetFile));
        if (blockers & attack)
        {
            if (captures)
            {
                attacks |= attack;
            }
            break;
        }
        attacks |= attack;
    }
    targetRank = rank;
    while (targetRank-- > FIRST_RANK)
    {
        U64 attack = getBoard(getSquare(targetRank, file));
        if (blockers & attack)
        {
            if (captures)
            {
                attacks |= attack;
            }
            break;
        }
        attacks |= attack;
    }
    targetFile = file;
    while (targetFile-- > A_FILE)
    {
        U64 attack = getBoard(getSquare(rank, targetFile));
        if (blockers & attack)
        {
            if (captures)
            {
                attacks |= attack;
            }
            break;
        }
        attacks |= attack;
    }

    return attacks;
}

U64 Magics::getBishopAttacks(Square from, U64 blockers, bool captures)
{
    U64 attacks = EMPTY_BOARD;

    U64 attack = getBoard(from);
    while (!(attack & RANKS[EIGHTH_RANK]) && !(attack & FILES[H_FILE]))
    {
        attack = northEast(attack);
        if (blockers & attack)
        {
            if (captures)
            {
                attacks |= attack;
            }
            break;
        }
        attacks |= attack;
    }
    attack = getBoard(from);
    while (!(attack & RANKS[FIRST_RANK]) && !(attack & FILES[H_FILE]))
    {
        attack = southEast(attack);
        if (blockers & attack)
        {
            if (captures)
            {
                attacks |= attack;
            }
            break;
        }
        attacks |= attack;
    }
    attack = getBoard(from);
    while (!(attack & RANKS[FIRST_RANK]) && !(attack & FILES[A_FILE]))
    {
        attack = southWest(attack);
        if (blockers & attack)
        {
            if (captures)
            {
                attacks |= attack;
            }
            break;
        }
        attacks |= attack;
    }
    attack = getBoard(from);
    while (!(attack & RANKS[EIGHTH_RANK]) && !(attack & FILES[A_FILE]))
    {
        attack = northWest(attack);
        if (blockers & attack)
        {
            if (captures)
            {
                attacks |= attack;
            }
            break;
        }
        attacks |= attack;
    }
    return attacks;
}

U64 Magics::getRookBlockers(Square from)
{
    int rank = getRank(from);
    int file = getFile(from);

    U64 leftAndRight = FILES[A_FILE] | FILES[H_FILE];
    U64 upAndDown = RANKS[FIRST_RANK] | RANKS[EIGHTH_RANK];

    U64 endpoints = EMPTY_BOARD;
    endpoints |= RANKS[rank] & leftAndRight;
    endpoints |= FILES[file] & upAndDown;
    endpoints &= ~getBoard(from);
    return getRookAttacks(from, endpoints, false) & ~getBoard(from);
}

U64 Magics::getBishopBlockers(Square from)
{
    U64 endpoints = FILES[A_FILE] |
                    FILES[H_FILE] |
                    RANKS[FIRST_RANK] |
                    RANKS[EIGHTH_RANK];
    endpoints &= ~getBoard(from);
    return getBishopAttacks(from, endpoints, false) & ~getBoard(from);

}

U64 Magics::getMagicNumber(Square square, bool isCardinal)
{
    U64 blockerMask = isCardinal ? cardinalMagics[square].blockers : ordinalMagics[square].blockers;
    int numPermutations = 1 << getNumPieces(blockerMask);

    int maxPermutations = isCardinal ? 4096 : 512;

    U64 attacks[maxPermutations];
    U64 blockers[maxPermutations];
    U64 attacksSeen[maxPermutations];

    for (int permutation = 0; permutation < numPermutations; permutation++)
    {
        U64 actualBlockers = 0;
        int blockerPermutation = permutation;
        U64 possibleBlockers = blockerMask;
        while (possibleBlockers)
        {
            U64 blocker = getBoard(popFirstPiece(possibleBlockers));
            if (blockerPermutation % 2) {
                actualBlockers |= blocker;
            }
            blockerPermutation >>= 1;
        }
        blockers[permutation] = actualBlockers;
        attacks[permutation] = isCardinal ? getRookAttacks(square, actualBlockers, true)
                                          : getBishopAttacks(square, actualBlockers, true);
    }

    int tries = 1000000;
    while (tries--)
    {
        for (int i = 0; i < maxPermutations; i++)
        {
            attacksSeen[i] = 0;
        }
        uint64_t magic = random64() & random64() & random64();

        for (int permutation = 0; permutation < numPermutations; permutation++)
        {
            uint16_t hash = blockers[permutation] * magic >> (isCardinal ? 52 : 55);
            if (!attacksSeen[hash])
            {
                attacksSeen[hash] = attacks[permutation];
            }
            else if (attacksSeen[hash] != attacks[permutation])
            {
                magic = 0;
                break;
            }
        }
        if (magic)
        {
            for (int i = 0; i < maxPermutations; i++)
            {
                if (isCardinal)
                {
                    cardinalAttacks[square][i] = attacksSeen[i];
                }
                else
                {
                    ordinalAttacks[square][i] = attacksSeen[i];
                }
            }

            return magic;
        }
    }
    std::cout << "~ Critical Error. Magic number generation failed on square " << square << ".\n";
}

