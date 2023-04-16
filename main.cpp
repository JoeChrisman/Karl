#include "Cli.h"

int main()
{
    srand(time(0));
    Gen::init();
    Magics::init();
    return Cli::runKarlCli();
}

