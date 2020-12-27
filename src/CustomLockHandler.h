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

#ifdef LDEBUG
        Serial.println("LockHandler");        
#endif
        return sss == "/Lock";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
#ifdef LDEBUG
        Serial.println("Handle lock!!");
#endif
        _Helper->Lock();
        _Helper->SendLocked(request);
    }
};