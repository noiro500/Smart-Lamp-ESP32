#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include "AM2320.h"
#include "ESP32Time.h"
#include "GeneralParameters.h"
#include "PsychicHttp.h"
#include <DNSServer.h>
#include "Json.h"
#include <memory>

extern SemaphoreHandle_t xMutexConfig;
extern SemaphoreHandle_t xMutexSensor;
extern TaskHandle_t lampTaskHandle;
extern TaskHandle_t wifiGuardTaskHandle;
extern TaskHandle_t temperatureInHoursTaskHandle;
extern int RelaylampPin;
extern ESP32Time rtc;
extern Preferences preferences;
extern DNSServer dnsServer;
extern PsychicHttpServer server;

//extern char *tempAndHumcachedResponse;
extern std::unique_ptr<char[]> tempAndHumcachedResponse;
/*extern float latestTemperature;
extern float latestHumidity;*/
// Структура конфигурации
//Structure of Configuration
typedef struct
{
    char WiFiMode[10];
    uint8_t SoftApIP[4];
    uint8_t SoftIpSubnetMask[4];
    char WiFiPassword[64];
    char SoftApSsid[32];
    char WiFiSsid[32];
    int LampAlwayseOn;
    int LampOnTimeHours;
    int LampOffTimeHours;
} ConfigValues;

typedef struct
{
    int hour[24];
    float temperature[24];
    float humidity[24];
} TempAndHumInHours;

extern TempAndHumInHours temperatureInHours;

void SaveConfigToNVS(ConfigValues &config);
bool LoadConfigFromNVC(ConfigValues &);
void ConfigWiFi(ConfigValues &);
void PrepareConfigString(const ConfigValues &, std::unique_ptr<char[]> &);
#endif // CONFIG_MANAGER_H