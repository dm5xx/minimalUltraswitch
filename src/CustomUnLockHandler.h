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

#ifdef LDEBUG
        Serial.println("UnlockHandler");        
#endif
        return sss == "/UnLock";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
#ifdef LDEBUG
        Serial.println("Handle UNlocked!!");
#endif
        _Helper->UnLock();
        _Helper->SendLocked(request);
    }
};