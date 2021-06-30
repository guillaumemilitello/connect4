#ifndef COMPUTER_HPP
#define COMPUTER_HPP

#include "board.h"
#include "player.h"

#include <limits>
#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>

struct Computer
{
    static unsigned getCol(const State& state, const unsigned recursionLevel);

private:
    struct Scores : std::array<int, WIDTH>
    {
        static constexpr value_type WIN_MOVE         { 1000000};
        static constexpr value_type FORCED_MOVE      {-10000};
        static constexpr value_type DOUBLE_TRAP_MOVE { 1000};
        static constexpr value_type TRAP_MOVE        { 100};

        Scores();
        Scores(value_type i);

        int max() const;
        unsigned getBestCol() const;

        friend std::ostream& operator<<(std::ostream& os, const Scores& scores);
    };
    friend std::ostream& operator<<(std::ostream& os, Computer::Scores const& scores);

    struct Log
    {
        static constexpr bool LOG_ENABLED {false};

        explicit Log(const std::string& fileName);

        template<typename T>
        Log& operator<<(const T& t);

    private:
        static unsigned _moveNumber;
        std::ofstream _file;
    };

    static Scores getScores(const State& state, const char player, const unsigned recursionLevel, Log& log, const bool multiThreading=false);

    static void getScoreColRec(const State& state, const unsigned col, const char player, const unsigned recursionLevel, Log& log, Scores& scores);
    static Scores::value_type getScoreCol(const State& state, unsigned const col, Log& log);

    static Board getEvaluationBoard(const Board& board, const char player, Log& log);
    static void horizontalEvaluation(const Board& board, const char player, Board& evaluationBoard);
    static void verticalEvaluation(const Board& board, const char player, Board& evaluationBoard);
    static void diagonalEvaluation(const Board& board, const char player, Board& evaluationBoard);
    static void doubleForceMoveEvaluation(const Board& board, const char player, Board& evaluationBoard);
};

Computer::Scores::Scores()
{
    for(size_t col = 0; col < WIDTH; ++col)
    {
        (*this)[col] = std::numeric_limits<value_type>::lowest();
    }
}

Computer::Scores::Scores(value_type i)
{
    for(size_t col = 0; col < WIDTH; ++col)
    {
        (*this)[col] = 0.0;
    }
}

int Computer::Scores::max() const
{
    value_type max {std::numeric_limits<value_type>::lowest()};
    for(size_t col = 0; col < WIDTH; ++col)
    {
        max = std::max(max, (*this)[col]);
    }
    return max;
}

unsigned Computer::Scores::getBestCol() const
{
    const value_type maxValue = max();
    std::vector<unsigned> colSet;
    for(size_t col = 0; col < WIDTH; ++col)
    {
        if ((*this)[col] == maxValue)
        {
            if (col == 3)
            {
                return 3;
            }
            colSet.emplace_back(col);
        }
    }

    if (colSet.size() == 1)
    {
        return colSet[0];
    }

    std::shuffle(std::begin(colSet), std::end(colSet), std::default_random_engine{});
    return colSet[0];
}

std::ostream& operator<<(std::ostream& os, const Computer::Scores& scores)
{
    for (unsigned col=0; col < WIDTH; ++col)
    {
        os << "|" << scores[col];
    }
    os << "|\n";
    return os;
}

Computer::Log::Log(const std::string& fileName)
{
    if (LOG_ENABLED)
    {
        _file = std::ofstream(fileName + "_" + std::to_string(_moveNumber++), std::ofstream::out);
    }
}

template<typename T>
Computer::Log& Computer::Log::operator<<(const T& t)
{
    if (LOG_ENABLED)
    {
        _file << t;
    }
    return *this;
}

unsigned Computer::Log::_moveNumber = 0;

