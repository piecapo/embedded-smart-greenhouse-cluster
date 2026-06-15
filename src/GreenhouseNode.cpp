#include "GreenhouseNode.hpp"

#include <algorithm>
#include <iomanip>
#include <random>
#include <sstream>
#include <utility>

namespace {
double clampDouble(double value, double low, double high) {
    return std::max(low, std::min(high, value));
}

int clampInt(int value, int low, int high) {
    return std::max(low, std::min(high, value));
}

std::mt19937& rng() {
    static std::mt19937 engine{std::random_device{}()};
    return engine;
}

double randomDouble(double low, double high) {
    std::uniform_real_distribution<double> dist(low, high);
    return dist(rng());
}

int randomInt(int low, int high) {
    std::uniform_int_distribution<int> dist(low, high);
    return dist(rng());
}
}

GreenhouseNode::GreenhouseNode(std::string id, std::string name, std::string crop, SensorData data)
    : id_(std::move(id)), name_(std::move(name)), crop_(std::move(crop)), sensors_(data) {}

const std::string& GreenhouseNode::id() const {
    return id_;
}

const std::string& GreenhouseNode::name() const {
    return name_;
}

const std::string& GreenhouseNode::crop() const {
    return crop_;
}

const SensorData& GreenhouseNode::sensors() const {
    return sensors_;
}

const ActuatorState& GreenhouseNode::actuators() const {
    return actuators_;
}

ControlMode GreenhouseNode::mode() const {
    return mode_;
}

void GreenhouseNode::setMode(ControlMode mode) {
    mode_ = mode;
}

void GreenhouseNode::setActuators(const ActuatorState& actuators) {
    actuators_ = actuators;
}

void GreenhouseNode::updateSensors(const SensorData& data) {
    sensors_ = data;
}

void GreenhouseNode::simulateOneStep() {
    sensors_.temperature = clampDouble(sensors_.temperature + randomDouble(-0.6, 0.8), 16.0, 38.0);
    sensors_.humidity = clampDouble(sensors_.humidity + randomDouble(-1.5, 1.3), 35.0, 95.0);
    sensors_.soilMoisture = clampDouble(sensors_.soilMoisture + randomDouble(-1.8, 1.0), 20.0, 90.0);
    sensors_.lightLux = clampInt(sensors_.lightLux + randomInt(-2400, 2600), 2000, 52000);
    sensors_.co2Ppm = clampInt(sensors_.co2Ppm + randomInt(-50, 70), 350, 1800);

    if (actuators_.pump) {
        sensors_.soilMoisture = clampDouble(sensors_.soilMoisture + 3.0, 20.0, 90.0);
    }
    if (actuators_.fan) {
        sensors_.temperature = clampDouble(sensors_.temperature - 0.8, 16.0, 38.0);
        sensors_.co2Ppm = clampInt(sensors_.co2Ppm - 80, 350, 1800);
    }
    if (actuators_.growLight) {
        sensors_.lightLux = clampInt(sensors_.lightLux + 3600, 2000, 52000);
    }

    if (mode_ == ControlMode::Auto) {
        applyAutoControl();
    }
}

void GreenhouseNode::applyAutoControl() {
    actuators_.fan = sensors_.temperature > 28.0 || sensors_.co2Ppm > 1000;
    actuators_.pump = sensors_.soilMoisture < 45.0;
    actuators_.growLight = sensors_.lightLux < 12000;

    if (sensors_.temperature > 30.0 || sensors_.humidity > 82.0) {
        actuators_.ventPercent = 85;
    } else if (sensors_.temperature > 27.0) {
        actuators_.ventPercent = 55;
    } else {
        actuators_.ventPercent = 25;
    }
}

std::string GreenhouseNode::statusLine() const {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1)
        << id_ << " | " << name_ << " | 作物:" << crop_
        << " | 温度:" << sensors_.temperature << "C"
        << " | 湿度:" << sensors_.humidity << "%"
        << " | 土壤:" << sensors_.soilMoisture << "%"
        << " | 光照:" << sensors_.lightLux << "lux"
        << " | CO2:" << sensors_.co2Ppm << "ppm"
        << " | 风机:" << (actuators_.fan ? "开" : "关")
        << " | 水泵:" << (actuators_.pump ? "开" : "关")
        << " | 补光:" << (actuators_.growLight ? "开" : "关")
        << " | 通风窗:" << actuators_.ventPercent << "%";
    return out.str();
}
