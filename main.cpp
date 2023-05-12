#include "Cli.h"

int main()
{
    srand(time(nullptr));
    Zobrist zobrist;
    Magics magics;

    Cli cli(zobrist, magics);
    return cli.runCli();
}

