#include "WorkTasks.h"
#include "ConfigManager.h"
#include "ESP32Time.h"

SemaphoreHandle_t xMutexConfig = xSemaphoreCreateMutex();
SemaphoreHandle_t xMutexSensor = xSemaphoreCreateMutex();
TaskHandle_t lampTaskHandle;
TaskHandle_t wifiGuardTaskHandle;
TaskHandle_t temperatureInHoursTaskHandle;
int lampPin = 22;
TempAndHumInHours temperatureInHours;

void LampTask(void *pvParameters)
{
    pinMode(lampPin, OUTPUT);
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
            digitalWrite(lampPin, HIGH);
            // Serial.println("Lamp always on");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        int hour = rtc.getHour(true);
        if (hour < config.LampOnTimeHours || hour >= config.LampOffTimeHours)
        {
            digitalWrite(lampPin, LOW);
            // Serial.println("Lamp off");
        }
        else
        {
            digitalWrite(lampPin, HIGH);
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
    // Получение начального времени для vTaskDelayUntil
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (true)
    {
        xSemaphoreTake(xMutexConfig, portMAX_DELAY);
        int currentHour = rtc.getHour(true);

        if (currentHour != lastUpdateHour)
        {
            if (currentHour == 0)
            // Очистить структуру temperatureInHours и обновить время с сервера
            {
                memset(tempAndHumInHours->hour, 0, sizeof(tempAndHumInHours->hour));
                memset(tempAndHumInHours->temperature, 0, sizeof(tempAndHumInHours->temperature));
                memset(tempAndHumInHours->humidity, 0, sizeof(tempAndHumInHours->humidity));
                if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED)
                {
                    configTime(3 * 3600, 0, "ntp0.ntp-servers.net");
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
            //delete[] measurements;
            xSemaphoreGive(xMutexSensor);
            lastUpdateHour = currentHour;
        }
        xSemaphoreGive(xMutexConfig);
        // Рассчитать задержку до начала следующего часа
        int currentMinute = rtc.getMinute();
        int currentSecond = rtc.getSecond();
        int delaySeconds = (60 - currentMinute) * 60 - currentSecond; // Секунды до следующего часа
        Serial.printf("%s\n", (rtc.getDateTime(true)).c_str());
        TickType_t delayTicks = delaySeconds * 1000 / portTICK_PERIOD_MS;
        // Ожидаем до начала следующего часа
        vTaskDelayUntil(&xLastWakeTime, delayTicks);
    }
}