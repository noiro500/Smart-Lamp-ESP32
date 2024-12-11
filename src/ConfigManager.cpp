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
    if (strcmp(config.WifiMode, "WIFI_AP") == 0)
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
    else if (strcmp(config.WifiMode, "WIFI_STA") == 0)
    {
        WiFi.mode(WIFI_STA);
        /*Automatically connect to the last successful network*/
        /*// Автоматически подключаемся к последней успешной сети*/
        WiFi.setAutoConnect(true);  
        /*Automatically try to reconnect when connection is lost*/
        /*Автоматически пытаемся переподключиться при потере соединения*/ 
        WiFi.setAutoReconnect(true); 

        WiFi.begin(config.WifiSsid, config.WiFiPassword);

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.println("Connecting to WiFi..");
        }

        Serial.println("Connected to the WiFi network");
        Serial.println(WiFi.localIP());
        ESP32Time rtcToWiFiStart;
        configTime(TIMEZONE * 3600, DAYLIGHTOFFSET, "ntp0.ntp-servers.net");
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            rtcToWiFiStart.setTimeStruct(timeinfo);
        }
        Serial.printf("%s\n", (rtc.getDateTime(true)).c_str());
        delay(3000);
    }
}

 /*Get temperature from am2320 sensor*/
 /*Получить температуру с датчика am2320*/
std::unique_ptr<float[]> GetMeasurementsFromSensor()
{
    std::unique_ptr<float[]> array(new float[2]);
    AM2320 am2320(&Wire);
    Wire.begin(19, 23);
    switch (am2320.Read())
    {
    case 2:
        // Error reading AM2320 sensor
        {
            array[0] = -50.00f;
            array[1] = -60.00f;
            Wire.end();
            return array;
        }
    case 1:
        // AM2320 sensor offline
        {
            array[0] = -60.00f;
            array[1] = -70.00f;
            Wire.end();
            return array;
        }
    case 0:
    {
        float tempTemp = 0, tempHum = 0;
        int i = 0;
        while (i < 3)
        {
            tempTemp += am2320.cTemp;
            tempHum += am2320.Humidity;
            i++;
            delay(2000);
        }
        array[0] = tempTemp / i;
        array[1] = tempHum / i;
        Wire.end();
        return array;
    }
    default:
        /*Возвращаем какое-то значение по умолчанию, если вдруг произошло что-то неожиданное*/
        /*Return some default value if something unexpected happens*/
        {
            array[0] = -1000.00f;
            array[1] = -1000.00f;
            Wire.end();
            return array;
        }
    }
}

void PrepareConfigString(const ConfigValues &config, std::unique_ptr<char[]> &configString)
{
    size_t estimatedSize = 512;
    configString.reset(new char[estimatedSize]);
    snprintf(configString.get(), estimatedSize,
             "%d, %d.%d.%d.%d, %d.%d.%d.%d, %s, %s, %s, %d, %d, %d",
             config.WifiMode,
             config.SoftApIP[0], config.SoftApIP[1], config.SoftApIP[2], config.SoftApIP[3],
             config.SoftIpSubnetMask[0], config.SoftIpSubnetMask[1], config.SoftIpSubnetMask[2], config.SoftIpSubnetMask[3],
             config.WiFiPassword,
             config.SoftApSsid,
             config.WifiSsid,
             config.LampAlwayseOn,
             config.LampOnTimeHours,
             config.LampOffTimeHours);
}