unsigned Computer::getCol(const State& state, const unsigned recursionLevel)
{
    Log log("computeur");
    std::cout << "COMPUTER... ";

    const auto timeBegin {std::chrono::high_resolution_clock::now()};

    const auto col {getScores(state, state.getTurn(), recursionLevel, log, true).getBestCol()};

    const std::chrono::duration<double, std::milli> duration {std::chrono::high_resolution_clock::now() - timeBegin};
    std::cout << duration.count() << "ms\n";

    return col;
}

Computer::Scores Computer::getScores(const State& state, const char player, const unsigned recursionLevel, Log& log, const bool multiThreading)
{
    if (recursionLevel == 0)
    {
        Scores s(0.0);
        log << "RECURSION LEVEL=" << recursionLevel << " SCORES=" << s << "\n";
        return s;
    }

    log << "GET SCORES ==========================================\n";

    Scores scores;
    std::array<std::thread, WIDTH> threadArray;
    std::array<std::chrono::system_clock::time_point, WIDTH> timeBeginArray; // for chrono, only if(multiThreading)
    std::atomic<int> threadNumber = 0;

    log << "PLAYER PLAYING ========================\n";
    for (unsigned col=0; col < WIDTH; ++col)
    {
        if (multiThreading)
        {
            static const int threadNumberMax {static_cast<int>(std::thread::hardware_concurrency()) - 1};
            while (threadNumber >= threadNumberMax)
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(50ms);
            }

            timeBeginArray[col] = std::chrono::high_resolution_clock::now();
            auto currentThread {std::thread(getScoreColRec, std::cref(state), col, player, recursionLevel, std::ref(log), std::ref(scores))};
            threadArray[col] = std::move(currentThread);
            ++threadNumber;
        }
        else
        {
            getScoreColRec(state, col, player, recursionLevel, log, scores);
        }
    }

    if (multiThreading)
    {
        for (unsigned col=0; col < WIDTH; ++col)
        {
            if (threadArray[col].joinable())
            {
                threadArray[col].join();
                --threadNumber;
            }
        }
    }

    log << "RECURSION LEVEL=" << recursionLevel << " SCORES=" << scores << "\n";

    return scores;
}


void Computer::getScoreColRec(const State& state, const unsigned col, const char player, const unsigned recursionLevel, Log& log, Scores& scores)
{
    if (!state.isColValid(col))
    {
        return;
    }

    State nextState {state};
    nextState.addPosition(col);

    const auto scoreCol {getScoreCol(nextState, col, log)};
    scores[col] = scoreCol;

    if (scoreCol == Scores::WIN_MOVE ||
        scoreCol == Scores::DOUBLE_TRAP_MOVE ||
        scoreCol == Scores::FORCED_MOVE)
    {
        log << "RECURSION LEVEL=" << recursionLevel << " COL=" << col << " SCORE=" << scores[col] << "\n";
        return;
    }

    Scores recScores {getScores(nextState, player, recursionLevel - 1, log)};
    log << "RECURSION LEVEL=" << recursionLevel << " COL=" << col << " MAX=" << recScores.max() << "\n";

    Scores::value_type bestRecScore {recScores.max()};
    if (player == nextState.getLastPayer())
    {
        bestRecScore = -bestRecScore;
    }

    const Scores::value_type bestRecScoreWithFactor {static_cast<Scores::value_type>(static_cast<double>(bestRecScore) / 1.5)}; // recursion factor
    log << "RECURSION LEVEL=" << recursionLevel << " COL=" << col << " SCORE=" << bestRecScoreWithFactor << "\n";
    scores[col] = bestRecScoreWithFactor;
}

