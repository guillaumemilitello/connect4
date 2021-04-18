#ifndef STATE_H
#define STATE_H

#include "player.h"
#include "board.h"

struct State
{
    explicit State(char pTurn) : _pTurn(pTurn), _pWin(P0), _done(false)
    {}

    char getTurn() const
    {
        return _pTurn;
    }

    char getLastPayer() const
    {
        return getOpponent(_pTurn);
    }

    const char getWinner() const
    {
        return _pWin;
    }

    bool isColValid(unsigned col) const
    {
        return _board.isColValid(col);
    }

    void addPosition(unsigned col)
    {
        _board[_board.getTopRow(col)][col] = _pTurn;
        if (!_board.isDone(_pTurn))
        {
            _pTurn = getOpponent(_pTurn);
            if (_board.isFull())
            {
                _done = true;
            }
        }
        else
        {
            _done = true;
            _pWin = _pTurn;
        }
    }

    bool isDone() const
    {
        return _done;
    }

    friend std::ostream& operator<<(std::ostream& os, const State& state)
    {
        os << "|-|-|-|-|-|-|-|\n";
        for (int row=5; row >= 0; --row)
        {
            for (int col=0; col < 7; ++col)
            {
                os << "|" << state._board[row][col];
            }
            os << "|\n";
        }
        os << "|-|-|-|-|-|-|-|\n";
        os << "TURN: " << playerToString(state._pTurn) << '\n';
        return os;
    }

    const Board& getBoard() const
    {
        return _board;
    }

private:
    Board  _board;
    char   _pTurn;
    char   _pWin;
    bool   _done;
};

#endif
