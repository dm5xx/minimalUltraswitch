#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "webhelper.h"

WebHelper::WebHelper(byte numberBoards, Boards &rb, const String URLToCust, const String URLToJS,  const String MyLocalIP, uint myPort) : _numberOfRelayBoards(numberBoards), 
    _relayboard(rb), _URLToCust(URLToCust), _URLToJS(URLToJS), _myPort(myPort), _MyLocalIP(MyLocalIP)
{

    Serial.println("Holla");
    Serial.println(_URLToCust);
    Serial.println("Waldfee");
}

void WebHelper::Lock()
{
    _isLocked = true;
}

void WebHelper::UnLock()
{
    _isLocked = false;
}


bool WebHelper::GetLock()
{
    return _isLocked;
}

void WebHelper::SendJSONOKHeader(AsyncWebServerRequest *request)
{
    request->addInterestingHeader("HTTP/1.1 200 OK");
    request->addInterestingHeader("Access-Control-Allow-Origin: *");
    request->addInterestingHeader("Content-Type: application/json");
    request->addInterestingHeader("Connection: close");  // the connection will be closed after completion of the response   client.println(F(""));
}

void WebHelper::SendHTMLOKHeader(AsyncWebServerRequest *request)
{
    request->addInterestingHeader("HTTP/1.1 200 OK");
    request->addInterestingHeader("Access-Control-Allow-Origin: *");
    request->addInterestingHeader("Content-Type: text/html");
    request->addInterestingHeader("Connection: close");  // the connection will be closed after completion of the response   client.println(F(""));
}

void WebHelper::SendLocked(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");    
    SendJSONOKHeader(request);

    response->printf("");
    response->printf("{\"Lockingstatus\": ");

    response->print(GetLock());
    response->printf("}");
    request->send(response);
}

void WebHelper::SendFavicon(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("text/html");    
    SendHTMLOKHeader(request);
    response->printf("");
    response->printf("<!DOCTYPE html>");
    response->printf("<HTML>");
    response->printf("<link rel=\"icon\" href=\"data:;base64,=\">");
    response->printf("</HTML>");
    request->send(response);
}

void WebHelper::Send200OK(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("text/html");    
    SendHTMLOKHeader(request);
    response->printf("");
    request->send(response);
}

void WebHelper::SendReset(AsyncWebServerRequest *request)
{
    Send200OK(request);
}


void WebHelper::GetDataJSON(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");    
    SendJSONOKHeader(request);
    response->printf("{\n\t\"B0\": ");
    response->print(_relayboard.GetBoardByNumber(0)->GetStatus());

    if(_numberOfRelayBoards > 1)
    {
        for(byte a= 0; a < _numberOfRelayBoards; a++)
        {
            response->printf(",\n\t \"B");
            response->print(a);
            response->printf("\": ");
            response->print(_relayboard.GetBoardByNumber(a)->GetStatus());
        }
    }
    response->printf(", \n\t \"LockStatus\": ");
    response->print(GetLock());
    response->printf("\n}");
    request->send(response);
}

void WebHelper::MainPage(AsyncWebServerRequest *request)
{
    Serial.println("Holla");
    Serial.println(_URLToCust);
    Serial.println("Waldfee");
    AsyncResponseStream *response = request->beginResponseStream("text/html");    
    SendHTMLOKHeader(request);
    response->printf(" ");
    response->printf("<!DOCTYPE html>\n");
    response->printf("<HTML>\n");
    response->printf("<HEAD>\n");
    response->printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>\n");
    response->printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"http://");
    response->print(_URLToJS);
    response->printf("style.css\" media=\"screen\"/>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToJS);
    response->printf("init.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToJS);
    response->printf("ShortCut.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToCust);
    response->printf("Custom_c.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToCust);
    response->printf("Profile_c.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToCust);
    response->printf("Disable_c.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToCust);
    response->printf("Label_c.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToCust);
    response->printf("BankDef_c.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToJS);
    response->printf("Globals.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToCust);
    response->printf("LockDef_c.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToJS);
    response->printf("Lock.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToCust);
    response->printf("GroupDef_c.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToJS);
    response->printf("Group.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToJS);
    response->printf("UiHandler.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToJS);
    response->printf("GetData.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToJS);
    response->printf("SetData.js\"></script>\n");
    response->printf("<script language=\"javascript\" type=\"text/javascript\" src=\"http://");
    response->print(_URLToJS);
    response->printf("Helper.js\"></script>\n");
    response->printf("\n");
    response->printf("<!-- Change SwitchURL to the url, where your webswitch is reachable from outside/inside. Dont forget the portforwarding...-->\n");
    response->printf("<script>var url='");
    response->print(_MyLocalIP);
    response->printf(":");
    response->print(_myPort);
    response->printf("';\r");
    response->printf("</script>\n");
    response->printf("<TITLE>\n");
    response->printf("minimalUltraswitch - Remote Switch by DM5XX\n");
    response->printf("</TITLE>\n");
    response->printf("</HEAD>\n");
    response->printf("<BODY>\n");
    response->printf("<div class=\"grid-container\" id=\"container\"></div>\n");
    response->printf("</BODY>\n");
    response->printf("<script>(() => { init(); })()</script>\n");
    response->printf("</HTML>\n");
    request->send(response);
}

void WebHelper::Send200JSONOK(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");    
    SendJSONOKHeader(request);
    response->print(F("{\"Status\": \"OK\"}"));
    request->send(response);
}

