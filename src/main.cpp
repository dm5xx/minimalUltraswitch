/****************************************************************************************************************************
  Async_AutoConnectWithFSParametersAndCustomIPr.ino
  For ESP8266 / ESP32 boards

  ESPAsync_WiFiManager is a library for the ESP8266/Arduino platform, using (ESP)AsyncWebServer to enable easy
  configuration and reconfiguration of WiFi credentials using a Captive Portal.

  Modified from 
  1. Tzapu               (https://github.com/tzapu/WiFiManager)
  2. Ken Taylor          (https://github.com/kentaylor)
  3. Alan Steremberg     (https://github.com/alanswx/ESPAsyncWiFiManager)
  4. Khoi Hoang          (https://github.com/khoih-prog/ESP_WiFiManager)

  Built by Khoi Hoang https://github.com/khoih-prog/ESPAsync_WiFiManager
  Licensed under MIT license
  Version: 1.4.1

  Version Modified By  Date      Comments
  ------- -----------  ---------- -----------
  1.0.11  K Hoang      21/08/2020 Initial coding to use (ESP)AsyncWebServer instead of (ESP8266)WebServer. Bump up to v1.0.11
                                  to sync with ESP_WiFiManager v1.0.11
  1.1.1   K Hoang      29/08/2020 Add MultiWiFi feature to autoconnect to best WiFi at runtime to sync with 
                                  ESP_WiFiManager v1.1.1. Add setCORSHeader function to allow flexible CORS
  1.1.2   K Hoang      17/09/2020 Fix bug in examples.
  1.2.0   K Hoang      15/10/2020 Restore cpp code besides Impl.h code to use if linker error. Fix bug.
  1.3.0   K Hoang      04/12/2020 Add LittleFS support to ESP32 using LITTLEFS Library
  1.4.0   K Hoang      18/12/2020 Fix staticIP not saved. Add functions. Add complex examples.
  1.4.1   K Hoang      21/12/2020 Fix bug and compiler warnings.
 *****************************************************************************************************************************/

//#define LDEBUG

#if !( defined(ESP8266) ||  defined(ESP32) )
#error This code is intended to run on the ESP8266 or ESP32 platform! Please check your Tools->Board setting.
#endif

#ifdef LDEBUG
// Use from 0 to 4. Higher number, more debugging messages and memory usage.
    #define _ESPASYNC_WIFIMGR_LOGLEVEL_       3
#else
    #define _ESPASYNC_WIFIMGR_LOGLEVEL_       0
#endif

// Default is 30s, using 20s now
#define TIME_BETWEEN_MODAL_SCANS          20000UL

// Default is 60s, using 30s now
#define TIME_BETWEEN_MODELESS_SCANS       30000UL

#include <FS.h>                   //this needs to be first, or it all crashes and burns...

//For ESP32, To use ESP32 Dev Module, QIO, Flash 4MB/80MHz, Upload 921600
//Ported to ESP32
#ifdef ESP32
  #include <esp_wifi.h>
  #include <WiFi.h>
  #include <WiFiClient.h>

  // From v1.1.1
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;

  // LittleFS has higher priority than SPIFFS
  #define USE_LITTLEFS    true
  #define USE_SPIFFS      false

  #if USE_LITTLEFS
    // Use LittleFS
    #include "FS.h"

    // The library will be depreciated after being merged to future major Arduino esp32 core release 2.x
    // At that time, just remove this library inclusion
    #include <LITTLEFS.h>             // https://github.com/lorol/LITTLEFS
    
    FS* filesystem =      &LITTLEFS;
    #define FileFS        LITTLEFS
    #define FS_Name       "LittleFS"
  #elif USE_SPIFFS
    #include <SPIFFS.h>
    FS* filesystem =      &SPIFFS;
    #define FileFS        SPIFFS
    #define FS_Name       "SPIFFS"
  #else
    // +Use FFat
    #include <FFat.h>
    FS* filesystem =      &FFat;
    #define FileFS        FFat
    #define FS_Name       "FFat"
  #endif
  //////

  #define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

  #define LED_BUILTIN       2
  #define LED_ON            HIGH
  #define LED_OFF           LOW

