#include <iostream>
#include "MoveGenerator.h"

int runCli()
{
    std::cout << "~ Karl CLI enabled.\n";
    std::cout << R"(____  __.                   .__      _________   .__
|    |/ _| _____    _______  |  |     \_   ___ \  |  |__     ____     ______   ______
|      <   \__  \   \_  __ \ |  |     /    \  \/  |  |  \  _/ __ \   /  ___/  /  ___/
|    |  \   / __ \_  |  | \/ |  |__   \     \____ |   Y  \ \  ___/   \___ \   \___ \
|____|__ \ (____  /  |__|    |____/    \______  / |___|  /  \___  > /____  > /____  >
        \/      \/                            \/       \/       \/       \/       \/ )";
    std::cout << "\n";
    std::cout << "~ Version " << VERSION << "\n";
    std::cout << "~ A UCI/CLI Chess Engine by Joe Chrisman.\n";
    std::cout << "~ Run help for a list of commands.\n";
    Position position;
    MoveGenerator moveGenerator;

    std::string command;
    std::cin.ignore();
    while (std::getline(std::cin, command))
    {
        if (command == "quit")
        {
            return 0;
        }
        else if (command == "help")
        {
            std::cout << "\t~ This is the user manual for Karl Chess version " << VERSION << ".\n";
            std::cout << "\t~ \"quit\" to exit the CLI.\n";
            std::cout << "\t~ \"loadposition <FEN>\" to load a position into the engine.\n";
            std::cout << "\t\t~ If you omit the FEN, the starting position for white will be loaded.\n";
            std::cout << "\t\t~ If you use \"loadposition white\", the starting position for white will be loaded.\n";
            std::cout << "\t\t~ If you use \"loadposition black\", the starting position for black will be loaded.\n";
            std::cout << "\t~ \"displayposition\" to display the current position.\n";
            std::cout << "\t~ \"move <from><to>\" to make a move.\n";
            std::cout << "\t\t~ The fields \"<from>\" and \"<to>\" describe the source square and target square in algebraic notation.\n";
            std::cout << "\t\t~ For example, if we wanted to play e4 out of the starting position, \"makemove e2e4\" would suffice.\n";
            std::cout << "\t~ \"help\" to see this list of commands.\n";
        }
        else if (command.substr(0, 12) == "loadposition")
        {
            std::string fen;
            if (command == "loadposition")
            {
                fen = INITIAL_WHITE_FEN;
            }
            else
            {
                fen = command.substr(13, std::string::npos);
                if (fen == "white" || fen == "WHITE")
                {
                    fen = INITIAL_WHITE_FEN;
                }
                else if (fen == "black" || fen == "BLACK")
                {
                    fen = INITIAL_BLACK_FEN;
                }
            }
            position = Position(fen);
            moveGenerator = MoveGenerator(position);
            std::cout << "~ Successfully loaded position \"" << fen << "\".\n";
        }
        else if (command == "displayposition")
        {
            position.printPosition();
            std::cout << "~ Successfully displayed position.\n";

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
            std::cout << "~ Successfully played move \"" << notation << "\".\n";
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
