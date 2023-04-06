#include <iostream>
#include "MoveGenerator.h"

int runCli()
{
    std::cout << "Welcome to KarlCLI, a command line interface for interacting with Karl.\n";
    Position position;

    std::string command;
    while (std::getline(std::cin, command))
    {

        if (command == "quit")
        {
            return 0;
        }
        else if (command.substr(0, 12) == "loadposition")
        {
            std::string fen = command.substr(13, std::string::npos);
            position = Position(fen);
            MoveGenerator moveGenerator(position);
            std::cout << "successfully loaded position " << fen << ".\n";
        }
        else if (command == "showposition")
        {
            position.printPosition();
        }

    }
}

int main()
{
    while (true)
    {
        std::string command;
        std::cin >> command;
        if (command == "quit")
        {
            return 0;
        }
        else if (command == "uci")
        {
            // enter uci loop later
            return 0;
        }
        else if (command == "cli")
        {
            return runCli();
        }
    }
}