#else

  #include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
  //needed for library
  #include <DNSServer.h>

  // From v1.1.1
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;

  #define USE_LITTLEFS      true
  
  #if USE_LITTLEFS
    #include <LittleFS.h>
    FS* filesystem =      &LittleFS;
    #define FileFS        LittleFS
    #define FS_Name       "LittleFS"
  #else
    FS* filesystem =      &SPIFFS;
    #define FileFS        SPIFFS
    #define FS_Name       "SPIFFS"
  #endif
  //////
  
  #define ESP_getChipId()   (ESP.getChipId())
  
  #define LED_ON      LOW
  #define LED_OFF     HIGH
#endif

// For Config Portal
// SSID and PW for Config Portal
String ssid           = "ESP_" + String(ESP_getChipId(), HEX);
const char* password  = "your_password";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

// From v1.1.1
// You only need to format the filesystem once
//#define FORMAT_FILESYSTEM       true
#define FORMAT_FILESYSTEM         false

#define MIN_AP_PASSWORD_SIZE    8

#define SSID_MAX_LEN            32
//From v1.0.10, WPA2 passwords can be up to 63 characters long.
#define PASS_MAX_LEN            64

typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw  [PASS_MAX_LEN];
}  WiFi_Credentials;

typedef struct
{
  String wifi_ssid;
  String wifi_pw;
}  WiFi_Credentials_String;

#define NUM_WIFI_CREDENTIALS      2

typedef struct
{
  WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
} WM_Config;

WM_Config         WM_config;

#define  CONFIG_FILENAME              F("/wifi_cred.dat")
//////

// Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
bool initialConfig = false;

// SSID and PW for Config Portal
String AP_SSID;
String AP_PASS;

// Use false if you don't like to display Available Pages in Information Page of Config Portal
// Comment out or use true to display Available Pages in Information Page of Config Portal
// Must be placed before #include <ESPAsync_WiFiManager.h>
#define USE_AVAILABLE_PAGES     false

// From v1.0.10 to permit disable/enable StaticIP configuration in Config Portal from sketch. Valid only if DHCP is used.
// You'll loose the feature of dynamically changing from DHCP to static IP, or vice versa
// You have to explicitly specify false to disable the feature.
//#define USE_STATIC_IP_CONFIG_IN_CP          false

// Use false to disable NTP config. Advisable when using Cellphone, Tablet to access Config Portal.
// See Issue 23: On Android phone ConfigPortal is unresponsive (https://github.com/khoih-prog/ESP_WiFiManager/issues/23)
#define USE_ESP_WIFIMANAGER_NTP     false

// Use true to enable CloudFlare NTP service. System can hang if you don't have Internet access while accessing CloudFlare
// See Issue #21: CloudFlare link in the default portal (https://github.com/khoih-prog/ESP_WiFiManager/issues/21)
#define USE_CLOUDFLARE_NTP          false

// New in v1.0.11
#define USING_CORS_FEATURE          true
//////

// Use USE_DHCP_IP == true for dynamic DHCP IP, false to use static IP which you have to change accordingly to your network
#if (defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP)
// Force DHCP to be true
#if defined(USE_DHCP_IP)
#undef USE_DHCP_IP
#endif
#define USE_DHCP_IP     true
#else
// You can select DHCP or Static IP here
#define USE_DHCP_IP     true
//#define USE_DHCP_IP     false
#endif

#if ( USE_DHCP_IP || ( defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP ) )
// Use DHCP
#warning Using DHCP IP
IPAddress stationIP   = IPAddress(0, 0, 0, 0);
IPAddress gatewayIP   = IPAddress(192, 168, 2, 1);
IPAddress netMask     = IPAddress(255, 255, 255, 0);
#else
// Use static IP
#warning Using static IP
#ifdef ESP32
IPAddress stationIP   = IPAddress(192, 168, 2, 232);
#else
IPAddress stationIP   = IPAddress(192, 168, 2, 186);
#endif

IPAddress gatewayIP   = IPAddress(192, 168, 2, 1);
IPAddress netMask     = IPAddress(255, 255, 255, 0);
#endif

#define USE_CONFIGURABLE_DNS      true

IPAddress dns1IP      = gatewayIP;
IPAddress dns2IP      = IPAddress(8, 8, 8, 8);

IPAddress APStaticIP  = IPAddress(192, 168, 100, 39);
IPAddress APStaticGW  = IPAddress(192, 168, 100, 39);
IPAddress APStaticSN  = IPAddress(255, 255, 255, 0);

#include <ESPAsync_WiFiManager.h>              //https://github.com/khoih-prog/ESPAsync_WiFiManager

//#define HTTP_PORT           59

