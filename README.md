# SmartLampESP32 Controller

The SmartLampESP32 electronic device controller is a unit capable of performing the following actions:

*   Connect to a Wi-Fi network in WiFi\_STA mode (the lamp connects to an access point).
*   Function as a Wi-Fi AP (the lamp itself acts as an access point).
*   Turn electrical devices on and off by command.
*   Turn electrical devices on and off using a timer.
*   Measure temperature and humidity.
*   Measure and record temperature and humidity over a 24-hour period.
*   Obtain time from the NTP time server.
*   Automatically reconnect to the Wi-Fi access point if the connection is lost.

Currently, the controller is used for controlling a grow light.

## Preparation for Use

The project uses:

*   The ESP32 WeMos LOLIN32 Lite board.
*   The Aosong AM2320 digital temperature and humidity sensor.
*   The relay module for switching 110/220V loads.

Connect the AM2320 sensor to the corresponding pins of the microcontroller board. Since the ESP32 WeMos LOLIN32 Lite board is being used, these are the following pins: SDA - 19, SCL - 23 (for other ESP32 boards, the pins may differ). Connect power to the AM2320. Connect the relay module to the appropriate pins (by default, In or Sig to pin 23, power pins VCC and GND).

**Note:** If the AM2320 sensor does not transmit readings, it likely lacks resistors on the SDA and SCL outputs. You need to connect a 10K Ohm resistor in parallel between the SDA and SCL outputs of the AM2320 and the positive terminal of the VCC power supply.