Computer::Scores::value_type Computer::getScoreCol(const State& state, unsigned const col, Log& log)
{
    if (state.isDone()) // check if winning move
    {
        log << "COL=" << col << " WIN_MOVE\n";
        return Scores::WIN_MOVE;
    }

    // EVALUATION
    log << "COL=" << col << " EVALUATION - opponent\n";
    Scores::value_type score {0};
    const Board& board {state.getBoard()};
    const char player {getOpponent(state.getTurn())}; // get last played
    const Board opponentEvaluationBoard {getEvaluationBoard(board, getOpponent(player), log)};
    for (unsigned evalCol=0; evalCol < WIDTH; ++evalCol)
    {
        if (board.isColValid(evalCol))
        {
            // future win for opponent: don't play
            const char opponentEvalCol {opponentEvaluationBoard[board.getTopRow(evalCol)][evalCol]};
            if (opponentEvalCol == 'F' || opponentEvalCol == 'D')
            {
                log << "COL=" << col << " EVALUATION - opponent EVALCOL=" << evalCol << " FORCED_MOVE\n";
                return Scores::FORCED_MOVE;
            }
        }
    }

    log << "COL=" << col << " EVALUATION - player\n";
    const Board evaluationBoard {getEvaluationBoard(board, player, log)};

    unsigned forceMoveCount = 0;
    for (unsigned evalCol=0; evalCol < WIDTH; ++evalCol)
    {
        if (board.isColValid(evalCol))
        {
            const char playerEvalCol {evaluationBoard[board.getTopRow(evalCol)][evalCol]};
            if (playerEvalCol == 'D')
            {
                score = std::max(score, Scores::DOUBLE_TRAP_MOVE);
                log << "COL=" << col << " EVALUATION - opponent EVALCOL=" << evalCol << " DOUBLE_TRAP_MOVE\n";
            }
            else if (playerEvalCol == 'F')
            {
                score = std::max(score, Scores::TRAP_MOVE);
                ++forceMoveCount;
                log << "COL=" << col << " EVALUATION - opponent EVALCOL=" << evalCol << " TRAP_MOVE\n";
            }
        }
    }

    // seven // double lines // 3 in a row
    // --> 2 forced moves in same col
    // --> 2 forced moves in the same board
    if (forceMoveCount > 1)
    {
        score = Scores::DOUBLE_TRAP_MOVE;
        log << "COL=" << col << " EVALUATION - opponent DOUBLE_TRAP_MOVE forceMoveCount > 1\n";
    }

    return score;
}

Board Computer::getEvaluationBoard(const Board& board, const char player, Log& log)
{
    Board evaluationBoard {board};

    horizontalEvaluation(board, player, evaluationBoard);
    verticalEvaluation(board, player, evaluationBoard);
    diagonalEvaluation(board, player, evaluationBoard);
    doubleForceMoveEvaluation(board, player, evaluationBoard);

    log << evaluationBoard;

    return evaluationBoard;
}

void Computer::horizontalEvaluation(const Board& board, const char player, Board& evaluationBoard)
{
    for (unsigned col=0; col < WIDTH - 3; ++col)
    {
        for (int row = HEIGHT - 1; row >= 0; --row)
        {
            if (board[row][col] == player)
            {
                // XXX- XX-X
                if (board[row][col + 1] == player)
                {
                    // XXX-
                    if (board[row][col + 2] == player &&
                        board[row][col + 3] == EMPTY)
                    {
                        evaluationBoard[row][col + 3] = 'F';
                    }
                    // XX-X
                    else if (board[row][col + 2] == EMPTY &&
                                board[row][col + 3] == player)
                    {
                        evaluationBoard[row][col + 2] = 'F';
                    }
                }
                // X-XX
                else if (board[row][col + 1] == EMPTY  &&
                            board[row][col + 2] == player &&
                            board[row][col + 3] == player)
                {
                    evaluationBoard[row][col + 1] = 'F';
                }
            }
            // -XXX
            else if(board[row][col    ] == EMPTY &&
                    board[row][col + 1] == player &&
                    board[row][col + 2] == player &&
                    board[row][col + 3] == player)
            {
                evaluationBoard[row][col] = 'F';
            }
        }
    }
}

void Computer::verticalEvaluation(const Board& board, const char player, Board& evaluationBoard)
{
    for (unsigned col=0; col < WIDTH; ++col)
    {
        for (unsigned row = HEIGHT - 1; row >= 3; --row)
        {
            // -
            // X
            // X
            // X
            if (board[row    ][col] == EMPTY  &&
                board[row - 1][col] == player &&
                board[row - 2][col] == player &&
                board[row - 3][col] == player)
            {
                evaluationBoard[row][col] = 'F';
            }
        }
    }
}

