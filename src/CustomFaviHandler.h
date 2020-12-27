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

#ifdef LDEBUG
        Serial.println("FlavHandler");        
#endif
        return sss == "/favicon";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
#ifdef LDEBUG
        Serial.println("Handle flavicon!!");
#endif
        _Helper->SendFavicon(request);      
    }
};