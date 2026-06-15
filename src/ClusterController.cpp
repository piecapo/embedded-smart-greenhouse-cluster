#include "ClusterController.hpp"

#include <iostream>

void ClusterController::addNode(GreenhouseNode node) {
    nodes_.push_back(std::move(node));
}

void ClusterController::simulate(int rounds) {
    for (int round = 1; round <= rounds; ++round) {
        std::cout << "\n========== 第 " << round << " 轮环境采集 ==========" << '\n';
        for (auto& node : nodes_) {
            node.simulateOneStep();
        }
        printDashboard();
        printAlerts();
    }
}

void ClusterController::printDashboard() const {
    std::cout << "\n[集群状态]\n";
    for (const auto& node : nodes_) {
        std::cout << node.statusLine() << '\n';
    }
}

void ClusterController::printAlerts() const {
    std::cout << "\n[预警信息]\n";
    bool hasAlert = false;
    for (const auto& node : nodes_) {
        const auto& data = node.sensors();
        if (data.temperature > 30.0) {
            hasAlert = true;
            std::cout << "- " << node.id() << " 高温预警：当前 " << data.temperature << "C，建议通风降温。\n";
        }
        if (data.soilMoisture < 42.0) {
            hasAlert = true;
            std::cout << "- " << node.id() << " 缺水预警：土壤湿度 " << data.soilMoisture << "%，建议灌溉。\n";
        }
        if (data.lightLux < 10000) {
            hasAlert = true;
            std::cout << "- " << node.id() << " 光照不足：当前 " << data.lightLux << "lux，建议补光。\n";
        }
        if (data.co2Ppm > 1200) {
            hasAlert = true;
            std::cout << "- " << node.id() << " CO2 偏高：当前 " << data.co2Ppm << "ppm，建议通风。\n";
        }
    }
    if (!hasAlert) {
        std::cout << "- 当前无异常。\n";
    }
}

bool ClusterController::setManualCommand(const std::string& nodeId, const std::string& device, bool enabled) {
    for (auto& node : nodes_) {
        if (node.id() != nodeId) {
            continue;
        }

        ActuatorState state = node.actuators();
        if (device == "fan") {
            state.fan = enabled;
        } else if (device == "pump") {
            state.pump = enabled;
        } else if (device == "light") {
            state.growLight = enabled;
        } else {
            return false;
        }

        node.setMode(ControlMode::Manual);
        node.setActuators(state);
        return true;
    }
    return false;
}
