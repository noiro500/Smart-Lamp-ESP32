#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include "AM2320.h"
#include "ESP32Time.h"
#include <memory>

extern SemaphoreHandle_t xMutexConfig;
extern SemaphoreHandle_t xMutexSensor;
extern TaskHandle_t lampTaskHandle;
extern TaskHandle_t wifiGuardTaskHandle;
extern TaskHandle_t temperatureInHoursTaskHandle;
extern int lampPin;
extern AM2320 am2320;
extern ESP32Time rtc;
extern Preferences preferences;

// Структура конфигурации
//Structure of Configuration
typedef struct
{
    char WifiMode[9];
    uint8_t SoftApIP[4];
    uint8_t SoftIpSubnetMask[4];
    char WiFiPassword[64];
    char SoftApSsid[32];
    char WifiSsid[32];
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
std::unique_ptr<float[]> GetMeasurementsFromSensor();
void PrepareConfigString(const ConfigValues &, std::unique_ptr<char[]> &);
#endif // CONFIG_MANAGER_H