uint myPort = 59;

AsyncWebServer webServer(myPort);
DNSServer dnsServer;

// Now support ArduinoJson 6.0.0+ ( tested with v6.15.2 to v6.16.1 )
#include <ArduinoJson.h>        // get it from https://arduinojson.org/ or install via Arduino library manager

const char* CONFIG_FILE = "/config.json";

//define your default values here, if there are different values in config.json, they are overwritten.
//length should be max size + 1

char switch_url  [40]  = "0";
char custom_js  [40]  = "h.mmmedia-online.de/minimal63/";
char custom_cu  [40]  = "h.mmmedia-online.de/minimal63XX/";
char numberOfBoards    [2]   = "1";

//flag for saving data
bool shouldSaveConfig = false;

///////////////////////////////////////////
// New in v1.4.0
/******************************************
 * // Defined in ESPAsync_WiFiManager.h
typedef struct
{
  IPAddress _ap_static_ip;
  IPAddress _ap_static_gw;
  IPAddress _ap_static_sn;

}  WiFi_AP_IPConfig;

typedef struct
{
  IPAddress _sta_static_ip;
  IPAddress _sta_static_gw;
  IPAddress _sta_static_sn;
#if USE_CONFIGURABLE_DNS  
  IPAddress _sta_static_dns1;
  IPAddress _sta_static_dns2;
#endif
}  WiFi_STA_IPConfig;
******************************************/

WiFi_AP_IPConfig  WM_AP_IPconfig;
WiFi_STA_IPConfig WM_STA_IPconfig;

void initAPIPConfigStruct(WiFi_AP_IPConfig &in_WM_AP_IPconfig)
{
  in_WM_AP_IPconfig._ap_static_ip   = APStaticIP;
  in_WM_AP_IPconfig._ap_static_gw   = APStaticGW;
  in_WM_AP_IPconfig._ap_static_sn   = APStaticSN;
}

void initSTAIPConfigStruct(WiFi_STA_IPConfig &in_WM_STA_IPconfig)
{
  in_WM_STA_IPconfig._sta_static_ip   = stationIP;
  in_WM_STA_IPconfig._sta_static_gw   = gatewayIP;
  in_WM_STA_IPconfig._sta_static_sn   = netMask;
#if USE_CONFIGURABLE_DNS  
  in_WM_STA_IPconfig._sta_static_dns1 = dns1IP;
  in_WM_STA_IPconfig._sta_static_dns2 = dns2IP;
#endif
}

void displayIPConfigStruct(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
  LOGERROR3(F("stationIP ="), in_WM_STA_IPconfig._sta_static_ip, ", gatewayIP =", in_WM_STA_IPconfig._sta_static_gw);
  LOGERROR1(F("netMask ="), in_WM_STA_IPconfig._sta_static_sn);
#if USE_CONFIGURABLE_DNS
  LOGERROR3(F("dns1IP ="), in_WM_STA_IPconfig._sta_static_dns1, ", dns2IP =", in_WM_STA_IPconfig._sta_static_dns2);
#endif
}

void configWiFi(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
  #if USE_CONFIGURABLE_DNS  
    // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
    WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn, in_WM_STA_IPconfig._sta_static_dns1, in_WM_STA_IPconfig._sta_static_dns2);  
  #else
    // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
    WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn);
  #endif 
}

///////////////////////////////////////////

uint8_t connectMultiWiFi()
{
#if ESP32
  // For ESP32, this better be 0 to shorten the connect time
  #define WIFI_MULTI_1ST_CONNECT_WAITING_MS       0
#else
  // For ESP8266, this better be 2200 to enable connect the 1st time
  #define WIFI_MULTI_1ST_CONNECT_WAITING_MS       2200L
#endif

#define WIFI_MULTI_CONNECT_WAITING_MS           100L
  
  uint8_t status;

  LOGERROR(F("ConnectMultiWiFi with :"));
  
  if ( (Router_SSID != "") && (Router_Pass != "") )
  {
    LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass );
  }

  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
  {
    // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
    if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
    {
      LOGERROR3(F("* Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
    }
  }
  
  LOGERROR(F("Connecting MultiWifi..."));

  WiFi.mode(WIFI_STA);

#if !USE_DHCP_IP
  // New in v1.4.0
  configWiFi(WM_STA_IPconfig);
  //////
#endif

  int i = 0;
  status = wifiMulti.run();
  delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

  while ( ( i++ < 20 ) && ( status != WL_CONNECTED ) )
  {
    status = wifiMulti.run();

    if ( status == WL_CONNECTED )
      break;
    else
      delay(WIFI_MULTI_CONNECT_WAITING_MS);
  }

  if ( status == WL_CONNECTED )
  {
    LOGERROR1(F("WiFi connected after time: "), i);
    LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
    LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );

#ifdef LDEBUG
    Serial.println(F("Corrected url while reconecting.."));
    WiFi.localIP().toString().toCharArray(switch_url, 40);
    delay(20);
#endif
  }
  else
    LOGERROR(F("WiFi not connected"));

  return status;
}

