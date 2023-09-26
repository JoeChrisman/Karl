//
// Created by Joe Chrisman on 4/7/23.
//

#include <ctime>
#include <sstream>
#include <fstream>
#include "Cli.h"

Cli::Cli(const Zobrist& zobrist, const Magics& magics)
: position(zobrist), moveGen(position, magics), evaluator(position, moveGen), search(position, moveGen, evaluator)
{
    isWhiteOnBottom = true;
}

int Cli::runCli()
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
            std::ifstream readme("../manual.txt");
            std::string line = "";
            while (std::getline(readme, line))
            {
                std::cout << line << "\n";
            }

            showReady();
        }
        else if (command.substr(0, 4) == "load")
        {
            std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";;

            if (command != "load")
            {
                fen = command.substr(5, std::string::npos);
            }
            if (position.loadFen(fen))
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
            position.print(isWhiteOnBottom);
            showReady();
        }
        else if (command == "info")
        {
            const std::string sideToMove = position.isWhiteToMove ? "white" : "black";
            const int castlingFlags = position.irreversibles.castlingFlags;
            const int enPassant = position.irreversibles.enPassantFile;
            const std::string enPassantFile = enPassant == -1 ? "NULL" : fileToStr(enPassant);

            std::cout << "~ Position info:\n";
            std::cout << "\t~ =====================================\n";
            std::cout << "\t~ Zobrist hash     | 0x" << std::hex << position.hash << std::dec << "\n";
            std::cout << "\t~ Side to move     | " << sideToMove << "\n";
            std::cout << "\t~ En passant file  | " << enPassantFile << "\n";
            std::cout << "\t~ Castling flags   | 0x" << std::hex << castlingFlags << std::dec << "\n";
            std::cout << "\t~ Total plies      | " << position.totalPlies << "\n";
            std::cout << "\t~ Reversible plies | " << position.irreversibles.reversiblePlies << "\n";
            std::cout << "\t~ =====================================\n";
            showReady();
        }
        else if (command == "evaluate")
        {
            std::cout << evaluator.evaluate() << "\n";
            showReady();
        }
        else if (command.substr(0, 6) == "search")
        {
            if (command.substr(7, 4) == "time")
            {
                int time;
                try
                {
                    time = std::stoi(command.substr(12, std::string::npos));
                }
                catch (const std::exception& exception)
                {
                    std::cout << "~ Unrecognized arguments\n";
                    std::cout << "~ Run \"help\" for a list of commands\n";
                    showReady();
                    continue;
                }
                Move best = search.searchByTime(time);
                std::cout << "~ Best move: " << moveToStr(best) << "\n";
            }
            else if (command.substr(7, 5) == "depth")
            {
                int depth;
                try
                {
                    depth = std::stoi(command.substr(13, std::string::npos));
                }
                catch (const std::exception& exception)
                {
                    std::cout << "~ Unrecognized arguments\n";
                    std::cout << "~ Run \"help\" for a list of commands\n";
                    showReady();
                    continue;
                }
                ScoredMove best = search.searchByDepth(depth);
                std::cout << "~ Best move: " << moveToStr(best.move) << "\n";
            }
            showReady();
        }
        else if (command == "who")
        {
            if (position.isWhiteToMove)
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
            moveGen.genMoves();
            Move legalMove = NULL_MOVE;
            for (int i = 0; i < moveGen.numMoves; i++)
            {
                Move move = moveGen.moveList[i];
                if (moveToStr(move) == notation)
                {
                    legalMove = move;
                    break;
                }
            }
            if (legalMove != NULL_MOVE)
            {
                position.makeMove(legalMove);
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
            moveGen.genMoves();
            std::cout << "~ There are " << moveGen.numMoves << " moves\n";
            for (int i = 0; i < moveGen.numMoves; i++)
            {
                std::cout << "\t~ " << moveToStr(moveGen.moveList[i]) << "\n";
            }
            showReady();
        }
        else if (command == "captures")
        {
            moveGen.genCaptures();
            std::cout << "~ There are " << moveGen.numMoves << " captures\n";
            for (int i = 0; i < moveGen.numMoves; i++)
            {
                std::cout << "\t~ " << moveToStr(moveGen.moveList[i]) << "\n";
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
            position.isWhiteToMove = !position.isWhiteToMove;
            std::cout << "~ Successfully gave " << (position.isWhiteToMove ? "white" : "black") << " the move\n";
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
            if (runUci())
            {
                return 0;
            }
            showReady();
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

int Cli::runUci()
{
    std::cout << std::flush;
    std::cout << "id name Karl\n";
    std::cout << "id author Joe Chrisman\n";
    std::cout << "uciok\n";

    std::string command;
    while (std::getline(std::cin, command))
    {
        // return to cli mode
        if (command == "exit")
        {
            return 0;
        }
        // kill this entire process
        if (command == "quit")
        {
            return 1;
        }
        else if (command == "isready")
        {
            std::cout << "readyok\n";
        }
        else if (command.substr(0, 8) == "position")
        {
            size_t movesIndex = command.find("moves");
            if (command.substr(9, 8) == "startpos")
            {
                position.loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            }
            else
            {
                if (!position.loadFen(command.substr(12, (movesIndex == std::string::npos) ? movesIndex : movesIndex - 12)))
                {
                    // our UCI client gave us an invalid FEN string
                    // just ignore it and hope everything will be okay (it won't)
                    continue;
                }
            }
            if (movesIndex != std::string::npos)
            {
                std::stringstream moves(command.substr(movesIndex + 5, std::string::npos));
                std::string moveStr;
                while (moves >> moveStr)
                {
                    moveGen.genMoves();
                    for (const Move move : moveGen.moveList)
                    {
                        if (moveStr == moveToStr(move))
                        {
                            position.makeMove(move);
                            break;
                        }
                    }
                }
            }
        }
        else if (command.substr(0, 2) == "go")
        {
            Move best;

            if (command.substr(0, 11) == "go movetime")
            {
                int time = std::stoi(command.substr(12, std::string::npos));
                best = search.searchByTime(time);
            }
            else
            {
                std::stringstream timeControls(command.substr(2, std::string::npos));
                int msRemaining = 0;
                int msIncrement = 0;
                std::string consume; // get rid of flags sent in the command
                timeControls >> consume >> msRemaining;
                timeControls >> consume >> consume;
                timeControls >> consume >> msIncrement;
                timeControls >> consume >> consume;

                best = search.searchByTimeControl(msRemaining, msIncrement);
            }

            std::cout << "bestmove " << moveToStr(best) << "\n";
        }
    }
    return 0;
}

void Cli::printPerftInfo(const PerftInfo& info, const int depth, const double msElapsed)
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

void Cli::perft(int depth, PerftInfo &info, int splitDepth)
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
    Position::Irreversibles state = position.irreversibles;
    moveGen.genMoves();
    Move moves[256];
    std::memcpy(moves, moveGen.moveList, sizeof(MoveGen::moveList));
    int numMoves = moveGen.numMoves;
    for (int i = 0; i < numMoves; i++)
    {
        Move move = moves[i];
        if (depth == 1)
        {
            if (getCaptured(move) != NULL_PIECE)
            {
                info.captures++;
            }
            if (move & EN_PASSANT)
            {
                info.enPassants++;
            }
            if (move & (LONG_CASTLE | SHORT_CASTLE))
            {
                info.castles++;
            }
            if (getPromoted(move) != NULL_PIECE)
            {
                info.promotions++;
            }
        }
        position.makeMove(move);
        if (splitDepth == depth)
        {
            info.totalSplit = 0;
            perft(depth - 1, info, splitDepth);
            std::cout << "~ " << moveToStr(move) << ": " << info.totalSplit << "\n";
        }
        else
        {
            perft(depth - 1, info, splitDepth);
        }

        position.unMakeMove(move, state);
    }
}

void Cli::runPerftSuite()
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
        if (!position.loadFen(testContents[0]))
        {
            std::cout << "~ Invalid FEN string found in file \"perftSuite.txt\"\n";
            return;
        }

        const Hash hashBefore = position.hash;
        const Score midgamePlacementScoreBefore = position.midgamePlacementScore;

        std::cout << "~ Running perft unit tests on position " << testContents[0] << "\n";
        // the rest of the strings are depths and node counts
        for (int depth = 1; depth < testContents.size(); depth++)
        {
            PerftInfo info = {};
            perft(depth, info);
            totalNodes += info.totalNodes;
            int nodes = std::stoi(testContents[depth]);
            if (position.hash != hashBefore)
            {
                std::cout << "~ [FAIL] Perft unit test at depth " << depth << " failed. Incorrect hash\n";
                std::cout << "\t~ Expected hash to be " << std::hex << "0x" << hashBefore;
                std::cout << ", but found " << "0x" << position.hash << std::dec << "\n";
                failures++;
            }
            else if (position.midgamePlacementScore != midgamePlacementScoreBefore)
            {
                std::cout << "~ [FAIL] Perft unit test at depth " << depth << " failed. Incorrect midgame placement score\n";
                std::cout << "\t~ Expected score to be " << midgamePlacementScoreBefore << ", but found " << position.midgamePlacementScore << "\n";
                failures++;
            }
            else if (info.nodes == nodes)
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

void Cli::showReady()
{
    std::cout << "> ";
}

