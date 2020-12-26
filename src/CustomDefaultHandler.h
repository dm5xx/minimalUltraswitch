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
        //request->addInterestingHeader("ANY");
        String sss = request -> url();

        Serial.println("DefaultHandler");
        // Serial.println(sss);

        // Serial.println("Eins");
        // _Helper->getJs();
        // Serial.println("Zwei");
        // _Helper->getCu();
        // Serial.println("Done");
        
        return sss == "/";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
        Serial.println("Handle for Main!!");
        // Serial.println("Eins");
        // _Helper->getJs();
        // Serial.println("Zwei");
        // _Helper->getCu();
        // Serial.println("Done");
        _Helper->MainPage(request);       
    }
};