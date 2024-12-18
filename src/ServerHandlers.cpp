#include "ServerHandlers.h"
#include "ConfigManager.h"
#include "SensorMeasurements.h"

// AM2320 am2320(&Wire);
ESP32Time rtc;
ConfigValues config;

void HandleRoot(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", GREETINGS);
}

void HandleGetTempAndHum(AsyncWebServerRequest *request)
{
    xSemaphoreTake(xMutexSensor, portMAX_DELAY);
    if (tempAndHumCachedResponse)
    {
        request->send(200, "application/json", tempAndHumCachedResponse.get());
    }
    else
    {
        request->send(500, "application/json", "Error: No data available");
    }
    xSemaphoreGive(xMutexSensor);
}

void HandleSetTime(AsyncWebServerRequest *request)
{
    static constexpr const char *urlExample = "http://x.x.x.x:180/settime?hour=0&min=0&sec=0&day=1&month=1&year=2022";

    if (WiFi.getMode() == WIFI_AP)
    {
        if (request->args() != 6)
        {
            request->send(400, "text/html", "Set time with example: " + String(urlExample));
            return;
        }
        rtc.setTime(request->arg("hour").toInt(), request->arg("min").toInt(), request->arg("sec").toInt(),
                    request->arg("day").toInt(), request->arg("month").toInt(), request->arg("year").toInt());
    }
    else if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED)
    {
        configTime(TIMEZONE * 3600, DAYLIGHTOFFSET, NTP_SERVER);
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            rtc.setTimeStruct(timeinfo);
        }
    }
    request->send(200, "text/html", "Time is set to: " + rtc.getDateTime(true));
}

void HandleGetTime(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", rtc.getDateTime(true));
}

void HandleTemperatureInHours(AsyncWebServerRequest *request)
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
    request->send(200, "application/json", returnJson.toString());
}

void HandleSetWiFiSTAParam(AsyncWebServerRequest *request)
{
    static constexpr const char *urlExample = "http://x.x.x.x:180/setwifistaparam?ssid=MyWiFi&password=12345678";
    if (request->args() != 2)
    {
        request->send(400, "text/html", "Set WiFi STA parameters with example: " + String(urlExample));
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        request->send(500, "text/html", "Unable to load config from NVS");
        return;
    }
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    strncpy(config.WiFiSsid, request->arg("ssid").c_str(), sizeof(config.WiFiSsid) - 1);
    strncpy(config.WiFiPassword, request->arg("password").c_str(), sizeof(config.WiFiPassword) - 1);
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
    request->send(200, "text/html", "WiFi STA parameters are set:" + String(config.WiFiSsid) + ", " + String(config.WiFiPassword));
    delay(5000);
    ESP.restart();
}

void HandleSetLampTime(AsyncWebServerRequest *request)
{
    static constexpr const char *urlExample = "http://x.x.x.x:180/setlamptime?on=8&off=22";
    if (request->args() != 2)
    {
        request->send(400, "text/html", "Set on/off time with example: " + String(urlExample));
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        request->send(500, "text/html", "Unable to load config from NVS");
        return;
    }
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    config.LampOnTimeHours = request->arg("on").toInt();
    config.LampOffTimeHours = request->arg("off").toInt();
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
    request->send(200, "text/html", "Lamp time is set to:\n" + String("On: ") + String(config.LampOnTimeHours) + " hours\n" + "Off: " + String(config.LampOffTimeHours) + " hours\n");
}

void HandleLampOlwayseOn(AsyncWebServerRequest *request)
{

    static constexpr const char *urlExample = "http://x.x.x.x:180/lampalwayson?on=0";
    if (request->args() != 1)
    {
        request->send(400, "text/html", "Set parameter with example: " + String(urlExample));
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        request->send(500, "text/html", "Unable to load config from NVS");
        return;
    }
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    config.LampAlwayseOn = request->arg("on").toInt();
    SaveConfigToNVS(config);
    request->send(200, "text/html", "Lamp alwayse on is set to: " + String(config.LampAlwayseOn));
    xSemaphoreGive(xMutexConfig);
}

void HandleGetConfigValues(AsyncWebServerRequest *request)
{
    if (!LoadConfigFromNVC(config))
    {
        request->send(500, "text/html", "Unable to load config from NVS");
        return;
    }
    std::unique_ptr<char[]> configString;
    PrepareConfigString(config, configString);
    request->send(200, "text/html", configString.get());
}

void HandleChangeWiFiMode(AsyncWebServerRequest *request)
{
    static constexpr const char *urlExample = "http://x.x.x.x:180/changewifimode?mode=WIFI_AP";
    if (request->args() != 1)
    {
        request->send(400, "text/html", "server.args() != 1 Set parameter only WIFI_STA or WIFI_AP with example: " + String(urlExample));
        return;
    }
    const String &mode = request->arg("mode");
    if (mode != "WIFI_STA" && mode != "WIFI_AP")
    {
        request->send(400, "text/html", "Set parameter only WIFI_STA or WIFI_AP with example: " + String(urlExample));
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        request->send(500, "text/html", "Unable to load config from NVS");
        return;
    }

    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    strncpy(config.WiFiMode, mode.c_str(), sizeof(config.WiFiMode) - 1);
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
    request->send(200, "text/html", "Wifi mode is set to: " + String(config.WiFiMode));
    delay(5000);
    ESP.restart();
}

void HandleRebootDevice(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", "Restart after 5 seconds");
    delay(5000);
    ESP.restart();
}