#pragma once

#include "ActuatorState.hpp"
#include "SensorData.hpp"

#include <string>

enum class ControlMode {
    Auto,
    Manual
};

class GreenhouseNode {
public:
    GreenhouseNode(std::string id, std::string name, std::string crop, SensorData data);

    const std::string& id() const;
    const std::string& name() const;
    const std::string& crop() const;
    const SensorData& sensors() const;
    const ActuatorState& actuators() const;
    ControlMode mode() const;

    void setMode(ControlMode mode);
    void setActuators(const ActuatorState& actuators);
    void updateSensors(const SensorData& data);
    void simulateOneStep();
    void applyAutoControl();
    std::string statusLine() const;

private:
    std::string id_;
    std::string name_;
    std::string crop_;
    SensorData sensors_;
    ActuatorState actuators_;
    ControlMode mode_ = ControlMode::Auto;
};
