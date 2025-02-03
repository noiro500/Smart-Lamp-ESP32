#include "ServerHandlers.h"
#include "ConfigManager.h"
#include "SensorMeasurements.h"

// AM2320 am2320(&Wire);
// ESP32Time rtc;
ConfigValues config;

void HandleRoot(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", GREETINGS);
}

void HandleFind(AsyncWebServerRequest *request)
{
    Json result;
    result.add("IP", WiFi.localIP().toString());
    result.add("Port", String(WEBSERVER_PORT));
    result.add("DeviceName", ("ESP32_" + WiFi.macAddress()));
    request->send(200, "application/json", result.toString());
    result.clear();
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
    const char *urlExample = "http://x.x.x.x:180/settime?hour=0&min=0&sec=0&day=1&month=1&year=2022";

    if (WiFi.getMode() == WIFI_AP)
    {
        if (request->args() != 6)
        {
            const char *partMessage = "Set time with example: ";
            std::unique_ptr<char[]> message = std::make_unique<char[]>(strlen(partMessage) + 1 + strlen(urlExample) + 1);
            strcpy(message.get(), partMessage);
            strcat(message.get(), urlExample);
            request->send(400, "text/plain", message.get());
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
    request->send(200, "text/plain", "Time is set to: " + rtc.getDateTime(true));
}

void HandleGetTime(AsyncWebServerRequest *request)
{
    request->send(200, "text/plain", rtc.getDateTime(true));
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
        hoursJsonArray.push((int32_t)temperatureInHours.hour[i]);
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
    if (request->args() != 2)
    {
        const char *urlExample = "http://x.x.x.x:180/setwifistaparam?ssid=MyWiFi&password=12345678";
        const char *partMessage = "Set WiFi STA parameters with example: ";
        std::unique_ptr<char[]> message = std::make_unique<char[]>(strlen(partMessage) + 1 + strlen(urlExample) + 1);
        strcpy(message.get(), partMessage);
        strcat(message.get(), urlExample);
        request->send(400, "text/plain", message.get());
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        request->send(500, "Unable to load config from NVS");
        return;
    }
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    strlcpy(config.WiFiSsid, request->arg("ssid").c_str(), sizeof(config.WiFiSsid));
    strlcpy(config.WiFiPassword, request->arg("password").c_str(), sizeof(config.WiFiPassword));
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
    const char *partMessage = "WiFi STA parameters are set: ";
    std::unique_ptr<char[]> message = std::make_unique<char[]>(strlen(partMessage) + 1 + strlen(config.WiFiSsid) + 1 + strlen(", ") + 1 + strlen(config.WiFiPassword) + 1);
    strcpy(message.get(), partMessage);
    strcat(message.get(), config.WiFiSsid);
    strcat(message.get(), ", ");
    strcat(message.get(), config.WiFiPassword);
    request->send(200, "text/plain", message.get());
    delay(5000);
    ESP.restart();
}

void HandleSetLampTime(AsyncWebServerRequest *request)
{
    if (request->args() != 2)
    {
        const char *urlExample = "http://x.x.x.x:180/setlamptime?on=8&off=22";
        const char *partMessage = "Set on/off time with example: ";
        std::unique_ptr<char[]> message = std::make_unique<char[]>(strlen(partMessage) + 1 + strlen(urlExample) + 1);
        strcpy(message.get(), partMessage);
        strcat(message.get(), urlExample);
        request->send(400, message.get());
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        request->send(500, "text/plain", "Unable to load config from NVS");
        return;
    }
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    config.LampOnTimeHours = request->arg("on").toInt();
    config.LampOffTimeHours = request->arg("off").toInt();
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
    const char *partMessage = "Lamp time is set to: ";
    char LampOnTimeHours[3];
    char LampOffTimeHours[3];
    itoa(config.LampOnTimeHours, LampOnTimeHours, 10);
    itoa(config.LampOffTimeHours, LampOffTimeHours, 10);
    std::unique_ptr<char[]> message = std::make_unique<char[]>(strlen(partMessage) + 1 + strlen("On: ") + 1 + strlen(LampOnTimeHours)+1 + strlen(" hours\n") + 1 + strlen("Off: ") + 1 + strlen(LampOffTimeHours)+1 + strlen(" hours\n")+1);
    strcpy(message.get(), partMessage);
    strcat(message.get(), "On: ");
    strcat(message.get(), LampOnTimeHours);
    strcat(message.get(), " hours\n");
    strcat(message.get(), "Off: ");
    strcat(message.get(), LampOffTimeHours);
    strcat(message.get(), " hours\n");
    request->send(200, "text/plain", message.get());
    return;
}

void HandleLampOlwayseOn(AsyncWebServerRequest *request)
{

    if (request->args() != 1)
    {
        const char *urlExample = "http://x.x.x.x:180/lampalwayson?on=0";
        const char *partMessage = "Set parameter with example: ";
        std::unique_ptr<char[]> message = std::make_unique<char[]>(strlen(partMessage) + 1 + strlen(urlExample) + 1);
        strcpy(message.get(), partMessage);
        strcat(message.get(), urlExample);
        request->send(400, "text/plain", message.get());
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        request->send(500, "text/plain", "Unable to load config from NVS");
        return;
    }
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    config.LampAlwayseOn = request->arg("on").toInt();
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
    const char *partMessage = "Lamp alwayse on is set to: ";
    char LampAlwayseOn[3];
    itoa(config.LampAlwayseOn, LampAlwayseOn, 10);
    std::unique_ptr<char[]> message = std::make_unique<char[]>(strlen(partMessage) + 1 + strlen(LampAlwayseOn) + 1);
    strcpy(message.get(), partMessage);
    strcat(message.get(), LampAlwayseOn);
    request->send(200, "text/plain", message.get());
}

void HandleGetConfigValues(AsyncWebServerRequest *request)
{
    if (!LoadConfigFromNVC(config))
    {
        request->send(500, "text/plain", "Unable to load config from NVS");
        return;
    }
    std::unique_ptr<char[]> configString;
    PrepareConfigString(config, configString);
    request->send(200, "text/plain", configString.get());
}

void HandleChangeWiFiMode(AsyncWebServerRequest *request)
{
    const char *urlExample = "http://x.x.x.x:180/changewifimode?mode=WIFI_AP";
    if (request->args() != 1)
    {
        const char *partMessage = "server.args() != 1 Set parameter only WIFI_STA or WIFI_AP with example: ";
        std::unique_ptr<char[]> message = std::make_unique<char[]>(strlen(partMessage) + 1 + strlen(urlExample) + 1);
        strcpy(message.get(), partMessage);
        strcat(message.get(), urlExample);  
        request->send(400, "text/plain", message.get());
        return;
    }
    const String &mode = request->arg("mode");
    if (mode != "WIFI_STA" && mode != "WIFI_AP")
    {
        const char *partMessage = "Set parameter only WIFI_STA or WIFI_AP with example: ";
        std::unique_ptr<char[]> message = std::make_unique<char[]>(strlen(partMessage) + 1 + strlen(urlExample) + 1);
        strcpy(message.get(), partMessage);
        strcat(message.get(), urlExample);
        request->send(400, "text/plain", message.get());
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        request->send(500, "text/plain", "Unable to load config from NVS");
        return;
    }

    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    strlcpy(config.WiFiMode, mode.c_str(), sizeof(config.WiFiMode));
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
    const char *partMessage = "Wifi mode is set to: ";
    std::unique_ptr<char[]> message = std::make_unique<char[]>(strlen(partMessage) + 1 + strlen(config.WiFiMode) + 1);
    strcpy(message.get(), partMessage);
    strcat(message.get(), config.WiFiMode);
    request->send(200, "text/plain", message.get());
    delay(5000);
    ESP.restart();
}

void HandleChangePassword(AsyncWebServerRequest *request)
{
    if (!request->hasParam("body", true))
    {
        request->send(400, "text/plain", "Bad Request: Body is missing");
        return;
    }
    if (!LoadConfigFromNVC(config))
    {
        request->send(500, "text/plain", "Unable to load config from NVS");
        return;
    }
    String body = request->getParam("body", true)->value();
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    strlcpy(config.WiFiPassword, body.c_str(), sizeof(config.WiFiPassword));
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
}

void HandleChangeConfigValues(AsyncWebServerRequest *request)
{
    if (!request->hasParam("body", true))
    {
        request->send(400, "text/plain", "Bad Request: Body is missing");
        return;
    }
    String body = request->getParam("body", true)->value();
    int lampAlwayseOn, lampOnTimeHours, lampOffTimeHours;
    char wifiMode[10], wifiPassword[64], softApSsid[32], wifiSsid[32];
    uint8_t softApIP[4], softIpSubnetMask[4];
    sscanf(body.c_str(),
           "WiFiMode=%9s&SoftApIP=%hhu.%hhu.%hhu.%hhu&SoftIpSubnetMask=%hhu.%hhu.%hhu.%hhu&WiFiPassword=%63s&SoftApSsid=%31s&WiFiSsid=%31s&LampAlwayseOn=%d&LampOnTimeHours=%d&LampOffTimeHours=%d",
           wifiMode,
           &softApIP[0], &softApIP[1], &softApIP[2], &softApIP[3],
           &softIpSubnetMask[0], &softIpSubnetMask[1], &softIpSubnetMask[2], &softIpSubnetMask[3],
           wifiPassword, softApSsid, wifiSsid,
           &lampAlwayseOn, &lampOnTimeHours, &lampOffTimeHours);

    strlcpy(config.WiFiMode, wifiMode, sizeof(config.WiFiMode));
    memcpy(config.SoftApIP, softApIP, sizeof(config.SoftApIP));
    memcpy(config.SoftIpSubnetMask, softIpSubnetMask, sizeof(config.SoftIpSubnetMask));
    strlcpy(config.WiFiPassword, wifiPassword, sizeof(config.WiFiPassword));
    strlcpy(config.SoftApSsid, softApSsid, sizeof(config.SoftApSsid));
    strlcpy(config.WiFiSsid, wifiSsid, sizeof(config.WiFiSsid));
    config.LampAlwayseOn = lampAlwayseOn;
    config.LampOnTimeHours = lampOnTimeHours;
    config.LampOffTimeHours = lampOffTimeHours;
    xSemaphoreTake(xMutexConfig, portMAX_DELAY);
    SaveConfigToNVS(config);
    xSemaphoreGive(xMutexConfig);
    request->send(200, "text/plain", "Configuration updated successfully");
}

void HandleRebootDevice(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", "Restart after 5 seconds");
    delay(5000);
    ESP.restart();
}