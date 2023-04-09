#include "Cli.h"

int main()
{
    srand(time(0));
    MagicSliders::init();
    return Cli::run();
}
