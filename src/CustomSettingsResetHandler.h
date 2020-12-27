#pragma once
#include <ESPAsyncWebServer.h>
#include "webhelper.h"

class CustomSettingsResetHandler  : public AsyncWebHandler {
    std::shared_ptr<WebHelper> _Helper;
    bool &_shouldSettingsReset;
public:
    CustomSettingsResetHandler(std::shared_ptr<WebHelper> hlp, bool* shouldSettingsReset)  : _Helper(hlp), _shouldSettingsReset(*shouldSettingsReset)
    {
    }

    bool canHandle(AsyncWebServerRequest *request){
        String sss = request -> url();
#ifdef LDEBUG
        Serial.println("SettingsResetHandler");
#endif
        return sss == "/XSettingsReset/DM5XX";
    }

    void handleRequest(AsyncWebServerRequest *request) {   
#ifdef LDEBUG
        Serial.println("Handle SettingsReset!!");
#endif
        _Helper->SendReset(request);
        _shouldSettingsReset = true;
    }
};