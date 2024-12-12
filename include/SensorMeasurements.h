#ifndef SENSOR_MEASUREMENTS_H
#define SENSOR_MEASUREMENTS_H

#include <memory>

std::unique_ptr<float[]> GetMeasurementsFromSensor();

#endif // SENSOR_MEASUREMENTS_H