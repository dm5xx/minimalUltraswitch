#pragma once
#include <ESPAsyncWebServer.h>
#include "webhelper.h"

class CustomGetHandler  : public AsyncWebHandler {
    std::shared_ptr<WebHelper> _Helper;
public:
    CustomGetHandler(std::shared_ptr<WebHelper> hlp)  : _Helper(hlp)
    {
    }
    bool canHandle(AsyncWebServerRequest *request){
        //request->addInterestingHeader("ANY");
        String sss = request -> url();

        Serial.println("GetHandler");
        return sss == "/Get/";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
        Serial.println("Handle Get!!");
                
      _Helper->GetDataJSON(request);
    }
};