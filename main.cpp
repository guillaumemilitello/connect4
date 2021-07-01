#include <iostream>

#include "state.h"
#include "player.h"
#include "computer.hpp"

int main()
{
    char pTurn {' '};
    while (pTurn != P1 && pTurn != P2)
    {
        std::cout << "ENTER FIRST PLAYER [1|2]: ";
        std::cin >> pTurn;
    }

    State state(pTurn);

    unsigned recursionLevel = 4;
    std::cout << "COMPUTER RECURSION LEVEL: ";
    std::cin >> recursionLevel;

    while(!state.isDone())
    {
        std::cout << "===============\n";
        std::cout << state;
        unsigned col = 99;
        do
        {
            if (state.getTurn() == P1)
            {
                do
                {
                    std::cout << "ENTER COL: ";
                    std::cin >> col;
                }
                while(!state.isColValid(col));
            }
            else // if computer
            {
                col = Computer::getCol(state, recursionLevel);
            }
        }
        while (!state.isColValid(col));
        state.addPosition(col);
    }

    std::cout << "===============\n";
    std::cout << state;
    std::cout << "WINNER: " << playerToString(state.getWinner()) << "\n";

    return 0;
}