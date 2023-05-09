//
// Created by Joe Chrisman on 4/11/23.
//

#include "Eval.h"

Score Eval::evaluate(const Score material, const Score midgamePlacement)
{
    return material + midgamePlacement;
}
