#pragma once
#include "Adafruit_MCP23017.h"
#include <Arduino.h>
#include "RelayBoard.h"

class Boards {
    private:
        RelayBoard * _boards;

    public:
       Boards(byte numberOfBoards);
        ~Boards();
       RelayBoard * GetBoardByNumber(byte boardNumber);
       RelayBoard * GetBoards();
};
