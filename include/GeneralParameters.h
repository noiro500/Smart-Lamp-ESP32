#pragma once
#define IS_TEST_MODE 0                              //Enable test mode? (0 - no, 1 - yes)

/*Default value structure ConfigValues defaultValues*/
#define WIFI_MODE "WIFI_STA"                        // WiFi Mode (WIFI_AP, WIFI_STA)
#define WIFI_AP_IP {192, 168, 4, 1}                 // WIFI_AP IP
#define WIFI_AP_NETMASK {255, 255, 255, 0}          // WIFI_AP Netmask
#define PASSWORD "11111111"                        // Password for WIFI_AP and WIFI_STA. Max 63 symbols
#define WIFI_AP_SSID "CactusLampESP32"              // Ssid for WIFI_AP. Max 31 symbols
#define WIFI_STA_SSID "Wi_Fi_Station"               // Ssid for WIFI_STA. Max 31 symbols
#define LAMP_OLWAYS_ON 0                            // Lamp always on? (0 - no, 1 - yes)
#define LAMP_ON_HOUR 8                              // Lamp on hour (0 - 23)
#define LAMP_OFF_HOUR 22                            // Lamp off hour (0 - 23)

/**************************************************/

#define TIMEZONE 3                                  //TimeZone, hour
#define DAYLIGHTOFFSET 0 * 60 * 60                  // Offset for summer/winter time transition in seconds
#define NTP_SERVER "ntp0.ntp-servers.net"
#define SDA_PIN 19
#define SCL_PIN 23
#define RELAY_LAMP_PIN 22                           //Lamp relay connection pin
#define WEBSERVER_PORT 180                          //Webserver port
#define GREETINGS "<h1>ESP32 Cactus Lamp</h1>"      //Return message 
