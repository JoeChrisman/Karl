//
// Created by Joe Chrisman on 4/7/23.
//

#include <ctime>
#include <sstream>
#include <fstream>
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
        U64 totalSplit;

        U64 nodes;
        U64 captures;
        U64 enPassants;
        U64 promotions;
        U64 castles;
    };

    void printPerftInfo(const PerftInfo& info, const int depth, const double msElapsed)
    {
        std::cout << "\t~ Depth " << depth << " perft results\n";
        std::cout << "\t~ =========================\n";
        std::cout << "\t~ Time        | " << msElapsed << "ms\n";
        std::cout << "\t~ kN/s        | " << (double)info.totalNodes / msElapsed << "\n";
        std::cout << "\t~ Nodes       | " << info.nodes << "\n";
        std::cout << "\t~ Promotions  | " << info.promotions << "\n";
        std::cout << "\t~ Captures    | " << info.captures << "\n";
        std::cout << "\t~ Castles     | " << info.castles << "\n";
        std::cout << "\t~ En passants | " << info.enPassants << "\n";
        std::cout << "\t~ =========================\n";
    }

    void perft(int depth, PerftInfo &info, int splitDepth = -1)
    {
        info.totalNodes++;

        if (!depth)
        {
            info.nodes++;
            if (splitDepth != -1)
            {
                info.totalSplit++;
            }
            return;
        }
        Position::Rights rightsCopy = Position::rights;
        Gen::genMoves();
        Move moveList[256];
        std::memcpy(moveList, Gen::moveList, sizeof(Gen::moveList));
        for (Move move : moveList)
        {
            if (move == Moves::NULL_MOVE)
            {
                break;
            }
            if (depth == 1)
            {
                if (Moves::getCaptured(move) != NULL_PIECE)
                {
                    info.captures++;
                }
                if (move & Moves::EN_PASSANT)
                {
                    info.enPassants++;
                }
                if (move & (Moves::LONG_CASTLE | Moves::SHORT_CASTLE))
                {
                    info.castles++;
                }
                if (Moves::getPromoted(move) != NULL_PIECE)
                {
                    info.promotions++;
                }
            }
            Position::makeMove(move);
            if (splitDepth == depth)
            {
                info.totalSplit = 0;
                perft(depth - 1, info, splitDepth);
                std::cout << "~ " << Notation::moveToStr(move) << ": " << info.totalSplit << "\n";
            }
            else
            {
                perft(depth - 1, info, splitDepth);
            }

            Position::unMakeMove(move, rightsCopy);
        }
    }

    void runPerftSuite()
    {
        int passes = 0;
        int failures = 0;

        std::ifstream tests("../perftSuite.txt");

        U64 totalNodes = 0;
        timespec start = {};
        timespec end = {};
        clock_gettime(CLOCK_REALTIME, &start);

        // read each test
        std::string test;
        while (std::getline(tests, test))
        {
            std::vector<std::string> testContents;
            size_t index = test.find(';');
            testContents.push_back(test.substr(0, index - 1));
            test = test.substr(index + 1, test.length() - index);
            // split each test string by semicolon delimiter
            do
            {
                index = test.find(';');
                testContents.push_back(test.substr(2, index - 2));
                test = test.substr(index + 1, test.length() - index);
            }
            while (index != std::string::npos);

            // the first string read was the fen string
            if (!Position::init(testContents[0]))
            {
                std::cout << "~ Invalid FEN string found in file \"perftSuite.txt\"\n";
                return;
            }

            std::cout << "~ Running perft unit tests on position " << testContents[0] << "\n";
            // the rest of the strings are depths and node counts
            for (int depth = 1; depth < testContents.size(); depth++)
            {
                PerftInfo info = {};
                perft(depth, info);
                totalNodes += info.totalNodes;
                int nodes = std::stoi(testContents[depth]);
                if (info.nodes == nodes)
                {
                    std::cout << "~ [PASS] Perft unit test at depth " << depth << " passed with " << info.nodes << " nodes\n";
                    passes++;
                }
                else
                {
                    std::cout << "~ [FAIL] Perft unit test at depth " << depth << " failed with " << info.nodes << " nodes\n";
                    std::cout << "\t~ Expected " << nodes << " nodes, but found " << info.nodes << "\n";
                    failures++;
                }
            }
        }
        clock_gettime(CLOCK_REALTIME, &end);

        double startMillis = (start.tv_sec * 1000.0) + (start.tv_nsec / 1000000.0);
        double endMillis = (end.tv_sec * 1000.0) + (end.tv_nsec / 1000000.0);
        double msElapsed = endMillis - startMillis;

        std::cout << "~ Perft suite run complete\n";
        std::cout << "\t~ =========================\n";
        std::cout << "\t~ Tests ran    | " << passes + failures << "\n";
        std::cout << "\t~ Time         | " << msElapsed / 1000 << "s\n";
        std::cout << "\t~ kN/s         | " << (double)totalNodes / msElapsed << "\n";
        std::cout << "\t~ Tests passed | " << passes << "\n";
        std::cout << "\t~ Tests failed | " << failures << "\n";
        std::cout << "\t~ =========================\n";
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
            std::cout << "\t\t~ A field in rounded braces, like \"(field)\", means that field is an optional flag\n";
            std::cout << "\t\t~ This is a list of all commands\n\n";
            std::cout << "\t~ \"exit\" or \"quit\" to exit the CLI\n";
            std::cout << "\t~ \"load {fen}\" to load a position into the engine\n";
            std::cout << "\t\t~ The default position is an empty board\n";
            std::cout << "\t\t~ If you omit the FEN, the starting position for white will be loaded\n";
            std::cout << "\t\t~ If you wish to play with black on the bottom, see the \"flip\" command\n";
            std::cout << "\t~ \"show\" to show the current position\n";
            std::cout << "\t~ \"go\" to start engine analysis\n";
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
            std::cout << "\t~ \"perft (split) (suite) {min} {max}\" to run a perft test\n";
            std::cout << "\t\t~ A perft test is a test that tests the accuracy and performance of the move generator\n";
            std::cout << "\t\t~ The field \"{min}\" is the lowest depth to search to\n";
            std::cout << "\t\t~ The field \"{max}\" is the highest depth to search to\n";
            std::cout << "\t\t~ All depths between \"{min}\" and \"{max}\" will be searched\n";
            std::cout << "\t\t~ If \"{max}\" is omitted, only the \"{min}\" depth will be searched\n";
            std::cout << "\t\t~ If \"{max}\" is omitted, and the \"(split)\" flag is present, split mode will be enabled\n";
            std::cout << "\t\t~ split mode only accepts one depth value and shows the number of leaf nodes after each move\n";
            std::cout << "\t\t~ If \"{max}\" and \"{min}\" are omitted, and the \"(suite)\" flag is present, a test suite will be run\n";
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
        else if (command == "go")
        {
            std::cout << "~ Best move: " << Notation::moveToStr(Search::getBestMove()) << "\n";
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
            for (const Move move : Gen::moveList)
            {
                if (move == Moves::NULL_MOVE)
                {
                    break;
                }
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
            for (Move move : Gen::moveList)
            {
                if (move == Moves::NULL_MOVE)
                {
                    break;
                }
                std::cout << "\t~ " << Notation::moveToStr(move) << "\n";
            }
            showReady();
        }
        else if (command == "captures")
        {
            Gen::genCaptures();
            for (Move move : Gen::moveList)
            {
                if (move == Moves::NULL_MOVE)
                {
                    break;
                }
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
            std::stringstream stream(command);
            std::string arg1;
            std::string arg2;
            stream >> arg1; // skip "perft"
            stream >> arg1; // read "split" or a number
            stream >> arg2; // read a number

            if (arg1 == "suite")
            {
                runPerftSuite();
                showReady();
                continue;
            }

            int minDepth = -1;
            int maxDepth = -1;
            try
            {
                if (arg1 == "split")
                {
                    maxDepth = std::stoi(arg2);
                }
                else
                {
                    minDepth = std::stoi(arg1);
                    if (arg2.empty())
                    {
                        maxDepth = minDepth;
                    }
                    else
                    {
                        maxDepth = std::stoi(arg2);
                    }
                }
            }
            catch (const std::exception& exception)
            {
                std::cout << "~ Unrecognized arguments\n";
                std::cout << "~ Run \"help\" for a list of commands\n";
                showReady();
                continue;
            }
            if (arg1 == "split")
            {
                // run perft with split mode enabled
                timespec start = {};
                timespec end = {};
                PerftInfo info = {};
                std::cout << "~ Running depth " << maxDepth << " split enabled perft\n";
                clock_gettime(CLOCK_REALTIME, &start);
                perft(maxDepth, info, maxDepth);
                clock_gettime(CLOCK_REALTIME, &end);

                double startMillis = (start.tv_sec * 1000.0) + (start.tv_nsec / 1000000.0);
                double endMillis = (end.tv_sec * 1000.0) + (end.tv_nsec / 1000000.0);
                double msElapsed = endMillis - startMillis;

                printPerftInfo(info, maxDepth, msElapsed);
            }
            else
            {
                // run perft in normal mode
                for (int depth = minDepth; depth <= maxDepth; depth++)
                {
                    timespec start = {};
                    timespec end = {};
                    PerftInfo info = {};
                    std::cout << "~ Running depth " << depth << " perft\n";

                    clock_gettime(CLOCK_REALTIME, &start);
                    perft(depth, info);
                    clock_gettime(CLOCK_REALTIME, &end);

                    double startMillis = (start.tv_sec * 1000.0) + (start.tv_nsec / 1000000.0);
                    double endMillis = (end.tv_sec * 1000.0) + (end.tv_nsec / 1000000.0);
                    double msElapsed = endMillis - startMillis;

                    printPerftInfo(info, depth, msElapsed);
                }
            }
            std::cout << "~ Perft test complete\n";
            showReady();
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
    std::cout << "id name Karl " << VERSION << "\n";
    std::cout << "id author Joe Chrisman\n";
    std::cout << "uciok\n";

    std::string command;
    while (std::getline(std::cin, command))
    {
        if (command == "isready")
        {
            std::cout << "readyok\n";
        }
        else if (command.substr(0, 8) == "position")
        {
            size_t movesIndex = command.find("moves");
            if (command.substr(9, 8) == "startpos")
            {
                Position::init(INITIAL_FEN);
            }
            else
            {
                Position::init(command.substr(9, movesIndex));
            }
            if (movesIndex != std::string::npos)
            {
                std::stringstream moves(command.substr(movesIndex + 5, std::string::npos));
                std::string moveStr;
                while (moves >> moveStr)
                {
                    Gen::genMoves();
                    for (const Move move : Gen::moveList)
                    {
                        if (move == Moves::NULL_MOVE)
                        {
                            break;
                        }
                        if (moveStr == Notation::moveToStr(move))
                        {
                            Position::makeMove(move);
                            break;
                        }
                    }
                }
            }
            Position::print(true);
        }
        else if (command == "go")
        {
            std::cout << "bestmove " <<Notation::moveToStr(Search::getBestMove()) << "\n";
        }

    }
    return 0;
}
