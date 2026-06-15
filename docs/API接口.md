# API 接口说明

服务默认地址：

```text
http://127.0.0.1:8080
```

## 获取节点列表

```http
GET /api/nodes
```

返回：

```json
{
  "nodes": [
    {
      "node_id": "GH-01",
      "name": "1号智能大棚",
      "crop": "番茄",
      "temperature": 26.5,
      "humidity": 66.2,
      "soil_moisture": 52.1,
      "light_lux": 18000,
      "co2_ppm": 760
    }
  ]
}
```

## 获取预警列表

```http
GET /api/alerts
```

## 获取汇总数据

```http
GET /api/summary
```

## 手动控制设备

```http
POST /api/nodes/GH-01/command
Content-Type: application/json
```

示例：

```json
{
  "mode": "manual",
  "actuator": "pump",
  "value": true
}
```

支持的 `actuator`：

- `fan`：风机
- `pump`：水泵
- `light`：补光灯
- `vent`：通风窗开度，0 到 100
