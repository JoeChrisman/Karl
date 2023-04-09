//
// Created by Joe Chrisman on 4/7/23.
//

#include "Cli.h"

bool Cli::isWhiteOnBottom = true;
Position* Cli::position = new Position(INITIAL_FEN);
MoveGenerator* Cli::moveGenerator = new MoveGenerator(*Cli::position);

int Cli::notationToFile(const char fileChar)
{
    return (int)(fileChar - 'a');
}

int Cli::notationToRank(const char rankChar)
{
    return (int)(rankChar - '1');
}

char Cli::fileToNotation(const int file)
{
    return (char)(file + 'a');
}

char Cli::rankToNotation(const int rank)
{
    return (char)('1' + rank);
}

Square Cli::notationToSquare(const std::string& notation)
{
    return getSquare(notationToRank(notation[1]),
                     notationToFile(notation[0]));
}

std::string Cli::squareToNotation(const Square square)
{
    return std::string{
        fileToNotation(getFile(square)),
        rankToNotation((getRank(square)))};
}

std::string Cli::moveToNotation(const Move move)
{
    return squareToNotation(getSquareFrom(move)) + squareToNotation(getSquareTo(move));
}

int Cli::run()
{
    std::cout << R"(
                                                  (\=,
                                                //   .\
                                               (( \__  \
      ____  __.                   .__           ))   `\_)
      |    |/ _| _____    _______  |  |        (/      \
      |      <   \__  \   \_  __ \ |  |         | _..-'|
      |    |  \   / __ \_  |  | \/ |  |__        )____(
      |____|__ \ (____  /  |__|    |____/       (======)
              \/      \/                        }======{
                                               (________)
 )";
    std::cout << "\n";
    std::cout << "~ Welcome to Karl, a UCI/CLI chess engine\n";
    std::cout << "~ Enter \"help\" for a list of commands\n";
    std::cout << "> ";

    std::string command;
    while (std::getline(std::cin, command))
    {
        if (command == "quit" || command == "exit")
        {
            std::cout << "~ Bye!\n";
            break;
        }
        else if (command == "help")
        {
            std::cout << "\t~ \"exit\" or \"quit\" to exit the CLI\n";
            std::cout << "\t~ \"load <FEN>\" to load a position into the engine\n";
            std::cout << "\t\t~ The default position is the initial position playing as white\n";
            std::cout << "\t\t~ If you omit the FEN, the starting position for white will be loaded\n";
            std::cout << "\t\t~ This program only accepts FEN strings where white is on the bottom and black is on the top\n";
            std::cout << "\t\t~ If you wish to play with black on the bottom, see the \"flip\" command\n";
            std::cout << "\t~ \"show\" to show the current position\n";
            std::cout << "\t~ \"who\" to show who's turn it is\n";
            std::cout << "\t~ \"move <from><to>\" to make a move\n";
            std::cout << "\t\t~ The fields \"<from>\" and \"<to>\" describe the move in long algebraic notation\n";
            std::cout << "\t\t~ For example, if you wanted to play e4 out of the starting position, \"makemove e2e4\" would suffice\n";
            std::cout << "\t~ \"moves\" to view a list of legal moves in the current position\n";
            std::cout << "\t~ \"captures\" to view a list of legal captures in the current position\n";
            std::cout << "\t~ \"flip\" to flip the board\n";
            //std::cout << "\t~ \"perft <plies>\" to run a perft test\n";
            //std::cout << "\t\t~ A perft test is a test that tests the accuracy and performance of the move generator\n";
            //std::cout << "\t\t~ The field \"<ply> must be an integer greater than zero and is the number of half moves to search\n";
            //std::cout << "\t\t~ Visit this webpage to learn more about perft: https://www.chessprogramming.org/Perft\n";
            std::cout << "\t~ \"uci\" to enter UCI mode\n";
            std::cout << "\t~ \"info\" to see additional info about Karl\n";
            std::cout << "\t~ \"help\" to see this list of commands\n";
            std::cout << "> ";
        }
        else if (command == "info")
        {
            std::cout << "\t~ Version: version " << VERSION << "\n";
            std::cout << "\t~ Author: Joe Chrisman\n";
            std::cout << "> ";
        }
        else if (command.substr(0, 4) == "load")
        {
            std::string fen = INITIAL_FEN;

            if (command != "load")
            {
                fen = command.substr(5, std::string::npos);
            }
            delete moveGenerator;
            delete position;
            position = new Position(fen);
            moveGenerator = new MoveGenerator(*position);
            std::cout << "~ Successfully loaded position \"" << fen << "\"\n";
            std::cout << "> ";
        }
        else if (command == "show")
        {
            position->printPosition(isWhiteOnBottom);
            std::cout << "> ";
        }
        else if (command == "who")
        {
            if (position->isWhiteToMove)
            {
                std::cout << "~ It is white to move\n";

            }
            else
            {
                std::cout << "~ It is black to move\n";
            }
            std::cout << "> ";
        }
        else if (command.substr(0, 8) == "makemove")
        {
            std::string notation = command.substr(9, std::string::npos);
            Square from = notationToSquare(notation.substr(0, 2));
            Square to = notationToSquare(notation.substr(2, 2));
            moveGenerator->generateMoves();
            Move legalMove = NULL_MOVE;
            for (const Move move : moveGenerator->moveList)
            {
                if (getSquareFrom(move) == from && getSquareTo(move) == to)
                {
                    legalMove = move;
                    break;
                }
            }
            if (legalMove != NULL_MOVE)
            {
                position->makeMove(legalMove);
                std::cout << "~ Successfully played move \"" << notation << "\"\n";
            }
            else
            {
                std::cout << "~ Failed to make move \"" << notation << "\"\n";
                std::cout << "~ \"" << notation << "\" is not a legal move\n";
            }
            std::cout << "> ";
        }
        else if (command == "moves")
        {
            moveGenerator->generateMoves();
            std::cout << "~ There are " << moveGenerator->moveList.size() << " legal moves\n";
            for (Move move : moveGenerator->moveList)
            {
                std::cout << "\t~ " << moveToNotation(move) << "\n";
            }
            std::cout << "> ";
        }
        else if (command == "captures")
        {
            moveGenerator->generateCaptures();
            std::cout << "~ There are " << moveGenerator->moveList.size() << " legal captures\n";
            for (Move move : moveGenerator->moveList)
            {
                std::cout << "\t~ " << moveToNotation(move) << "\n";
            }
            std::cout << "> ";
        }
        else if (command == "flip")
        {
            isWhiteOnBottom = !isWhiteOnBottom;
            std::cout << "~ Successfully flipped the board\n";
            std::cout << "> ";
        }
        else if (command == "perft")
        {

        }
        else if (command == "uci")
        {
            runUci();
        }
        else
        {
            std::cout << "~ Unrecognized command\n";
            std::cout << "~ Run \"help\" for a list of commands\n";
            std::cout << "> ";
        }
    }
    delete moveGenerator;
    delete position;
    return 0;
}


int Cli::runUci()
{
    return 0;
}
