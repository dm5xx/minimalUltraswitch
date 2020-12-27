#pragma once
#include <ESPAsyncWebServer.h>
#include "webhelper.h"

class CustomStatusHandler  : public AsyncWebHandler {
    std::shared_ptr<WebHelper> _Helper;
public:
    CustomStatusHandler(std::shared_ptr<WebHelper> hlp) : _Helper(hlp)
    {
    }

    bool canHandle(AsyncWebServerRequest *request){
        //request->addInterestingHeader("ANY");
        String sss = request -> url();

#ifdef LDEBUG
        Serial.println("UnlockHandler");        
#endif
        return sss == "/ESPStatus";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
#ifdef LDEBUG
        Serial.println("ESPStatus UNlocked!!");
#endif
        _Helper->SendESPStatus(request);
    }
};