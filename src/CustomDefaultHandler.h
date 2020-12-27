#pragma once
#include <ESPAsyncWebServer.h>
#include "webhelper.h"
#include <memory>

class CustomDefaultHandler  : public AsyncWebHandler {
    std::shared_ptr<WebHelper> _Helper;
public:
    CustomDefaultHandler(std::shared_ptr<WebHelper> hlp)  : _Helper(hlp)
    {
    }
    bool canHandle(AsyncWebServerRequest *request){
        String sss = request -> url();

#ifdef LDEBUG
        Serial.println("DefaultHandler");
        Serial.println(sss);
#endif
        
        return sss == "/";
    }

    void handleRequest(AsyncWebServerRequest *request) {   

#ifdef LDEBUG
        Serial.println("Handle for Main!!");
#endif
        _Helper->MainPage(request);       
    }
};