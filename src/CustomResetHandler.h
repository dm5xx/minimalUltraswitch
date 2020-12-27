#pragma once
#include <ESPAsyncWebServer.h>
#include "webhelper.h"

class CustomResetHandler  : public AsyncWebHandler {
    std::shared_ptr<WebHelper> _Helper;
    bool &_shouldReboot;
public:
    CustomResetHandler(std::shared_ptr<WebHelper> hlp, bool* shouldReboot)  : _Helper(hlp), _shouldReboot(*shouldReboot)
    {
    }

    bool canHandle(AsyncWebServerRequest *request){
        //request->addInterestingHeader("ANY");
        String sss = request -> url();
#ifdef LDEBUG
        Serial.println("ResetHandler");
#endif        
        return sss == "/Reset";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
#ifdef LDEBUG
        Serial.println("Handle reset!!");
#endif
        _Helper->SendReset(request);
        _shouldReboot = true;
    }
};