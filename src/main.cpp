#include "ClusterController.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
SensorData makeSensor(double temperature, double humidity, double soil, int light, int co2) {
    SensorData data;
    data.temperature = temperature;
    data.humidity = humidity;
    data.soilMoisture = soil;
    data.lightLux = light;
    data.co2Ppm = co2;
    return data;
}

ClusterController buildDemoCluster() {
    ClusterController cluster;
    cluster.addNode(GreenhouseNode("GH-01", "1号智能大棚", "番茄", makeSensor(26.2, 66.0, 56.0, 21000, 780)));
    cluster.addNode(GreenhouseNode("GH-02", "2号智能大棚", "黄瓜", makeSensor(27.8, 72.0, 48.0, 18000, 910)));
    cluster.addNode(GreenhouseNode("GH-03", "3号智能大棚", "生菜", makeSensor(23.4, 70.0, 62.0, 9500, 640)));
    cluster.addNode(GreenhouseNode("GH-04", "4号智能大棚", "辣椒", makeSensor(30.4, 61.0, 44.0, 26000, 1120)));
    cluster.addNode(GreenhouseNode("GH-05", "5号智能大棚", "草莓", makeSensor(24.6, 76.0, 58.0, 16000, 720)));
    cluster.addNode(GreenhouseNode("GH-06", "6号智能大棚", "小白菜", makeSensor(22.8, 68.0, 55.0, 12500, 690)));
    return cluster;
}

int parseRounds(int argc, char** argv) {
    if (argc < 2) {
        return 5;
    }
    int rounds = std::atoi(argv[1]);
    return rounds > 0 ? rounds : 5;
}
}

int main(int argc, char** argv) {
    std::cout << "嵌入式智能大棚蔬菜集群 C++ 仿真程序\n";
    std::cout << "用法：greenhouse_cluster [仿真轮数]\n";

    ClusterController cluster = buildDemoCluster();
    int rounds = parseRounds(argc, argv);
    cluster.simulate(rounds);

    std::cout << "\n仿真结束。后续可将该控制逻辑移植到 ESP32 或其他嵌入式平台。\n";
    return 0;
}
