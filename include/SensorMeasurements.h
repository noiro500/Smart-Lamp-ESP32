#pragma once
#define SENSOR_MEASUREMENTS_H

#include <memory>

std::unique_ptr<float[]> GetMeasurementsFromSensor();