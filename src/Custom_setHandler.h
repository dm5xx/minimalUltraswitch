#pragma once
#include <ESPAsyncWebServer.h>
#include "webhelper.h"

class Custom_setHandler : public AsyncWebHandler {
    std::shared_ptr<WebHelper> _Helper;
public:
    Custom_setHandler(std::shared_ptr<WebHelper> hlp)  : _Helper(hlp)
    {
    }

    bool canHandle(AsyncWebServerRequest *request){

        String sss = request -> url();

        Serial.println("setHandler");
        Serial.println(sss);
        
        return sss == "/set/";
    }

    void handleRequest(AsyncWebServerRequest *request) {   

    }
};