//callback notifying us of the need to save config
void saveConfigCallback ()
{
#ifdef LDEBUG
  Serial.println(F("Should save config"));
#endif
  shouldSaveConfig = true;
}

void heartBeatPrint()
{
  static int num = 1;

#ifdef LDEBUG
  if (WiFi.status() == WL_CONNECTED)
    Serial.print(F("H"));        // H means connected to WiFi
  else
    Serial.print(F("F"));        // F means not connected to WiFi
#endif

  if (num == 80)
  {
#ifdef LDEBUG
    Serial.println();
#endif
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
#ifdef LDEBUG
    Serial.print(F(" "));
#endif
  }
}

void toggleLED()
{
  //toggle state
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void check_WiFi()
{
  if ( (WiFi.status() != WL_CONNECTED) )
  {
#ifdef LDEBUG
    Serial.println(F("\nWiFi lost. Call connectMultiWiFi in loop"));
#endif
    connectMultiWiFi();
  }
}  

bool shouldReboot = false;
bool shouldSettingsReset = false;


void check_status()
{
  static ulong checkstatus_timeout  = 0;
  static ulong LEDstatus_timeout    = 0;
  static ulong checkwifi_timeout    = 0;
  
  static ulong currentMillis;

#define HEARTBEAT_INTERVAL    10000L
#define LED_INTERVAL          2000L
#define WIFICHECK_INTERVAL    1000L

  currentMillis = millis();

  // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
  if ((currentMillis > checkwifi_timeout) || (checkwifi_timeout == 0))
  {
    check_WiFi();
    checkwifi_timeout = currentMillis + WIFICHECK_INTERVAL;
  }

  if ((currentMillis > LEDstatus_timeout) || (LEDstatus_timeout == 0))
  {
    // Toggle LED at LED_INTERVAL = 2s
    toggleLED();
    LEDstatus_timeout = currentMillis + LED_INTERVAL;

    if(shouldReboot || shouldSettingsReset)
    {
        if(shouldSettingsReset)
        {
            
            ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "ConfigOnSwitch");
#ifdef LDEBUG
            Serial.println(F("ESP rest!"));
#endif
            ESPAsync_wifiManager.resetSettings();
            WiFi.disconnect();
            delay(1000);
#ifdef LDEBUG
            Serial.println(F("ESP restarting..."));
#endif
        }
#ifdef LDEBUG
        Serial.println(F("I do reset.."));
#endif
        ESP.reset();
    }
  }

  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((currentMillis > checkstatus_timeout) || (checkstatus_timeout == 0))
  {
    heartBeatPrint();
    checkstatus_timeout = currentMillis + HEARTBEAT_INTERVAL;
  }
}

