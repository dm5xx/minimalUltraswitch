#pragma once
#include <ESPAsyncWebServer.h>
#include "webhelper.h"

class CustomSetHandler : public AsyncWebHandler {
    std::shared_ptr<WebHelper> _WHelper;
    std::shared_ptr<Helper> _Helper;
public:
    CustomSetHandler(std::shared_ptr<WebHelper> whlp, std::shared_ptr<Helper> hlp) : _WHelper(whlp), _Helper(hlp)
    {
    }

    bool canHandle(AsyncWebServerRequest *request){

        if(_WHelper->GetLock())
            return false;
            
        String sss = request -> url();

        Serial.println("Sethandler");

        if(sss.startsWith("/Set"))
            return true;
        return false;
    }

    void handleRequest(AsyncWebServerRequest *request) {   
        Serial.println("Handle Set");

        String xx = request -> url();
        byte len = xx.length()+1;
        char buff[len];
        xx.toCharArray(buff, len);

        std::vector<std::string> yo = _Helper->getResult(buff, '/');

        byte cmdLen = yo[1].length();
        byte bankNr = yo[1].c_str()[cmdLen-1] - '0';
        long value  = atol(yo[2].c_str());
        
        Serial.println(bankNr);
        Serial.println(value);

        _Helper->SetStatusOfBank(bankNr, value);
        _WHelper->Send200JSONOK(request);  
    }
};