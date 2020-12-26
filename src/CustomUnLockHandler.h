#pragma once
#include <ESPAsyncWebServer.h>
#include "webhelper.h"

class CustomUnlockHandler  : public AsyncWebHandler {
    std::shared_ptr<WebHelper> _Helper;
public:
    CustomUnlockHandler(std::shared_ptr<WebHelper> hlp) : _Helper(hlp)
    {
    }

    bool canHandle(AsyncWebServerRequest *request){
        //request->addInterestingHeader("ANY");
        String sss = request -> url();

        Serial.println("UnlockHandler");        
        return sss == "/UnLock";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
        Serial.println("Handle UNlocked!!");
        _Helper->UnLock();
        _Helper->SendLocked(request);
    }
};