void loadConfigData()
{
  File file = FileFS.open(CONFIG_FILENAME, "r");
  LOGERROR(F("LoadWiFiCfgFile "));

  memset(&WM_config,       0, sizeof(WM_config));

  // New in v1.4.0
  memset(&WM_STA_IPconfig, 0, sizeof(WM_STA_IPconfig));
  //////
    
  if (file)
  {
    file.readBytes((char *) &WM_config,   sizeof(WM_config));

    // New in v1.4.0
    file.readBytes((char *) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
    //////
    
    file.close();
    LOGERROR(F("OK"));

    // New in v1.4.0
    displayIPConfigStruct(WM_STA_IPconfig);
    //////
  }
  else
  {
    LOGERROR(F("failed"));
  }
}
    
void saveConfigData()
{
  File file = FileFS.open(CONFIG_FILENAME, "w");
  LOGERROR(F("SaveWiFiCfgFile "));

  if (file)
  {
    file.write((uint8_t*) &WM_config,   sizeof(WM_config));

    // New in v1.4.0
    file.write((uint8_t*) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
    //////
    
    file.close();
    LOGERROR(F("OK"));
  }
  else
  {
    LOGERROR(F("failed"));
  }
}

//******************************************************************************************* Custom stuff ********************************************************************************
    
#include "RelayBoard.h"
#include "Boards.h"

#include "helper.h"
#include "webhelper.h"
#include <memory>

Boards relayArray = Boards();

#include "CustomSetHandler.h"
#include "CustomGetHandler.h"
#include "Custom_setHandler.h"
#include "CustomDefaultHandler.h"
#include "CustomLockHandler.h"
#include "CustomUnlockHandler.h"
#include "CustomFaviHandler.h"
#include "CustomResetHandler.h"
#include "CustomSettingsResetHandler.h"


//*****************************************************************************************************************************************************************************************
void setup()
{
  //set led pin as output
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_ON);
  pinMode(D0, INPUT_PULLUP);

  // Put your setup code here, to run once
  Serial.begin(115200);
  while (!Serial);

#ifdef LDEBUG
  Serial.print("\nStarting Async_AutoConnectWithFSParametersAndCustomIP using " + String(FS_Name));
  Serial.println(" on " + String(ARDUINO_BOARD));
  Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);
#endif

  //clean FS, for testing
  //FileFS.format();

  // Mount the filesystem
  bool result = FileFS.begin();

#ifdef LDEBUG
    #if USE_LITTLEFS
    Serial.print(F("\nLittleFS opened: "));
    #else
    Serial.print("\nSPIFFS opened: ");
    #endif

      Serial.println(result ? "OK" : "Failed");

  //read configuration from FS json
    Serial.println(F("Mounting FS..."));
#endif

  if (result) 
  {
#ifdef LDEBUG
    Serial.println(F("Mounted file system"));
#endif
    
    if (FileFS.exists(CONFIG_FILE)) 
    {
      //file exists, reading and loading
#ifdef LDEBUG
      Serial.println(F("Reading config file"));
#endif
      
      File configFile = FileFS.open(CONFIG_FILE, "r");
      
      if (configFile) 
      {
#ifdef LDEBUG
        Serial.println(F("Opened config file"));
#endif
        
        size_t size = configFile.size();
        
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

#if (ARDUINOJSON_VERSION_MAJOR >= 6)

        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());

        serializeJson(json, Serial);
        
        if ( !deserializeError )    
#else
        DynamicJsonBuffer jsonBuffer;
        
        // Parse JSON string
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        
        json.printTo(Serial);

        // Test if parsing succeeds.
        if (json.success()) 
#endif        
        {
#ifdef LDEBUG
          Serial.println(F("\nParsed json"));
#endif

          strcpy(switch_url, json["switch_url"]);
          strcpy(custom_js, json["custom_js"]);
          strcpy(custom_cu, json["custom_cu"]);
          strcpy(numberOfBoards,   json["numberOfBoards"]);
        }  
        else 
        {
#ifdef LDEBUG
          Serial.println(F("Failed to load json config"));
#endif
        }
      }
    }
  } 
  else 
  {
#ifdef LDEBUG
    Serial.println(F("Failed to mount FS"));
#endif
  }
  
  //end read
#ifdef LDEBUG
  Serial.println(String("Switch_url = ") + switch_url);
  Serial.println(String("NumberOfBoards   = ") + numberOfBoards);
  Serial.println(String("Custom_js = ") + custom_js);
  Serial.println(String("Custom_cu = ") + custom_cu);
