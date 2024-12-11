#ifndef WORK_TASKS_H
#define WORK_TASKS_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

void LampTask(void *pvParameters);
void TemperatureInHoursTask(void *pvParameters);

#endif