//
// Created by Joe Chrisman on 4/7/23.
//

#include <ctime>
#include "Cli.h"
#include "Notation.h"

namespace
{
    bool isWhiteOnBottom = true;

    void showReady()
    {
        std::cout << "\033[;35m> \033[0m" << std::flush;
    }

    struct PerftInfo
    {
        U64 totalNodes;

        U64 leafNodes;
        U64 leafCaptures;
        U64 leafEnPassants;
        U64 leafPromotions;
        U64 leafCastles;
        U64 leafCheckmates;
        U64 leafStalemates;
        U64 leafChecks;
    };

    void perft(int depth, PerftInfo &info)
    {
        if (!depth)
        {
            info.totalNodes++;
            info.leafNodes++;

            /*Gen::genMoves();
            if (Gen::moveList.empty())
            {
                if (Gen::isInCheck())
                {
                    info.leafChecks++;
                    info.leafCheckmates++;
                }
                else
                {
                    info.leafStalemates++;
                }
            }
            else
            {
                if (Gen::isInCheck())
                {
                    info.leafChecks++;
                }
            }*/
            return;
        }
        info.totalNodes++;
        Position::Rights rightsCopy = Position::rights;
        Gen::genMoves();
        const std::vector<Move> moveList = Gen::moveList;
        for (const Move move : moveList)
        {
            if (depth == 1)
            {
                if (Moves::getCaptured(move) != NULL_PIECE)
                {
                    info.leafCaptures++;
                }
                if (move & Moves::EN_PASSANT)
                {
                    info.leafEnPassants++;
                }
                if (move & (Moves::LONG_CASTLE | Moves::SHORT_CASTLE))
                {
                    info.leafCastles++;
                }
                if (Moves::getPromoted(move) != NULL_PIECE)
                {
                    info.leafPromotions++;
                }
            }
            Position::makeMove(move);
            perft(depth - 1, info);
            Position::unMakeMove(move, rightsCopy);
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
            std::cout << "\t~ This is the user manual for the Karl Chess Engine Command Line Interface\n";
            std::cout << "\t~ Version: version " << VERSION << "\n";
            std::cout << "\t~ Author: Joe Chrisman\n";
            std::cout << "\t\t~ A field in angle braces, like \"<field>\", means that field is required\n";
            std::cout << "\t\t~ A field in curly braces, like \"{field}\", means that field is optional\n";
            std::cout << "\t\t~ This is a list of all commands\n\n";
            std::cout << "\t~ \"exit\" or \"quit\" to exit the CLI\n";
            std::cout << "\t~ \"load {fen}\" to load a position into the engine\n";
            std::cout << "\t\t~ The default position is an empty board\n";
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
            std::cout << "\t\t~ To castle, use the king's starting and ending squares\n";
            std::cout << "\t\t~ To promote, append the promotion type to the end of the move, such as \"e7e8q\"\n";
            std::cout << "\t~ \"moves\" to view a list of legal moves in the current position\n";
            std::cout << "\t~ \"captures\" to view a list of legal captures in the current position\n";
            std::cout << "\t~ \"perft <min> {max}\" to run a perft test\n";
            std::cout << "\t\t~ A perft test is a test that tests the accuracy and performance of the move generator\n";
            std::cout << "\t\t~ The field \"<min>\" is the lowest depth to search to\n";
            std::cout << "\t\t~ The field \"{max}\" is the highest depth to search to\n";
            std::cout << "\t\t~ All depths between \"<min>\" and \"{max}\" will be searched\n";
            std::cout << "\t\t~ If \"{max}\" is omitted, only the \"<min>\" depth will be searched\n";
            std::cout << "\t~ \"uci\" to enter UCI mode\n";
            std::cout << "\t~ \"help\" to see this list of commands\n";
            showReady();
        }
        else if (command.substr(0, 4) == "load")
        {
            std::string fen = INITIAL_FEN;

            if (command != "load")
            {
                fen = command.substr(5, std::string::npos);
            }
            if (Position::init(fen))
            {
                std::cout << "~ Successfully loaded FEN string \"" << fen << "\"\n";
            }
            else
            {
                std::cout << "~ Failed to load the position\n";
                std::cout << "~ Invalid FEN string \"" << fen << "\"\n";
            }
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
            Gen::genMoves();
            Move legalMove = Moves::NULL_MOVE;
            std::cout << "\n";
            for (const Move move : Gen::moveList)
            {
                std::cout << Notation::moveToStr(move) << ", ";
                if (Notation::moveToStr(move) == notation)
                {
                    legalMove = move;
                    break;
                }
            }
            if (legalMove != Moves::NULL_MOVE)
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
                std::cout << "\t~ " << Notation::moveToStr(move) << "\n";
            }
            showReady();
        }
        else if (command == "captures")
        {
            Gen::genCaptures();
            std::cout << "~ There are " << Gen::moveList.size() << " legal captures\n";
            for (Move move : Gen::moveList)
            {
                std::cout << "\t~ " << Notation::moveToStr(move) << "\n";
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
            const std::string::size_type minDepthIndex = command.find_first_of(' ') + 1;
            const std::string::size_type maxDepthIndex = command.find_last_of(' ') + 1;
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
                    
                    double branchingFactor = (double)(info.totalNodes - 1) / (double)(info.totalNodes - info.leafNodes);

                    std::cout << "\t~ Depth " << depth << " results:\n";
                    std::cout << "\t\t~ Time: " << msElapsed << "ms\n";
                    std::cout << "\t\t~ kN/s: " << (double)info.totalNodes / msElapsed << "\n";
                    std::cout << "\t\t~ Branching factor: " <<  branchingFactor << "\n";
                    std::cout << "\t\t~ Total nodes: " << info.totalNodes << "\n";
                    std::cout << "\t\t~ Leaf nodes: " << info.leafNodes << "\n";
                    std::cout << "\t\t~ Leaf checks: " << info.leafChecks << "\n";
                    std::cout << "\t\t~ Leaf checkmates: " << info.leafCheckmates << "\n";
                    std::cout << "\t\t~ Leaf stalemates: " << info.leafStalemates << "\n";
                    std::cout << "\t\t~ Leaf promotions: " << info.leafPromotions << "\n";
                    std::cout << "\t\t~ Leaf captures: " << info.leafCaptures << "\n";
                    std::cout << "\t\t~ Leaf castles: " << info.leafCastles << "\n";
                    std::cout << "\t\t~ Leaf en passants: " << info.leafEnPassants << "\n";
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
