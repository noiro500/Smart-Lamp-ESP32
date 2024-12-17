#include "WorkTasks.h"
#include "ConfigManager.h"

void HandleRoot(PsychicRequest*);
void HandleGetTempAndHum(PsychicRequest*);
void HandleSetTime(PsychicRequest*);
void HandleGetTime(PsychicRequest*);
void HandleTemperatureInHours(PsychicRequest*);
void HandleSetWiFiSTAParam(PsychicRequest*);
void HandleSetLampTime(PsychicRequest*);
void HandleLampOlwayseOn(PsychicRequest*);
void HandleGetConfigValues(PsychicRequest*);
void HandleChangeWiFiMode(PsychicRequest*);
void HandleRebootDevice(PsychicRequest*);