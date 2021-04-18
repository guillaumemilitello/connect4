#ifndef PLAYER_H
#define PLAYER_H

#include <string>

constexpr char P0 {'0'};
constexpr char P1 {'1'};
constexpr char P2 {'2'};

static inline char getOpponent(const char player)
{
    return player == P1 ? P2 : P1;
}

static inline std::string playerToString(const char player)
{
    if (player == P1 || player == P2)
    {
        return "P" + std::string{player};
    }
    return "NONE";
}

#endif