#endif
  
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length

  //TODO change length!!!
  ESPAsync_WMParameter custom_switch_url ("switch_url",  "switch_url",  switch_url,  40);
  ESPAsync_WMParameter custom_numberOfBoards   ("numberOfBoards",    "numberOfBoards",    numberOfBoards,    2);
  ESPAsync_WMParameter custom_custom_js   ("custom_js",    "custom_js",    custom_js,    40);
  ESPAsync_WMParameter custom_custom_cu ("custom_cu",   "custom_cu",  custom_cu,  40);

  unsigned long startedAt = millis();

  // New in v1.4.0
  initAPIPConfigStruct(WM_AP_IPconfig);
  initSTAIPConfigStruct(WM_STA_IPconfig);
  //////

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  //ESPAsync_WiFiManager ESPAsync_wifiManager(&&webServer, &dnsServer,);
  //Here starts the WiFi Manager initialization
  //Local intialization. Once its business is done, there is no need to keep it around
  ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "AutoConnect-FSParams");

  //set config save notify callback
  ESPAsync_wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  ESPAsync_wifiManager.addParameter(&custom_switch_url);
  ESPAsync_wifiManager.addParameter(&custom_numberOfBoards);
  ESPAsync_wifiManager.addParameter(&custom_custom_js);
  ESPAsync_wifiManager.addParameter(&custom_custom_cu);

  //reset settings - for testing
  //ESPAsync_wifiManager.resetSettings();

  ESPAsync_wifiManager.setDebugOutput(true);

  //set custom ip for portal
  // New in v1.4.0
  ESPAsync_wifiManager.setAPStaticIPConfig(WM_AP_IPconfig);
  //////

  //set minimum quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //ESPAsync_wifiManager.setMinimumSignalQuality();
  ESPAsync_wifiManager.setMinimumSignalQuality(-1);

  // From v1.0.10 only
  // Set config portal channel, default = 1. Use 0 => random channel from 1-13
  ESPAsync_wifiManager.setConfigPortalChannel(0);
  //////
  
#if !USE_DHCP_IP    
    // Set (static IP, Gateway, Subnetmask, DNS1 and DNS2) or (IP, Gateway, Subnetmask). New in v1.0.5
    // New in v1.4.0
    ESPAsync_wifiManager.setSTAStaticIPConfig(WM_STA_IPconfig);
    //////
#endif

  // New from v1.1.1
#if USING_CORS_FEATURE
  ESPAsync_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");
#endif

  // We can't use WiFi.SSID() in ESP32 as it's only valid after connected.
  // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
  // Have to create a new function to store in EEPROM/SPIFFS/LittleFS for this purpose
  Router_SSID = ESPAsync_wifiManager.WiFi_SSID();
  Router_Pass = ESPAsync_wifiManager.WiFi_Pass();

  //Remove this line if you do not want to see WiFi password printed
#ifdef LDEBUG
  Serial.println("\nStored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);
#endif

  if (Router_SSID != "")
  {
    ESPAsync_wifiManager.setConfigPortalTimeout(120); //If no access point name has been previously entered disable timeout.
#ifdef LDEBUG
    Serial.println(F("Got stored Credentials. Timeout 120s"));
#endif
  }
  else
  {
#ifdef LDEBUG
    Serial.println(F("No stored Credentials. No timeout"));
#endif
  }

  String chipID = String(ESP_getChipId(), HEX);
  chipID.toUpperCase();

  // SSID and PW for Config Portal
  AP_SSID = "ESP_" + chipID + "_AutoConnectAP";
  AP_PASS = "MyESP_" + chipID;

  // From v1.1.0, Don't permit NULL password
  if ( (Router_SSID == "") || (Router_Pass == "") )
  {
#ifdef LDEBUG
    Serial.println(F("We haven't got any access point credentials, so get them now"));
#endif

    initialConfig = true;

    // Starts an access point
    //if (!ESPAsync_wifiManager.startConfigPortal((const char *) ssid.c_str(), password))
    if ( !ESPAsync_wifiManager.startConfigPortal(AP_SSID.c_str(), AP_PASS.c_str()) ) {
#ifdef LDEBUG
      Serial.println(F("Not connected to WiFi but continuing anyway."));
#endif
    }
    else {
#ifdef LDEBUG
      Serial.println(F("WiFi connected...yeey :)"));
#endif
    }

    // Stored  for later usage, from v1.1.0, but clear first
    memset(&WM_config, 0, sizeof(WM_config));
    
    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      String tempSSID = ESPAsync_wifiManager.getSSID(i);
      String tempPW   = ESPAsync_wifiManager.getPW(i);
  
      if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);

      if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);  

      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }

    // New in v1.4.0
    ESPAsync_wifiManager.getSTAStaticIPConfig(WM_STA_IPconfig);
    displayIPConfigStruct(WM_STA_IPconfig);
    //////
    
    saveConfigData();
  }
  else
  {
    wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());
  }

  startedAt = millis();

  if (!initialConfig)
  {
    // Load stored data, the addAP ready for MultiWiFi reconnection
    loadConfigData();

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }

    if ( WiFi.status() != WL_CONNECTED ) 
    {
#ifdef LDEBUG
      Serial.println(F("ConnectMultiWiFi in setup"));
#endif
      connectMultiWiFi();
    }
  }

