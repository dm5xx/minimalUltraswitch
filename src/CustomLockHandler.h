#pragma once
#include <ESPAsyncWebServer.h>
#include "webhelper.h"

class CustomLockHandler  : public AsyncWebHandler {
    std::shared_ptr<WebHelper> _Helper;
public:
    CustomLockHandler(std::shared_ptr<WebHelper> hlp) : _Helper(hlp)
    {
    }

    bool canHandle(AsyncWebServerRequest *request){
        //request->addInterestingHeader("ANY");
        String sss = request -> url();

        Serial.println("LockHandler");        
        return sss == "/Lock";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
        Serial.println("Handle lock!!");
        _Helper->Lock();
        _Helper->SendLocked(request);
    }
};