#include "WorkTasks.h"
#include "ConfigManager.h"

void HandleRoot(AsyncWebServerRequest*);
void HandleFind(AsyncWebServerRequest*);
void HandleGetTempAndHum(AsyncWebServerRequest*);
void HandleSetTime(AsyncWebServerRequest*);
void HandleGetTime(AsyncWebServerRequest*);
void HandleTemperatureInHours(AsyncWebServerRequest*);
void HandleSetWiFiSTAParam(AsyncWebServerRequest*);
void HandleSetLampTime(AsyncWebServerRequest*);
void HandleLampOlwayseOn(AsyncWebServerRequest*);
void HandleGetConfigValues(AsyncWebServerRequest*);
void HandleChangeWiFiMode(AsyncWebServerRequest*);
void HandleChangePassword(AsyncWebServerRequest*);
void HandleChangeConfigValues(AsyncWebServerRequest*);
void HandleRebootDevice(AsyncWebServerRequest*);