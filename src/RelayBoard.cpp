#include "Adafruit_MCP23017.h"
#include <Arduino.h>
#include "RelayBoard.h"

//#define LDEBUG

RelayBoard::RelayBoard() : _status(0), _pinStatus{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, _RelayPins{ 0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15 }
{
}


void RelayBoard::Init(byte boardnumber)
{
#ifdef LDEBUG
    Serial.print("Init boardnumber: ");
    Serial.println(boardnumber);  // just for test  
#endif

    _boardNumber = boardnumber;    
    _mcp.begin(_boardNumber);
     
    for (byte p = 0; p < 16; p++)
    {
        _pinStatus[p] = 0;
        _mcp.pinMode(p, OUTPUT);
        _mcp.digitalWrite(p, HIGH);
        delay(5);
    }
}

unsigned int RelayBoard::GetStatus() {

#ifdef LDEBUG
    Serial.print("Status: ");
    Serial.print(_status);  // just for test  
    Serial.print("OnBoard: ");
    Serial.println(_boardNumber);  // just for test  
#endif

    return _status;
}

void RelayBoard::SetStatus(unsigned int value)
{

#ifdef LDEBUG
    Serial.print("OnBoard: ");
    Serial.println(_boardNumber);  // just for test  
    Serial.print("Status befor: ");
    Serial.println(_status);
#endif

    _status = value;

#ifdef LDEBUG
    Serial.print("Status after: ");
    Serial.println(_status);
#endif

    GetOrderedArraybyValue(_status, _pinStatus);

    for(byte a = 0; a < 16; a++)
    {
#ifdef LDEBUG
        Serial.print("PS: ");
        Serial.print(_pinStatus[a]);
        Serial.print(" of pin number ");
        Serial.println(_RelayPins[a]);
#endif
         _mcp.digitalWrite(_RelayPins[a], !_pinStatus[a]);
#ifdef LDEBUG
        Serial.print(" Mcp write done");
#endif
    }
}

void RelayBoard::GetOrderedArraybyValue(unsigned int value, byte * feld)
{
    int i;

    for (i = 0; i < 16; i++)
    {
        feld[i] = value % 2;
        value /= 2;
    }
}

unsigned int RelayBoard::GetValueByOrderedArray(byte * arr)
{
    unsigned int result = 0;
        
    for(uint8_t a = 0; a < 16; a++)
    {
            result = result + (arr[a] * 1<<a);
    }

    return result;
}