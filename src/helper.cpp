#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "helper.h"

//    WebHelper(byte numberBoards, RelayBoards &rb, bool& IsLocked);

Helper::Helper(byte numberBoards, Boards &rb) : _numberOfRelayBoards(numberBoards), 
    _relayboard(rb)
{
}
    void Helper::SetStatusOfBank(byte bankNr, unsigned int value)
    {
        byte tempArray[16]; 
        GetOrderedArraybyValue(value, tempArray);
        
        _relayboard.GetBoardByNumber(bankNr)->SetStatus((uint) value);
    }

    String Helper::getValue(String data, char separator, int index)
    {
        int found = 0;
        int strIndex[] = { 0, -1 };
        int maxIndex = data.length() - 1;
    
        for (int i = 0; i <= maxIndex && found <= index; i++) {
            if (data.charAt(i) == separator || i == maxIndex) {
                found++;
                strIndex[0] = strIndex[1] + 1;
                strIndex[1] = (i == maxIndex) ? i+1 : i;
            }
        }
        return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
    }

    void Helper::GetOrderedArraybyValue(unsigned int value, byte * feld)
    {
        int i;

        for (i = 0; i < 16; i++)
        {
            feld[i] = value % 2;
            value /= 2;
        }
    }

    unsigned int Helper::GetValueByOrderedArray(byte * arr)
    {
    unsigned int result = 0;
        
    for(uint8_t a = 0; a < 16; a++)
    {
            result = result + (arr[a] * 1<<a);
    }

    return result;
    }

    std::vector<std::string> Helper::getResult(char * s, char delimiter)
    {
        std::vector<std::string> splits;                                                                                                                                                           
        std::string split;                                                                                                                                                                         
        std::istringstream ss(s);                                                                                                                                                                  
        while (std::getline(ss, split, delimiter))                                                                                                                                                 
        {                                                                                                                                                                                          
            splits.push_back(split);                                                                                                                                                                
        }                                                                                                                                                                                          
        return splits;         
    }