void Computer::diagonalEvaluation(const Board& board, const char player, Board& evaluationBoard)
{
    for (unsigned col=WIDTH-1; col >= 3; --col)
    {
        for (unsigned row=HEIGHT-1; row >= 3; --row)
        {
            //    X    X    X
            //   X?   X?   -?
            //  X??  -??  X??
            // -??? X??? X???
            if (board[row][col] == player)
            {
                //    X    X
                //   X?   X?
                //  X??  -??
                // -??? X???
                if (board[row - 1][col - 1] == player)
                {
                    //    X
                    //   X?
                    //  X??
                    // -???
                    if (board[row - 2][col - 2] == player &&
                        board[row - 3][col - 3] == EMPTY)
                    {
                        evaluationBoard[row - 3][col - 3] = 'F';
                    }
                    //    X
                    //   X?
                    //  -??
                    // X???
                    else if (board[row - 2][col - 2] == EMPTY &&
                                board[row - 3][col - 3] == player)
                    {
                        evaluationBoard[row - 2][col - 2] = 'F';
                    }
                }
                //    X
                //   -?
                //  X??
                // X???
                else if (board[row - 1][col - 1] == EMPTY &&
                            board[row - 2][col - 2] == player &&
                            board[row - 3][col - 3] == player)
                {
                    evaluationBoard[row - 1][col - 1] = 'F';
                }
            }
            //    -
            //   X?
            //  X??
            // X???
            else if (board[row    ][col    ] == EMPTY &&
                        board[row - 1][col - 1] == player &&
                        board[row - 2][col - 2] == player &&
                        board[row - 3][col - 3] == player)
            {
                evaluationBoard[row][col] = 'F';
            }
        }
    }

    for (unsigned col=0; col < WIDTH - 3; ++col)
    {
        for (unsigned row=HEIGHT-1; row >= 3; --row)
        {
            // X    X    X
            // ?X   ?X   ?-
            // ??X  ??-  ??X
            // ???- ???X ???X
            if (board[row][col] == player)
            {
                // X    X
                // ?X   ?X
                // ??X  ??-
                // ???- ???X
                if (board[row - 1][col + 1] == player)
                {
                    // X
                    // ?X
                    // ??X
                    // ???-
                    if (board[row - 2][col + 2] == player &&
                        board[row - 3][col + 3] == EMPTY)
                    {
                        evaluationBoard[row - 3][col + 3] = 'F';
                    }
                    // X
                    // ?X
                    // ??-
                    // ???X
                    else if (board[row - 2][col + 2] == EMPTY &&
                                board[row - 3][col + 3] == player)
                    {
                        evaluationBoard[row - 2][col + 2] = 'F';
                    }
                }
                // X
                // ?-
                // ??X
                // ???X
                else if (board[row - 1][col + 1] == EMPTY &&
                        board[row - 2][col + 2] == player &&
                        board[row - 3][col + 3] == player)
                {
                    evaluationBoard[row - 1][col + 1] = 'F';
                }
            }
            // -
            // ?X
            // ??X
            // ???X
            else if (board[row    ][col    ] == EMPTY &&
                        board[row - 1][col + 1] == player &&
                        board[row - 2][col + 2] == player &&
                        board[row - 3][col + 3] == player)
            {
                evaluationBoard[row][col] = 'F';
            }
        }
    }
}

void Computer::doubleForceMoveEvaluation(const Board& board, const char player, Board& evaluationBoard)
{
    for (unsigned col=0; col < WIDTH; ++col)
    {
        for (unsigned row = HEIGHT - 1; row >= 1; --row)
        {
            if (evaluationBoard[row    ][col] == 'F' &&
                evaluationBoard[row - 1][col] == 'F')
            {
                evaluationBoard[row - 1][col] = 'D';
            }
        }
    }
}

#endif // COMPUTER_HPP
