#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <bitset>
#include <iostream>

constexpr unsigned WIDTH  = 7;
constexpr unsigned HEIGHT = 6;

constexpr char EMPTY = ' ';

struct Board : std::array<std::array<char, WIDTH>, HEIGHT>
{
    Board()
    {
        for (unsigned row = 0; row < HEIGHT; ++row)
        {
            for (unsigned col=0; col < WIDTH; ++col)
            {
                (*this)[row][col] = EMPTY;
            }
        }
    }

    bool isColValid(unsigned col) const
    {
        return (*this)[HEIGHT - 1][col] == EMPTY;
    }

    unsigned getTopRow(unsigned col) const
    {
        for (unsigned row=0; row < WIDTH; ++row)
        {
            if ((*this)[row][col] == EMPTY)
            {
                return row;
            }
        }
        return HEIGHT - 1;
    }

    bool addPosition(unsigned col, char player)
    {
        if (isColValid(col))
        {
            (*this)[getTopRow(col)][col] = player;
            return true;
        }
        return false;
    }

    bool isDone(char player) const
    {
        // check col
        for (unsigned row=0; row < 3; ++row)
        {
            for (unsigned col=0; col < WIDTH; ++col)
            {
                if ((*this)[row  ][col] == player &&
                    (*this)[row+1][col] == player &&
                    (*this)[row+2][col] == player &&
                    (*this)[row+3][col] == player)
                {
                    return true;
                }
            }
        }

        // check row
        for (unsigned row=0; row < HEIGHT; ++row)
        {
            for (unsigned col=0; col < 4; ++col)
            {
                if ((*this)[row][col  ] == player &&
                    (*this)[row][col+1] == player &&
                    (*this)[row][col+2] == player &&
                    (*this)[row][col+3] == player)
                {
                    return true;
                }
            }
        }

        // check diag
        for (unsigned row=0; row < 3; ++row)
        {
            for (unsigned col=0; col < 4; ++col)
            {
                if ((*this)[row  ][col  ] == player &&
                    (*this)[row+1][col+1] == player &&
                    (*this)[row+2][col+2] == player &&
                    (*this)[row+3][col+3] == player)
                {
                    return true;
                }
            }
        }
        for (unsigned row=0; row < 3; ++row)
        {
            for (unsigned col=3; col < WIDTH; ++col)
            {
                if ((*this)[row  ][col  ] == player &&
                    (*this)[row+1][col-1] == player &&
                    (*this)[row+2][col-2] == player &&
                    (*this)[row+3][col-3] == player)
                {
                    return true;
                }
            }
        }

        return false;
    }

    bool isFull() const
    {
        for (unsigned row=0; row < HEIGHT; ++row)
        {
            for (unsigned col=0; col < WIDTH; ++col)
            {
                if ((*this)[row][col] == EMPTY)
                {
                    return false;
                }
            }
        }

        return true;
    }


    friend std::ostream& operator<<(std::ostream& os, const Board& board)
    {
        os << "|-|-|-|-|-|-|-|\n";
        for (int row = HEIGHT - 1; row >= 0; --row)
        {
            for (unsigned col=0; col < WIDTH; ++col)
            {
                os << "|" << board[row][col];
            }
            os << "|\n";
        }
        os << "|-|-|-|-|-|-|-|\n";
        return os;
    }
};

#endif
