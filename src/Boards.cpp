#include <Arduino.h>
#include "Boards.h"

Boards::Boards(byte numberOfBanks)
{
    _boards = new RelayBoard[numberOfBanks];

    for(byte a = 0; a < numberOfBanks; ++a)
    {
        _boards[a].Init(a);
    }
    //Serial.println(_boards[0].GetStatus());
}

Boards::~Boards()
{
    delete[] (_boards);
}

RelayBoard * Boards::GetBoardByNumber(byte boardNumber)
{
    return &_boards[boardNumber];
}

RelayBoard * Boards::GetBoards()
{
    return _boards;
}
