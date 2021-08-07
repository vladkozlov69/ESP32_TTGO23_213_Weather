#include <NetworkManager.h>

NetworkManager::NetworkManager(Preferences * prefs, Stream * debug)
{
    m_Prefs = prefs;
    m_Debug = debug;
}

void NetworkManager::begin()
{
    m_WebServer = new AsyncWebServer(80);
    m_DnsServer = new DNSServer();

    wm = new AsyncWiFiManager(m_WebServer, m_DnsServer);

    WiFi.mode(WIFI_STA); 
    char ssid[20];
    char pass[20];
    int ssid_len = 0;

    m_Prefs->begin("network");
    // m_Prefs->clear();
    // wm.erase(true);
    ssid_len = m_Prefs->getString("n_ssid", ssid, sizeof(ssid));
    m_Prefs->getString("n_pass", pass, sizeof(pass));
    m_Prefs->end();

    String mac = WiFi.macAddress();
    mac.replace(":", "");

    wm->resetSettings();
    if (m_Debug)
    {
        m_Debug->println(F("Starting autoconnect..."));
    }
    AsyncWiFiManagerParameter owm_key("owm_key", "mqtt server", "", 40);
    AsyncWiFiManagerParameter accu_key("accu_key", "mqtt prefix", mac.c_str(), 40);
    AsyncWiFiManagerParameter climacell_key("climacell_key", "mqtt prefix", mac.c_str(), 40);
    // latlong
    // owm tz
    // accuLocation
    // climacellTimezone
    // gmtOffset_sec
    // daylightOffset_sec

    wm->addParameter(&owm_key);
    wm->addParameter(&accu_key);
    wm->addParameter(&climacell_key);
    if(wm->startConfigPortal("AutoConnectAP"))
    {
        if (m_Debug)
        {
            m_Debug->println(F("connected...yeey :)"));
            m_Debug->print(F("SSID:"));
            m_Debug->println(WiFi.SSID());
            m_Debug->print(F("PASS:"));
            m_Debug->println(WiFi.psk());
        }

        m_Prefs->begin("network");
        m_Prefs->putString("n_ssid", WiFi.SSID().c_str());
        m_Prefs->putString("n_pass", WiFi.psk().c_str());
        m_Prefs->end();

        if (strlen(custom_mqtt_server.getValue()) > 0)
        {
            if (m_Debug)
            {
                m_Debug->print(F("Saving mqtt:"));
                m_Debug->println(custom_mqtt_server.getValue());
            }
            m_Prefs->begin("network");
            m_Prefs->putString("n_mqtt", custom_mqtt_server.getValue());
            m_Prefs->putString("n_mpfx", custom_mqtt_prefix.getValue());
            m_Prefs->end();
        }
    } 
    else 
    {
        if (m_Debug)
        {
            m_Debug->println(F("Not connected"));
        }
    }
}

void NetworkManager::loop()
{
    if (WiFi.status() == WL_CONNECTED) 
    {
        // dnsHelper.loop();
        m_NmStatusCallbackFunc(1000);
        failure_count_network = 0;

        // if (mqttReady())
        // {
        //     if (millis() - m_LastSent > 60000)
        //     {
        //         m_MqttSendCallbackFunc(mqttClient, mqttPrefix.c_str());
        //         updateStats();
        //         m_LastSent = millis();
        //     }
        // }

        // AsyncElegantOTA.loop();
    }
    else 
    { 
        if (m_Debug)
        {
            m_Debug->println(F("Wifi disconnected"));
        }
        m_NmStatusCallbackFunc(100);
        failure_count_network++;
        disconnection_handling(failure_count_network);
    }
}

void NetworkManager::reset()
{
    m_Prefs->begin("network");
    m_Prefs->clear();
    m_Prefs->end();
#ifdef ESP32
    WiFi.disconnect(true, true);
#endif
#ifdef ESP8266
    wm->resetSettings();
#endif
}

void NetworkManager::restartESP()
{
    ESP.restart();
}

void NetworkManager::disconnection_handling(int failure_number) 
{
    if (failure_number > 12)
    {
        if (m_Debug)
        {
            m_Debug->println(F("Restarting esp32..."));
        }
        restartESP();
    }

    delay(100);

    if (m_Debug)
    {
        m_Debug->println(F("Reconnecting esp32 wifi..."));
    }


    #ifdef ESP32
    int index = (failure_number/10) % 4;
    m_WifiProtocol = WIFI_PROTOS[index];
    reinit_wifi();
    #else
    WiFi.disconnect();
    WiFi.reconnect();
    #endif
}

void NetworkManager::forceWifiProtocol() 
{
    #ifdef ESP32
    if (m_Debug)
    {
        m_Debug->print(F("ESP32: Forcing to wifi ")); m_Debug->println(m_WifiProtocol);
    }
    esp_wifi_set_protocol(WIFI_IF_STA, m_WifiProtocol);
    #endif
}

void NetworkManager::reinit_wifi() 
{
    #ifdef ESP32
    delay(10);
    WiFi.mode(WIFI_STA);
    if (m_WifiProtocol) forceWifiProtocol();
    WiFi.begin();
    #endif
}


