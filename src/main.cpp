#include <Arduino.h>
#include "ConfigManager.h"
#include "WiFi_AP.h"
#include "ServerHandlers.h"
#include "WorkTasks.h"

/*Default value structure */
ConfigValues defaultValues = {
    "WIFI_STA",         //WiFi Mode (WIFI_AP, WIFI_STA)
    {192, 168, 4, 1},   //WIFI_AP IP
    {255, 255, 255, 0}, //WIFI_AP Netmask
    "123456789",        //Password
    "CactusLampESP32",  //Ssid for WIFI_AP
    "Wi_Fi_Station",    //Ssid for WIFI_STA
    0,                  //Lamp always on? (0 - yes, 1 - no)  
    8,                  //Lamp on hour  
    22};                //Lamp off hour

ConfigValues loadConfig;

void setup()
{
  Serial.begin(115200);
  if (!LoadConfigFromNVC(loadConfig))
  {
    SaveConfigToNVS(defaultValues);
    Serial.println("Save Default config in EEPROM. Restart");
    delay(1000);
    ESP.restart();
  }

  ConfigWiFi(loadConfig);

  server.on("/", HandleRoot);
  server.on("/gettempandhum", HandleGetTempAndHum);
  server.on("/settime", HandleSetRtc);
  server.on("/gettime", HandleGetRtc);
  server.on("/data", HandleTemperatureInHours);
  server.on("/setwifistaparam", HandleSetWiFiSTAParam);
  server.on("/setlamptime", HandleSetLampTime);
  server.on("/lampalwayson", HandleLampOlwayseOn);
  server.on("/getconfigvalues", HandleGetConfigValues);
  server.on("/changewifimode", HandleChangeWiFiMode);
  server.on("/rebootdevice", HandleRebootDevice);

  server.begin();

  xTaskCreatePinnedToCore(LampTask, "LampTask", 4096, NULL, 1, &lampTaskHandle, tskNO_AFFINITY);
  if (lampTaskHandle == NULL)
    Serial.println("Error creating LampTask");
  xTaskCreatePinnedToCore(TemperatureInHoursTask, "TemperatureInHoursTask", 4096, &temperatureInHours, 1, &temperatureInHoursTaskHandle, tskNO_AFFINITY);
  if (temperatureInHoursTaskHandle == NULL)
    Serial.println("Error creating TemperatureInHoursTask");
}

void loop()
{
  server.handleClient();
  if (strcmp(loadConfig.WifiMode, "WIFI_AP") == 0)
    dnsServer.processNextRequest();
}