#include "WorkTasks.h"
#include "ConfigManager.h"

void HandleRoot(AsyncWebServerRequest*);
void HandleGetTempAndHum();
void HandleSetTime();
void HandleGetTime();
void HandleTemperatureInHours();
void HandleSetWiFiSTAParam();
void HandleSetLampTime();
void HandleLampOlwayseOn();
void HandleGetConfigValues();
void HandleChangeWiFiMode();
void HandleRebootDevice();