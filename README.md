# 嵌入式智能大棚蔬菜集群管理系统

这是一个面向课程设计、创新实践和毕业设计雏形的 C++ 嵌入式智能农业项目。项目以多个蔬菜大棚为集群对象，通过 C++ 程序模拟温湿度、土壤湿度、光照和 CO2 数据，并联动风机、水泵、补光灯和卷帘/通风窗，实现自动化环境调控。项目同时包含 ESP32/Arduino C++ 固件示例，可继续移植到真实硬件。

## 项目亮点

- C++17 主程序：使用类封装大棚节点、传感器数据、执行器状态和集群控制器。
- 多大棚集群管理：支持 6 个以上大棚节点统一监控。
- 嵌入式控制闭环：根据阈值自动控制风机、水泵、补光灯和通风窗。
- ESP32 固件示例：真实硬件端也使用 C++/Arduino 风格编写。
- 可选网页演示：附带本地看板，用于没有硬件时展示效果。
- GitHub 友好：包含固件、服务端、网页端、硬件清单、接口文档和演示说明。

## 目录结构

```text
embedded-smart-greenhouse-cluster/
  include/                           C++ 头文件
  src/                               C++ 主程序与控制逻辑
  firmware/esp32_greenhouse_node/    ESP32 节点固件示例
  server/                            可选本地网页演示服务
  web/                               可选网页监控看板
  docs/                              中文项目文档
  hardware/                          硬件清单与引脚表
  scripts/                           演示数据脚本
  data/                              示例数据
```

## C++ 快速运行

如果电脑已安装 CMake 和 C++ 编译器：

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\greenhouse_cluster.exe 5
```

如果使用 Linux/macOS：

```bash
cmake -S . -B build
cmake --build build
./build/greenhouse_cluster 5
```

参数 `5` 表示仿真 5 轮环境采集与自动控制。

## 可选网页演示

进入项目目录后运行：

```powershell
python server/app.py
```

然后浏览器打开：

```text
http://127.0.0.1:8080
```

## C++ 默认演示功能

- 创建 6 个蔬菜大棚节点。
- 模拟温度、湿度、土壤湿度、光照、CO2 的动态变化。
- 自动判断是否开启风机、水泵、补光灯和通风窗。
- 输出每轮集群状态。
- 输出高温、缺水、光照不足、CO2 偏高等预警。

## 适合扩展的方向

- 接入真实 MQTT 服务器。
- 接入 MySQL、InfluxDB 或 SQLite 持久化数据。
- 增加手机端小程序。
- 增加模型预测控制，例如根据天气预报提前调节。
- 增加摄像头病虫害识别。
