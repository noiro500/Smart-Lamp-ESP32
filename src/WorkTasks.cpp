#include "WorkTasks.h"
#include "ConfigManager.h"
#include "ESP32Time.h"
#include "SensorMeasurements.h"

SemaphoreHandle_t xMutexConfig = xSemaphoreCreateMutex();
SemaphoreHandle_t xMutexSensor = xSemaphoreCreateMutex();
TaskHandle_t lampTaskHandle;
TaskHandle_t wifiGuardTaskHandle;
TaskHandle_t temperatureInHoursTaskHandle;
int RelaylampPin = RELAY_LAMP_PIN;
TempAndHumInHours temperatureInHours;
std::unique_ptr<char[]> tempAndHumcachedResponse;
//char *tempAndHumcachedResponse;


void LampTask(void *pvParameters)
{
    pinMode(RelaylampPin, OUTPUT);
    ConfigValues config;

    while (true)
    {
        xSemaphoreTake(xMutexConfig, portMAX_DELAY);
        if (!LoadConfigFromNVC(config))
        {
            xSemaphoreGive(xMutexConfig);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        xSemaphoreGive(xMutexConfig);

        if (config.LampAlwayseOn == 1)
        {
            digitalWrite(RelaylampPin, HIGH);
            // Serial.println("Lamp always on");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        int hour = rtc.getHour(true);
        if (hour < config.LampOnTimeHours || hour >= config.LampOffTimeHours)
        {
            digitalWrite(RelaylampPin, LOW);
            // Serial.println("Lamp off");
        }
        else
        {
            digitalWrite(RelaylampPin, HIGH);
            // Serial.println("Lamp on");
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void TemperatureInHoursTask(void *pvParameters)
{
    TempAndHumInHours *tempAndHumInHours = (TempAndHumInHours *)pvParameters;
    ESP32Time rtc;
    int lastUpdateHour = -1;
    /*Getting the initial time for vTaskDelayUntil*/
    /*Получение начального времени для vTaskDelayUntil*/
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (true)
    {
        int currentHour = rtc.getHour(true);
        // xSemaphoreTake(xMutexConfig, portMAX_DELAY);
        if (currentHour != lastUpdateHour)
        {
            if (currentHour == 0)
            /*Clear the temperatureInHours structure and update the time from the server*/
            /*Очистить структуру temperatureInHours и обновить время с сервера*/
            {
                memset(tempAndHumInHours->hour, 0, sizeof(tempAndHumInHours->hour));
                memset(tempAndHumInHours->temperature, 0, sizeof(tempAndHumInHours->temperature));
                memset(tempAndHumInHours->humidity, 0, sizeof(tempAndHumInHours->humidity));
                if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED)
                {
                    configTime(TIMEZONE * 3600, DAYLIGHTOFFSET, NTP_SERVER);
                    struct tm timeinfo;
                    if (getLocalTime(&timeinfo))
                    {
                        rtc.setTimeStruct(timeinfo);
                    }
                }
            }
            xSemaphoreTake(xMutexSensor, portMAX_DELAY);
            auto measurements = GetMeasurementsFromSensor();
            tempAndHumInHours->hour[currentHour] = currentHour;
            tempAndHumInHours->temperature[currentHour] = measurements[0];
            tempAndHumInHours->humidity[currentHour] = measurements[1];
            xSemaphoreGive(xMutexSensor);
            lastUpdateHour = currentHour;
        }
        // xSemaphoreGive(xMutexConfig);
        /*Calculate the delay until the beginning of the next hour*/
        /*Рассчитать задержку до начала следующего часа*/
        int currentMinute = rtc.getMinute();
        int currentSecond = rtc.getSecond();
        /*Seconds until the next hour*/
        /*Секунды до следующего часа*/
        int delaySeconds = (60 - currentMinute) * 60 - currentSecond;
        // Serial.printf("%s\n", (rtc.getDateTime(true)).c_str());
        TickType_t delayTicks = delaySeconds * 1000 / portTICK_PERIOD_MS;
        /*Waiting until the beginning of the next hour*/
        /*Ожидание до начала следующего часа*/
        vTaskDelayUntil(&xLastWakeTime, delayTicks);
    }
}

void TempAndHumCacheUpdateTask(void *pvParameters)
{
    const float epsilon = 0.00001f;
    while (true)
    {
        tempAndHumcachedResponse = std::make_unique<char[]>(50); 
        xSemaphoreTake(xMutexSensor, portMAX_DELAY);
        auto measurements = GetMeasurementsFromSensor();
        if (fabsf(measurements[0] - (-50.00f)) <= epsilon && fabsf(measurements[1] - (-60.00f)) <= epsilon)
             strcpy(tempAndHumcachedResponse.get(), "Error reading AM2320 sensor");
        else if (fabsf(measurements[0] - (-50.00f)) <= epsilon && fabsf(measurements[1] - (-70.00f)) <= epsilon)
            strcpy(tempAndHumcachedResponse.get(), "AM2320 sensor offline");
        else if (fabsf(measurements[0] - (-1000.00f)) <= epsilon && fabsf(measurements[1] - (-1000.00f)) <= epsilon)
            strcpy(tempAndHumcachedResponse.get(), "AM2320 something unexpected happens");
        else
        {
            Json measurementJson;
            measurementJson.add("Temperature", measurements[0]);
            measurementJson.add("Humidity", measurements[1]);
            strcpy(tempAndHumcachedResponse.get(), measurementJson.toString().c_str());
            xSemaphoreGive(xMutexSensor);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}