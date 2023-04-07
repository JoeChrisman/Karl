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
        else if (command.substr(0, 8) == "makemove")
        {
            std::string notation = command.substr(9, std::string::npos);
            Square from = toSquare(notation.substr(0, 2));
            Square to = toSquare(notation.substr(2, 2));
            position.makeMove(createMove(
                    NORMAL,
                    position.pieces[from],
                    position.pieces[to],
                    from,
                    to
            ));
        }
        else
        {
            std::cout << "~ Unrecognized command.\n";
            std::cout << "~ Run \"help\" for a list of commands.\n";
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
