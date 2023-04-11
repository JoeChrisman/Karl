//
// Created by Joe Chrisman on 4/7/23.
//

#include <ctime>
#include "Cli.h"

namespace
{
    bool isWhiteOnBottom = true;

    void showReady()
    {
        std::cout << "\033[;35m> \033[0m";
    }

    int notationToFile(const char fileChar)
    {
        return (int)(fileChar - 'a');
    }

    int notationToRank(const char rankChar)
    {
        return (int)(rankChar - '1');
    }

    char fileToNotation(const int file)
    {
        return (char)(file + 'a');
    }

    char rankToNotation(const int rank)
    {
        return (char)('1' + rank);
    }

    Square notationToSquare(const std::string& notation)
    {
        return getSquare(notationToRank(notation[1]),
                         notationToFile(notation[0]));
    }

    std::string squareToNotation(const Square square)
    {
        return std::string{
                fileToNotation(getFile(square)),
                rankToNotation((getRank(square)))};
    }

    std::string moveToNotation(const Move move)
    {
        return squareToNotation(getSquareFrom(move)) + squareToNotation(getSquareTo(move));
    }

    struct PerftInfo
    {
        U64 leafNodes;
        U64 totalNodes;
        U64 totalCaptures;
        U64 totalEnPassant;
        U64 totalPromotions;
    };

    void perft(int depth, PerftInfo &info)
    {
        Gen::genMoves();
        if (!depth)
        {
            info.leafNodes += Gen::moveList.size();
            info.totalNodes += Gen::moveList.size();
            return;
        }
        info.totalNodes++;
        const std::vector<Move> moveList = Gen::moveList;
        for (const Move move : moveList)
        {
            if (getPieceCaptured(move) != NULL_PIECE)
            {
                info.totalCaptures++;
            }
            if (getMoveType(move) > 2)
            {
                info.totalPromotions++;
            }
            Position::makeMove(move);
            perft(depth - 1, info);
            Position::unMakeMove(move);
        }
    }
}

