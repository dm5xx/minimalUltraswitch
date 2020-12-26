#pragma once
#include "RelayBoard.h"
#include "Boards.h"

class AsyncWebServerRequest; // nur wegen header vorwärtsdeklaration
class Helper {
    byte _numberOfRelayBoards;
    Boards _relayboard;
public:
    Helper(byte numberBoards, Boards &rb);

    void SetStatusOfBank(byte bankNr, unsigned int value);
    
    String getValue(String data, char separator, int index);

    void GetOrderedArraybyValue(unsigned int value, byte * feld);

    unsigned int GetValueByOrderedArray(byte * arr);

    std::vector<std::string> getResult(char * s, char delimiter);
};