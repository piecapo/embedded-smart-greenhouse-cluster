#pragma once

#include "GreenhouseNode.hpp"

#include <string>
#include <vector>

class ClusterController {
public:
    void addNode(GreenhouseNode node);
    void simulate(int rounds);
    void printDashboard() const;
    void printAlerts() const;
    bool setManualCommand(const std::string& nodeId, const std::string& device, bool enabled);

private:
    std::vector<GreenhouseNode> nodes_;
};