int Cli::runKarlCli()
{
    std::cout << R"(
                      (\=,
                    //   .\
                   (( \__  \
____  __.           ))   `\_) .__
|    |/ _| _____   (/      \  | `|
| `    <   \__  \   | _..-'|  |  |
|    |  \   / __ \_  )____(   |  |__.
|____|__ \ (____  / (======)  |  __/
        \/      \/  }======{   \/
                   (________)
    )";
    std::cout << "\n";
    std::cout << "~ Welcome to Karl, a UCI/CLI chess engine\n";
    std::cout << "~ Enter \"help\" for a list of commands\n";
    showReady();

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
            std::cout << "\t\t~ If you wish to play with black on the bottom, see the \"flip\" command\n";
            std::cout << "\t~ \"show\" to show the current position\n";
            std::cout << "\t~ \"who\" to show who's turn it is\n";
            std::cout << "\t~ \"flip\" to flip the board\n";
            std::cout << "\t~ \"pass\" to switch turns without making a move\n";
            std::cout << "\t~ \"move <from><to>\" to make a move\n";
            std::cout << "\t\t~ The fields \"<from>\" and \"<to>\" describe the move in long algebraic notation\n";
            std::cout << "\t\t~ \"<from>\" is where the piece is, \"<to>\" is where to move the piece\n";
            std::cout << "\t\t~ For example, \"move e2e4\" would move the piece on e2 to e4\n";
            std::cout << "\t~ \"moves\" to view a list of legal moves in the current position\n";
            std::cout << "\t~ \"captures\" to view a list of legal captures in the current position\n";
            //std::cout << "\t~ \"perft <plies>\" to run a perft test\n";
            //std::cout << "\t\t~ A perft test is a test that tests the accuracy and performance of the move generator\n";
            //std::cout << "\t\t~ The field \"<ply> must be an integer greater than zero and is the number of half moves to search\n";
            //std::cout << "\t\t~ Visit this webpage to learn more about perft: https://www.chessprogramming.org/Perft\n";
            std::cout << "\t~ \"uci\" to enter UCI mode\n";
            std::cout << "\t~ \"info\" to see additional info about Karl\n";
            std::cout << "\t~ \"help\" to see this list of commands\n";
            showReady();
        }
        else if (command == "info")
        {
            std::cout << "\t~ Version: version " << VERSION << "\n";
            std::cout << "\t~ Author: Joe Chrisman\n";
            showReady();
        }
        else if (command.substr(0, 4) == "load")
        {
            std::string fen = INITIAL_FEN;

            if (command != "load")
            {
                fen = command.substr(5, std::string::npos);
            }
            Position::init(fen);
            std::cout << "~ Successfully loaded position \"" << fen << "\"\n";
            showReady();
        }
        else if (command == "show")
        {
            Position::print(isWhiteOnBottom);
            showReady();
        }
        else if (command == "who")
        {
            if (Position::rights.isWhiteToMove)
            {
                std::cout << "~ It is white to move\n";

            }
            else
            {
                std::cout << "~ It is black to move\n";
            }
            showReady();
        }
        else if (command.substr(0, 8) == "makemove")
        {
            std::string notation = command.substr(9, std::string::npos);
            Square from = notationToSquare(notation.substr(0, 2));
            Square to = notationToSquare(notation.substr(2, 2));
            Gen::genMoves();
            Move legalMove = NULL_MOVE;
            for (const Move move : Gen::moveList)
            {
                if (getSquareFrom(move) == from && getSquareTo(move) == to)
                {
                    legalMove = move;
                    break;
                }
            }
            if (legalMove != NULL_MOVE)
            {
                Position::makeMove(legalMove);
                std::cout << "~ Successfully played move \"" << notation << "\"\n";
            }
            else
            {
                std::cout << "~ Failed to make move \"" << notation << "\"\n";
                std::cout << "~ \"" << notation << "\" is not a legal move\n";
            }
            showReady();
        }
        else if (command == "moves")
        {
            Gen::genMoves();
            std::cout << "~ There are " << Gen::moveList.size() << " legal moves\n";
            for (Move move : Gen::moveList)
            {
                std::cout << "\t~ " << moveToNotation(move) << "\n";
            }
            showReady();
        }
        else if (command == "captures")
        {
            Gen::genCaptures();
            std::cout << "~ There are " << Gen::moveList.size() << " legal captures\n";
            for (Move move : Gen::moveList)
            {
                std::cout << "\t~ " << moveToNotation(move) << "\n";
            }
            showReady();
        }
        else if (command == "flip")
        {
            isWhiteOnBottom = !isWhiteOnBottom;
            std::cout << "~ Successfully flipped the board\n";
            showReady();
        }
        else if (command == "pass")
        {
            Position::rights.isWhiteToMove = !Position::rights.isWhiteToMove;
            std::cout << "~ Successfully gave " << (Position::rights.isWhiteToMove ? "white" : "black") << " the move\n";
            showReady();
        }
        else if (command.substr(0, 5) == "perft")
        {
            // read desired depths
            const std::string::size_type minDepthIndex = command.find_first_of(' ');
            const std::string::size_type maxDepthIndex = command.find_last_of(' ');
            if (minDepthIndex == std::string::npos)
            {
                std::cout << "~ Unrecognized arguments\n";
                std::cout << "~ Run \"help\" for a list of commands\n";
                showReady();
            }
            else
            {
                std::string minDepth = command.substr(minDepthIndex, command.length() - minDepthIndex);
                std::string maxDepth = command.substr(maxDepthIndex, command.length() - maxDepthIndex);
                for (int depth = std::stoi(minDepth); depth <= std::stoi(maxDepth); depth++)
                {
                    timespec start = {};
                    timespec end = {};

                    PerftInfo info = {};
                    std::cout << "~ Running perft test with depth " << depth << "\n";
                    clock_gettime(CLOCK_REALTIME, &start);
                    perft(depth, info);
                    clock_gettime(CLOCK_REALTIME, &end);

                    double startMillis = (start.tv_sec * 1000.0) + (start.tv_nsec / 1000000.0);
                    double endMillis = (end.tv_sec * 1000.0) + (end.tv_nsec / 1000000.0);
                    double msElapsed = endMillis - startMillis;

                    std::cout << "\t~ Depth " << depth << " results:\n";
                    std::cout << "\t\t~ Time: " << msElapsed << "ms\n";
                    std::cout << "\t\t~ kN/s: " << (double)info.totalNodes / msElapsed << "\n";
                    std::cout << "\t\t~ Leaf nodes: " << info.leafNodes << "\n";
                    std::cout << "\t\t~ Nodes: " << info.totalNodes << "\n";
                    std::cout << "\t\t~ Promotions: " << info.totalEnPassant << "\n";
                    std::cout << "\t\t~ Captures: " << info.totalCaptures << "\n";
                    std::cout << "\t\t~ En passant captures: " << info.totalEnPassant << "\n";
                }
                std::cout << "~ Perft test complete\n";
                showReady();
            }
        }
        else if (command == "uci")
        {
            runKarlUci();
        }
        else
        {
            std::cout << "~ Unrecognized command\n";
            std::cout << "~ Run \"help\" for a list of commands\n";
            showReady();
        }
    }
    return 0;
}

int Cli::runKarlUci()
{
    return 0;
}
