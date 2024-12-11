#include "ServerHandlers.h"
#include "WiFi_AP.h"
#include "ConfigManager.h"
#include "Json.h"

AM2320 am2320(&Wire);
ESP32Time rtc;
ConfigValues config;

void HandleRoot()
{
    server.send(200, "text/html", "<h1>ESP32 Cactus Lamp</h1>");
}

void HandleGetTempAndHum()
{
    xSemaphoreTake(xMutexSensor, portMAX_DELAY);
    Wire.begin(19, 23);
    switch (am2320.Read())
    {
    case 2:
        server.send(503, "text/html", "Error reading AM2320 sensor");
        break;
    case 1:
        server.send(503, "text/html", "AM2320 sensor offline");
        break;
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
        Json measurementJson;
        measurementJson.add("Temperature", tempTemp / i);
        measurementJson.add("Humidity", tempHum / i);
        server.send(200, "application/json", measurementJson.toString());
        break;
    }
    }
    Wire.end();
    xSemaphoreGive(xMutexSensor);
}

void HandleSetRtc()
{
    const char *urlExample = "http://x.x.x.x:180/settime?hour=0&min=0&sec=0&day=1&month=1&year=2022";
    if (WiFi.getMode() == WIFI_AP && server.args() == 0)
    {
        server.send(400, "text/html", "Set time with example: " + String(urlExample));
        return;
    }
    else if (WiFi.getMode() == WIFI_AP && server.args() != 6)
    {
        server.send(400, "text/html", "Set time with example: " + String(urlExample));
        return;
    }
    else if (WiFi.getMode() == WIFI_AP && server.args() == 6)
    {
        rtc.setTime(server.arg("hour").toInt(), server.arg("min").toInt(), server.arg("sec").toInt(), server.arg("day").toInt(), server.arg("month").toInt(), server.arg("year").toInt());
    }
    if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED)
    {
        configTime(3 * 3600, 0, "ntp0.ntp-servers.net");
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            rtc.setTimeStruct(timeinfo);
        }
    }
    server.send(200, "text/html", "Time is set to: " + rtc.getDateTime(true));
}

void HandleGetRtc()
{
    server.send(200, "text/html", rtc.getDateTime(true));
}

void HandleTemperatureInHours()
{
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    Json returnJson;
    JsonArray hoursJsonArray;
    JsonArray temperaturesJsonArray;
    JsonArray humidityJsonArray;
    for (int i = 0; i < 24; i++)
    {
        hoursJsonArray.push(temperatureInHours.hour[i]);
        temperaturesJsonArray.push(temperatureInHours.temperature[i]);
        humidityJsonArray.push(temperatureInHours.humidity[i]);
    }
    returnJson.add("hour", hoursJsonArray);
    returnJson.add("temperature", temperaturesJsonArray);
    returnJson.add("humidity", humidityJsonArray);
    xSemaphoreGive(xMutexConfig);
    server.send(200, "application/json", returnJson.toString());
}

void HandleSetWiFiSTAParam()
{
    const char *urlExample = "http://x.x.x.x:180/setwifistaparam?ssid=MyWiFi&password=12345678";
    if (server.args() != 2)
    {
        server.send(400, "text/html", "Set WiFi STA parameters with example: " + String(urlExample));
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        server.send(500, "text/html", "Unable to load config from NVS");
        return;
    }
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    strcpy(config.WifiSsid, server.arg("ssid").c_str());
    strcpy(config.WiFiPassword, server.arg("password").c_str());
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
    server.send(400, "text/html", "WiFi STA parameters are set:" + String(config.WifiSsid) + ", " + String(config.WiFiPassword));
    delay(5000);
    ESP.restart();
}

void HandleSetLampTime()
{
    const char *urlExample = "http://x.x.x.x:180/setlamptime?on=8&off=22";
    if (server.args() != 2)
    {
        server.send(400, "text/html", "Set on/off time with example: " + String(urlExample));
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        server.send(500, "text/html", "Unable to load config from NVS");
        return;
    }
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    config.LampOnTimeHours = server.arg("on").toInt();
    config.LampOffTimeHours = server.arg("off").toInt();
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
    server.send(200, "text/html", "Lamp time is set to:\n" + String("On: ") + String(config.LampOnTimeHours) + " hours\n" + "Off: " + String(config.LampOffTimeHours) + " hours\n");
}

void HandleLampOlwayseOn()
{

    const char *urlExample = "http://x.x.x.x:180/lampalwayson?on=0";
    if (server.args() != 1)
    {
        server.send(400, "text/html", "Set parameter with example: " + String(urlExample));
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        server.send(500, "text/html", "Unable to load config from NVS");
        return;
    }
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    config.LampAlwayseOn = server.arg("on").toInt();
    SaveConfigToNVS(config);
    server.send(200, "text/html", "Lamp alwayse on is set to: " + String(config.LampAlwayseOn));
    xSemaphoreGive(xMutexConfig);
}

void HandleGetConfigValues()
{
    if (!LoadConfigFromNVC(config))
    {
        server.send(500, "text/html", "Unable to load config from NVS");
        return;
    }
    std::unique_ptr<char[]> configString;
    PrepareConfigString(config, configString);
    server.send(200, "text/html", configString.get());
}

void HandleChangeWiFiMode()
{
    const char *urlExample = "http://x.x.x.x:180/changewifimode?mode=WIFI_AP";
    if (server.args() != 1)
    {
        server.send(400, "text/html", "server.args() != 1 Set parameter only WIFI_STA or WIFI_AP with example: " + String(urlExample));
        return;
    }
    if (server.arg("mode") != String("WIFI_STA") && server.arg("mode") != String("WIFI_AP"))
    {
        server.send(400, "text/html", "Set parameter only WIFI_STA or WIFI_AP with example: " + String(urlExample));
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        server.send(500, "text/html", "Unable to load config from NVS");
        return;
    }

    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    strcpy(config.WifiMode, server.arg("mode").c_str());
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
    server.send(200, "text/html", "Wifi mode is set to: " + String(config.WifiMode));
    delay(5000);
    ESP.restart();
}

void HandleRebootDevice()
{
    server.send(200, "text/html", "Restart after 5 seconds");
    delay(5000);
    ESP.restart();
}