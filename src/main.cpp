#include <Arduino.h>
#include "ConfigManager.h"
// #include "WebAndDnsServers"
#include "ServerHandlers.h"
#include "WorkTasks.h"

/*Default value structure */
ConfigValues defaultValues = {
    WIFI_MODE,       // WiFi Mode (WIFI_AP, WIFI_STA)
    WIFI_AP_IP,      // WIFI_AP IP
    WIFI_AP_NETMASK, // WIFI_AP Netmask
    PASSWORD,        // Password
    WIFI_AP_SSID,    // Ssid for WIFI_AP
    WIFI_STA_SSID,   // Ssid for WIFI_STA
    LAMP_OLWAYS_ON,  // Lamp always on? (0 - yes, 1 - no)
    LAMP_ON_HOUR,    // Lamp on hour
    LAMP_OFF_HOUR}; // Lamp off hour

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
  server.on("/find", HandleFind);
  server.on("/gettempandhum", HandleGetTempAndHum);
  server.on("/settime", HandleSetTime);
  server.on("/gettime", HandleGetTime);
  server.on("/data", HandleTemperatureInHours);
  server.on("/setwifistaparam", HandleSetWiFiSTAParam);
  server.on("/setlamptime", HandleSetLampTime);
  server.on("/lampalwayson", HandleLampOlwayseOn);
  server.on("/getconfigvalues", HandleGetConfigValues);
  server.on("/changewifimode", HandleChangeWiFiMode);
  server.on("/changepassword", HTTP_PUT, HandleChangePassword);
  server.on("/changeconfigvalues", HTTP_PUT, HandleChangeConfigValues);
  server.on("/rebootdevice", HandleRebootDevice);

  server.begin();

  xTaskCreatePinnedToCore(LampTask, "LampTask", 4096, NULL, 1, &lampTaskHandle, tskNO_AFFINITY);
  if (lampTaskHandle == NULL)
    Serial.println("Error creating LampTask");

  xTaskCreatePinnedToCore(TemperatureInHoursTask, "TemperatureInHoursTask", 4096, &temperatureInHours, 1, &temperatureInHoursTaskHandle, tskNO_AFFINITY);
  if (temperatureInHoursTaskHandle == NULL)
    Serial.println("Error creating TemperatureInHoursTask");

  xTaskCreatePinnedToCore(TempAndHumCacheUpdateTask, "TempAndHumCacheUpdateTask", 4096, NULL, 1, NULL, tskNO_AFFINITY);
}

void loop()
{
  // server.handleClient();
  if (strcmp(loadConfig.WiFiMode, "WIFI_AP") == 0)
    dnsServer.processNextRequest();
}