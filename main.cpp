#include "Cli.h"

int main()
{
    srand(time(0));
    Gen::init();
    Magics::init();
    return Cli::runKarlCli();
}

// some scrapped debug code will live here temporarily
/*
void printBitboard(const U64 board)
{
    for (Square square = A8; square <= H1; square++)
    {
        if (FILE_MASKS[A_FILE] & getBoard(square))
        {
            std::cout << "\n";
        }
        if (board & getBoard(square))
        {
            std::cout << " 1 ";
        }
        else
        {
            std::cout << " . ";
        }
    }
    std::cout << "\n";
}
 */
