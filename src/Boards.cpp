#include <Arduino.h>
#include "Boards.h"

Boards::Boards()
{
}

Boards::~Boards()
{
    delete[] (_boards);
}

void Boards::Init(byte numberOfBanks)
{
    _boards = new RelayBoard[numberOfBanks];

    for(byte a = 0; a < numberOfBanks; ++a)
    {
        _boards[a].Init(a);
    }
}

RelayBoard * Boards::GetBoardByNumber(byte boardNumber)
{
    return &_boards[boardNumber];
}

RelayBoard * Boards::GetBoards()
{
    return _boards;
}
