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
        Serial.println("ResetHandler");
        
        return sss == "/Reset";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
        Serial.println("Handle reset!!");
        _Helper->SendReset(request);
        _shouldReboot = true;
    }
};