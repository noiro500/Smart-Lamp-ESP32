#include "ConfigManager.h"
#include "WiFi_AP.h"
#include "ESP32Time.h"
#include "AM2320.h"

Preferences preferences;

void SaveConfigToNVS(ConfigValues &config)
{
    preferences.begin("config", false);
    preferences.clear();
    preferences.putBytes("config_data", &config, sizeof(config));
    preferences.end();
}

bool LoadConfigFromNVC(ConfigValues &config)
{
    preferences.begin("config", true);
    if (preferences.getBytesLength("config_data") == sizeof(ConfigValues))
    {
        preferences.getBytes("config_data", &config, sizeof(ConfigValues));
    }
    else
    {
        return false;
    }
    preferences.end();
    return true;
}

void ConfigWiFi(ConfigValues &config)
{
    if (strcmp(config.WiFiMode, "WIFI_AP") == 0)
    {
        IPAddress apIP(config.SoftApIP);
        IPAddress netMask(config.SoftIpSubnetMask);
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(apIP, apIP, netMask);
        Serial.println("Setting up WiFi Access Point...");
        WiFi.softAP(config.SoftApSsid, config.WiFiPassword);
        Serial.println("AP IP address: " + String(apIP.toString()));
        Serial.println("SoftApSsid: " + String(config.SoftApSsid));
        bool dhcp = dnsServer.start(53, "*", apIP);
        if (!dhcp)
        {
            Serial.println("DHCP server failed to start");
            delay(5000);
            ESP.restart();
        }
        Serial.println("DHCP server started");
    }
    else if (strcmp(config.WiFiMode, "WIFI_STA") == 0)
    {
        WiFi.mode(WIFI_STA);
        /*Automatically try to reconnect when connection is lost*/
        /*Автоматически пытаемся переподключиться при потере соединения*/
        WiFi.setAutoReconnect(true);
        WiFi.begin((strcat(config.WiFiSsid, "\0")), strcat(config.WiFiPassword, "\0"));
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.println("Connecting to WiFi..");
        }

        Serial.println("Connected to the WiFi network");
        Serial.println(WiFi.localIP());
        Serial.println(WiFi.macAddress());

        ESP32Time rtcToWiFiStart;
        configTime(TIMEZONE * 3600, DAYLIGHTOFFSET, NTP_SERVER);
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            rtcToWiFiStart.setTimeStruct(timeinfo);
        }
        Serial.printf("%s\n", (rtc.getDateTime(true)).c_str());
        delay(3000);
    }
}

void PrepareConfigString(const ConfigValues &config, std::unique_ptr<char[]> &configString)
{
    size_t estimatedSize = 512;
    configString.reset(new char[estimatedSize]);
    snprintf(configString.get(), estimatedSize,
             "%s, %d.%d.%d.%d, %d.%d.%d.%d, %s, %s, %s, %d, %d, %d",
             config.WiFiMode,
             config.SoftApIP[0], config.SoftApIP[1], config.SoftApIP[2], config.SoftApIP[3],
             config.SoftIpSubnetMask[0], config.SoftIpSubnetMask[1], config.SoftIpSubnetMask[2], config.SoftIpSubnetMask[3],
             config.WiFiPassword,
             config.SoftApSsid,
             config.WiFiSsid,
             config.LampAlwayseOn,
             config.LampOnTimeHours,
             config.LampOffTimeHours);
}