#ifdef LDEBUG
  Serial.print(F("After waiting "));
  Serial.print((float) (millis() - startedAt) / 1000L);
  Serial.print(F(" secs more in setup(), connection result is "));
#endif

  if (WiFi.status() == WL_CONNECTED)
  {
#ifdef LDEBUG
    Serial.print(F("connected. Local IP: "));
    Serial.println(WiFi.localIP());
#endif
  }
  else
  {
    auto r = ESPAsync_wifiManager.getStatus(WiFi.status());
#ifdef LDEBUG
    Serial.println(r);
#endif
  }

  //read updated parameters
  strcpy(switch_url, custom_switch_url.getValue());
  strcpy(numberOfBoards, custom_numberOfBoards.getValue());
  strcpy(custom_js, custom_custom_js.getValue());
  strcpy(custom_cu, custom_custom_cu.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) 
  {
#ifdef LDEBUG
    Serial.println(F("Saving config"));
#endif

#if (ARDUINOJSON_VERSION_MAJOR >= 6)
    DynamicJsonDocument json(1024);
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
#endif
    
    if(switch_url[0] == '0')
    {
#ifdef LDEBUG
        Serial.println(F("Corrected url while reconecting.."));
#endif
        WiFi.localIP().toString().toCharArray(switch_url, 40);
        delay(20);
    }

    json["numberOfBoards"]   = numberOfBoards;
    json["switch_url"] = switch_url;
    json["custom_js"] = custom_js;
    json["custom_cu"] = custom_cu;

    File configFile     = FileFS.open(CONFIG_FILE, "w");
    
    if (!configFile) 
    {
#ifdef LDEBUG
      Serial.println(F("Failed to open config file for writing"));
#endif
    }

#if (ARDUINOJSON_VERSION_MAJOR >= 6)
    serializeJsonPretty(json, Serial);
    // Write data to file and close it
    serializeJson(json, configFile);
#else
    json.prettyPrintTo(Serial);
    // Write data to file and close it
    json.printTo(configFile);
#endif
    
    configFile.close();
    //end save
  }

#ifdef LDEBUG
  Serial.print(F("Local IP = "));
  Serial.println(WiFi.localIP());
  Serial.print(F("GateWay IP = "));
  Serial.println(WiFi.gatewayIP());
  Serial.print(F("SubnetMask = ")); 
  Serial.println(WiFi.subnetMask());

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(F("NOT CONNECTED  NOT CONNECT"));
  }
  else
  {
    Serial.print(F("I AM CONNECTED"));
  }
#endif
  


  //keep LED off
  digitalWrite(LED_BUILTIN, LED_OFF);

byte counter = 0;

while(counter < 5)
{
    byte buttonVal = digitalRead(D0);
    
    if (buttonVal == LOW) {
          
#ifdef LDEBUG
          Serial.println(F("ESP rest!"));
#endif
          ESPAsync_wifiManager.resetSettings();
          WiFi.disconnect();
          delay(1000);
#ifdef LDEBUG
          Serial.println(F("ESP restarting..."));
#endif
          delay(1000);
          ESP.restart();
    }
    else
    {
#ifdef LDEBUG
      Serial.println(F("NotPushed!"));
#endif
    }
    
    delay(200);
    counter++;
}

byte numberOfRelayBoards = String(numberOfBoards).toInt();
relayArray.Init(numberOfRelayBoards);

auto helper = std::make_shared<Helper>(numberOfRelayBoards, relayArray);
auto webhelper = std::make_shared<WebHelper>(numberOfRelayBoards, relayArray, String(custom_cu), String(custom_js),  String(switch_url), myPort);

webServer.addHandler(new CustomDefaultHandler(webhelper));
webServer.addHandler(new CustomSetHandler(webhelper, helper));
webServer.addHandler(new Custom_setHandler(webhelper));
webServer.addHandler(new CustomGetHandler(webhelper));
webServer.addHandler(new CustomLockHandler(webhelper));
webServer.addHandler(new CustomUnlockHandler(webhelper));
webServer.addHandler(new CustomResetHandler(webhelper, &shouldReboot));
webServer.addHandler(new CustomSettingsResetHandler(webhelper, &shouldSettingsReset));
webServer.addHandler(new CustomFaviHandler(webhelper));

#ifdef LDEBUG
    Serial.println(F("Starting"));
#endif
webServer.begin();

}

void loop()
{  
  check_status();
}
