#ifndef _NETWORKMANAGER_H_
#define _NETWORKMANAGER_H_

#include <esp_wifi.h>
#include "WiFi.h"
#include <ESPAsyncWiFiManager.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>

static uint8_t WIFI_PROTOS[4] = {
    0,
    WIFI_PROTOCOL_11B,
    WIFI_PROTOCOL_11B + WIFI_PROTOCOL_11G,
    WIFI_PROTOCOL_11G
};

class NetworkManager
{
private:
    Stream * m_Debug;
    Preferences * m_Prefs;
    uint8_t m_WifiProtocol = 0;

    void (*m_NmStatusCallbackFunc)(int);
    AsyncWiFiManager * wm = NULL;
    WiFiClient espClient;    
    int failure_count_network = 0;
public:
    AsyncWebServer * m_WebServer;
    DNSServer * m_DnsServer;
public:
    NetworkManager(Preferences * prefs, Stream * debug);   
    void begin();
    void loop();
    void reset();
private:
    void reinit_wifi();
    void forceWifiProtocol();
    void disconnection_handling(int failure_number);
    void restartESP();
    void callback(char* topic, byte* payload, unsigned int length) {}
};




#endif