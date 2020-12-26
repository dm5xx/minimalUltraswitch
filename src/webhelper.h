#pragma once
#include "RelayBoard.h"
#include "Boards.h"

class AsyncWebServerRequest; // nur wegen header vorwärtsdeklaration
class WebHelper {
    byte _numberOfRelayBoards;
    Boards _relayboard;
    const String _URLToCust;
    const String _URLToJS;
    const String _MyLocalIP;
    uint _myPort;
    bool _isLocked = false;
public:
    WebHelper(byte numberBoards, Boards &rb, const String URLToCust, const String URLToJS,  const String MyLocalIP, uint myPort);

    void SendJSONOKHeader(AsyncWebServerRequest *request);

    void SendHTMLOKHeader(AsyncWebServerRequest *request);

    void SendLocked(AsyncWebServerRequest *request);

    void SendFavicon(AsyncWebServerRequest *request);

    void Send200OK(AsyncWebServerRequest *request);

    void SendReset(AsyncWebServerRequest *request);

    void Send200JSONOK(AsyncWebServerRequest *request);

    void GetDataJSON(AsyncWebServerRequest *request);

    void MainPage(AsyncWebServerRequest *request);

    void Lock();

    void UnLock();

    bool GetLock();
};