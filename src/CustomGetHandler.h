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
        String sss = request -> url();

#ifdef LDEBUG
        Serial.println("GetHandler");
#endif
        return sss == "/Get/";
    }

    void handleRequest(AsyncWebServerRequest *request) {   

#ifdef LDEBUG
        Serial.println("Handle Get!!");
#endif
                
      _Helper->GetDataJSON(request);
    }
};