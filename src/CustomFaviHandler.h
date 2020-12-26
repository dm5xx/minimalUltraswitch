#pragma once
#include <ESPAsyncWebServer.h>
#include "webhelper.h"

class CustomFaviHandler  : public AsyncWebHandler {
    std::shared_ptr<WebHelper> _Helper;
public:
    CustomFaviHandler(std::shared_ptr<WebHelper> hlp) : _Helper(hlp)
    {
    }

    bool canHandle(AsyncWebServerRequest *request){
        //request->addInterestingHeader("ANY");
        String sss = request -> url();

        Serial.println("FlavHandler");        
        return sss == "/favicon";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
        Serial.println("Handle flavicon!!");
        _Helper->SendFavicon(request);      
    }
};