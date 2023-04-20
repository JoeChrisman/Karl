#include "Cli.h"

int main()
{
    srand(3297);
    Zobrist::init();
    srand(time(nullptr));
    Magics::init();
    Gen::init();
    return Cli::runKarlCli();
}

