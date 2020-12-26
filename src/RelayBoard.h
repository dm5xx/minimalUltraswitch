#pragma once
#include "Adafruit_MCP23017.h"
#include <Arduino.h>

class RelayBoard {
    private:
        byte _RelayPins[16];
        byte _pinStatus[16];
        Adafruit_MCP23017 _mcp;
        unsigned int _status;
        byte _boardNumber;

        void GetOrderedArraybyValue(unsigned int value, byte * feld);
        unsigned int GetValueByOrderedArray(byte * arr);

        //RelayBoard(const RelayBoard&){}; // prevent copy constructor to be generated the easy way...
    public:
        RelayBoard();
        RelayBoard(const RelayBoard&) = delete; // prevent copy constructor to be generated - prevent object for being copied
        RelayBoard& operator=(const RelayBoard&) = delete; // prevent assignment constructor to be generated - prevent object for being assigned
        void Init(byte boardnumber);
        unsigned int GetStatus();        
        void SetStatus(unsigned int value);
};



