#pragma once

#include <cstdint>

struct SensorData {
    double temperature = 25.0;
    double humidity = 65.0;
    double soilMoisture = 55.0;
    std::int32_t lightLux = 18000;
    std::int32_t co2Ppm = 700;
};