![ESP32 AM2320](Image//ConnectAM2320.png)

### Firmware Configuration

Open the file `GeneralParameters.h` and configure the default parameters:

```cpp
#define WIFI_MODE "WIFI_STA"                        // ESP32 WiFi operating mode (WIFI_AP, WIFI_STA)
#define WIFI_AP_IP {192, 168, 4, 1}                 // WIFI_AP IP
#define WIFI_AP_NETMASK {255, 255, 255, 0}          // WIFI_AP subnet mask
#define PASSWORD "123456789"                        // Password for WIFI_AP and WIFI_STA
#define WIFI_AP_SSID "CactusLampESP32"              // SSID for WIFI_AP
#define WIFI_STA_SSID "Wi_Fi_Station"               // SSID for WIFI_STA
#define LAMP_ALWAYS_ON 0                            // Is the lamp always on? (0 - no, 1 - yes)
#define LAMP_ON_HOUR 8                              // Hour to turn the lamp on (0 to 23)
#define LAMP_OFF_HOUR 22                            // Hour to turn the lamp off (0 to 23)

#define TIMEZONE 3                                  // Time zone (e.g., UTC 0), hours
#define DAYLIGHT_OFFSET 0 * 60 * 60                 // Daylight saving offset (0 means no daylight saving)
#define NTP_SERVER "ntp0.ntp-servers.net"           // Time server
#define SDA_PIN 19                                  // SDA pin
#define SCL_PIN 23                                  // SCL pin
#define RELAY_LAMP_PIN 22                           // Lamp relay connection pin
#define WEBSERVER_PORT 80                           // Webserver port
#define GREETINGS "<h1>ESP32 Cactus Lamp</h1>"      // Message when accessing http://<ESP32_IP>:WEBSERVER_PORT/
```

Upload the code to your ESP32 board.

### Usage

When initially flashed, the default values set in `GeneralParameters.h` are used. By default, the ESP32 attempts to connect to the access point with the SSID `WIFI_STA_SSID` and the password `PASSWORD`. **Note:** If you need to operate in access point mode, change `WIFI_MODE` to `WIFI_AP`. The access point's IP address will be `WIFI_AP_IP`, and the password will be `PASSWORD`.

After uploading, a web server starts on port `WEBSERVER_PORT`, which accepts the following GET requests:

| Request | Result | Parameters |
| --- | --- | --- |
| http://\<ESP32\_IP>:WEBSERVER\_PORT/ | Displays a greeting string | None |
| http://\<ESP32\_IP>:WEBSERVER\_PORT/gettempandhum | Retrieves average temperature and humidity | None |
| http://\<ESP32\_IP>:WEBSERVER\_PORT/settime?hour=0&min=0&sec=0&day=1&month=1&year=2022 | Sets the RTC time of ESP32 | If ESP32 is in WIFI\_STA mode, the time is updated from the NTP server. If in WIFI\_AP mode, specify hours, minutes, seconds, day, month, year in the request parameters |
| http://\<ESP32\_IP>:WEBSERVER\_PORT/gettime | Gets the RTC time of ESP32 | None |
| http://\<ESP32\_IP>:WEBSERVER\_PORT/data | Outputs time, temperature, and humidity starting from midnight in JSON format | None |
| http://\<ESP32\_IP>:WEBSERVER\_PORT/setwifistaparam?ssid=MyWiFi&password=12345678 | Sets the SSID for WIFI\_STA mode and the password for both WIFI\_STA and WIFI\_AP modes | ssid - access point name, password - password |
| http://\<ESP32\_IP>:WEBSERVER\_PORT/setlamptime?on=8&off=22 | Sets the on/off times (in hours) for the device connected to the relay | on - hour to turn on, off - hour to turn off |
| http://\<ESP32\_IP>:WEBSERVER\_PORT/lampalwayson?on=0 | Checks if the device connected to the relay is constantly on | on=1 - allow constant operation, on=0 - disallow constant operation |
| http://\<ESP32\_IP>:WEBSERVER\_PORT/getconfigvalues | Outputs the current configuration | None |
| http://\<ESP32\_IP>:WEBSERVER\_PORT/changewifimode?mode=WIFI\_AP | Switches the Wi-Fi operating mode of ESP32 | mode - either WIFI\_STA or WIFI\_AP |
| http://\<ESP32\_IP>:WEBSERVER\_PORT/rebootdevice | Reboots ESP32 | None |


# Контроллер лампы SmartLampESP32

Контроллер электронных устройств SmartLampESP32 представляет из себя устройство, которое может выполнять следующие действия:

*   Подключаться к сети WiFi в режиме WiFi\_STA (лампа подключается у точке доступа).
*   Выступать в режиме WiFi\_AP (лампа сама выступает в роли точки доступа).
*   Включать и выключать электрические приботы по команде.
*   Включать и выключать алектрические приботы по таймеру.
*   Измерять температуру и влажность.
*   Измерять и сохранять температуру и влажность в течении суток.
*   Получать время с сервера времени ntp.
*   Автоматически переподключаться к точке доступа WfFi при потере соединения.

В настоящий момент контроллер используется для контроля фитолампы.

## Подготовка к использованию

В проекте используются:

*   Плата ESP32 WeMos LOLIN32 Lite.
*   Цифровой датчик температуры и влажности Aosong AM2320.
*   Модуль реле для коммутации нагрузки 110/220 вольт

Подключить датчик AM2320 к соответствующим пинам платы микроконтроллера. Так как используется плата ESP32 WeMos LOLIN32 Lite, то это следующие пины: SDA - 19, SCL - 23 (для других плат ESP32 пины могут отличаться). Подключить питание к AM2320. Подключить модуль реле к соответствующи пинам (по умолчанию In или Sig к пину 23, пины питания VCC и GND)  

**Внимание:** если датчик AM2320 не передает показания, то, вероятно, в нем отсутствуют резисторы на выводах SDA и SCL.  Необходимо подключить резистор 10КОм параллельно выходу SDA и SCA AM2320 и плюсом питания VCC.

![ESP32 AM2320](Image//ConnectAM2320.png)

### Настройка прошивки

Откройте файл GeneralParameters.h и настройте параметры конфигурации по умолчанию:

```cpp
#define WIFI_MODE "WIFI_STA"                        // Режим работы ESP32 WiFi (WIFI_AP, WIFI_STA)
#define WIFI_AP_IP {192, 168, 4, 1}                 // WIFI_AP IP
#define WIFI_AP_NETMASK {255, 255, 255, 0}          // WIFI_AP Маска подсети
#define PASSWORD "123456789"                        // Пароль WIFI_AP and WIFI_STA
#define WIFI_AP_SSID "CactusLampESP32"              // Ssid for WIFI_AP
#define WIFI_STA_SSID "Wi_Fi_Station"               // Ssid for WIFI_STA
#define LAMP_OLWAYS_ON 0                            // Лампа всегда включена? (0 - нет, 1 - да)
#define LAMP_ON_HOUR 8                              // Час включения лампы (0 до 23)
#define LAMP_OFF_HOUR 22                            // Час выключения лампы (0 до 23) 

#define TIMEZONE 3                                  // Временная зона (например UTC 0), hour
#define DAYLIGHTOFFSET 0 * 60 * 60                  // Смещение летнего времени (0 - нет перехода на летнее время)
#define NTP_SERVER "ntp0.ntp-servers.net"           //Сервер времени
#define SDA_PIN 19                                  //SDA пин
#define SCL_PIN 23                                  //SCL пин                            
#define RELAY_LAMP_PIN 22                           //Lamp relay connection pin
#define WEBSERVER_PORT 180                          //Порт Веб-сервера (Web Server)
#define GREETINGS "<h1>ESP32 Cactus Lamp</h1>"      //Сообщение при обращении к http://<IP_ESP32>:WEBSERVER_PORT/
```

Загрузите код в плату ESP32

### Использование

При первоначальный прошивке используются значения по умолчанию, установленные в файле GeneralParameters.h . По умолчанию ESP32 пытается подключиться к точке доступа с ssid WIFI\_STA\_SSID и паролем PASSWORD.  **Внимание: е**сли необходимо подключиться в режиме точки доступа, то необходимо изменить  WIFI\_MODE на WIFI\_AP. IP адрес точки доступа будет WIFI\_AP\_IP и пароль PASSWORD .

После загрузки запускается веб-сервер на порту WEBSERVER\_PORT, который принимает следующие GET запросы :

| Запрос | Результат выполнения запроса | Параметры |
| --- | --- | --- |
| http://\<ESP32\_IP>:`WEBSERVER_PORT/` | Вывод приветственной строки | Отсутствуют |
| http://\<ESP32\_IP>:`WEBSERVER_PORT/gettempandhum` | Получение усредненной температуры и влажности | Отсутствуют |
| http://\<ESP32\_IP>:`WEBSERVER_PORT`/settime?hour=0&min=0&sec=0&day=1&month=1&year=2022" | Установка времени RTC ESP32 | Если ESP32 в режиме WIFI\_STA, то  время обновляется с сервера ntp. Если ESP32 в режиме WIFI\_AP, то в параметрах запроса необходимо ввести часы, минуты, секунды, день, месяц, год |
| http://\<ESP32\_IP>:`WEBSERVER_PORT`/gettime | Получение времени RTC ESP32 | Отсутствуют |
| http://\<ESP32\_IP>:`WEBSERVER_PORT`/data | Выводи в формате JSON время, температуру, влажность, начиная с 0 часов | Отсутствуют |
| http://\<ESP32\_IP>:`WEBSERVER_PORT`/setwifistaparam?ssid=MyWiFi&password=12345678 | Установка ssid для режима WIFI\_STA и пароля для режима WIFI\_STA и WIFI\_AP | ssid - имя точки доступа, password - пароль |
| http://\<ESP32\_IP>:`WEBSERVER_PORT`/setlamptime?on=8&off=22 | Установка времени (в часах) включения и выключения подключенного к реле устройства | on - час включения, off - час отключения |
| http://\<ESP32\_IP>:`WEBSERVER_PORT`/lampalwayson?on=0 | Включено ли подключенное к реле устройство постоянно | on=1 - разрешить работать постоянно постоянно, on=0 - запретить работать постоянно |
| http://\<ESP32\_IP>:`WEBSERVER_PORT/getconfigvalues` | Вывод текущей конфигурации | Отсутствуют |
| http://\<ESP32\_IP>:`WEBSERVER_PORT/`changewifimode?mode=WIFI\_AP | Смена режима работы WIFI ESP32 | mode - либо WIFI\_STA, либо WIFI\_AP |
| http://\<ESP32\_IP>:`WEBSERVER_PORT/rebootdevice` | Перезагрузка ESP32 | Отсутствуют |
