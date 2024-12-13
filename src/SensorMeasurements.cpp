#include "ConfigManager.h"
#include "SensorMeasurements.h"

/*Get temperature from am2320 sensor*/
 /*Получить температуру с датчика am2320*/
std::unique_ptr<float[]> GetMeasurementsFromSensor()
{
    std::unique_ptr<float[]> array(new float[2]);
    if(IS_TEST_MODE)
    {
        array[0] = random(-1.0f, 15.0f);
        array[1] = random(-1.0f, 15.0f);
        return array;  
    }
    
    AM2320 am2320(&Wire);
    Wire.begin(SDA_PIN, SCL_PIN);
    switch (am2320.Read())
    {
    case 2:
        // Error reading AM2320 sensor
        {
            array[0] = -50.00f;
            array[1] = -60.00f;
            Wire.end();
            return array;
        }
    case 1:
        // AM2320 sensor offline
        {
            array[0] = -60.00f;
            array[1] = -70.00f;
            Wire.end();
            return array;
        }
    case 0:
    {
        float tempTemp = 0, tempHum = 0;
        int i = 0;
        while (i < 3)
        {
            tempTemp += am2320.cTemp;
            tempHum += am2320.Humidity;
            i++;
            delay(2000);
        }
        array[0] = tempTemp / i;
        array[1] = tempHum / i;
        Wire.end();
        return array;
    }
    default:
        /*Возвращаем какое-то значение по умолчанию, если вдруг произошло что-то неожиданное*/
        /*Return some default value if something unexpected happens*/
        {
            array[0] = -1000.00f;
            array[1] = -1000.00f;
            Wire.end();
            return array;
        }
    }
}