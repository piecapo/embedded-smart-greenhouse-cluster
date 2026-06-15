from __future__ import annotations

import json
import math
import random
import time
from dataclasses import asdict, dataclass, field
from http import HTTPStatus
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any
from urllib.parse import urlparse


ROOT = Path(__file__).resolve().parents[1]
WEB_ROOT = ROOT / "web"
DATA_DIR = ROOT / "data"
STATE_FILE = DATA_DIR / "runtime_state.json"


@dataclass
class ActuatorState:
    fan: bool = False
    pump: bool = False
    light: bool = False
    vent: int = 20


@dataclass
class GreenhouseNode:
    node_id: str
    name: str
    crop: str
    temperature: float
    humidity: float
    soil_moisture: float
    light_lux: int
    co2_ppm: int
    actuators: ActuatorState = field(default_factory=ActuatorState)
    mode: str = "auto"
    updated_at: float = field(default_factory=time.time)


def now_ms() -> int:
    return int(time.time() * 1000)


def create_initial_nodes() -> list[GreenhouseNode]:
    crops = ["番茄", "黄瓜", "生菜", "辣椒", "草莓", "小白菜"]
    nodes: list[GreenhouseNode] = []
    for index, crop in enumerate(crops, start=1):
        nodes.append(
            GreenhouseNode(
                node_id=f"GH-{index:02d}",
                name=f"{index}号智能大棚",
                crop=crop,
                temperature=round(random.uniform(22.0, 30.0), 1),
                humidity=round(random.uniform(58.0, 78.0), 1),
                soil_moisture=round(random.uniform(42.0, 76.0), 1),
                light_lux=random.randint(9000, 36000),
                co2_ppm=random.randint(520, 1100),
            )
        )
    return nodes


def load_nodes() -> list[GreenhouseNode]:
    if not STATE_FILE.exists():
        return create_initial_nodes()

    data = json.loads(STATE_FILE.read_text(encoding="utf-8"))
    nodes: list[GreenhouseNode] = []
    for item in data.get("nodes", []):
        actuators = ActuatorState(**item.get("actuators", {}))
        item = {key: value for key, value in item.items() if key != "actuators"}
        nodes.append(GreenhouseNode(**item, actuators=actuators))
    return nodes or create_initial_nodes()


def save_nodes(nodes: list[GreenhouseNode]) -> None:
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    payload = {"updated_at": now_ms(), "nodes": [asdict(node) for node in nodes]}
    STATE_FILE.write_text(json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8")


def apply_control_strategy(node: GreenhouseNode) -> None:
    if node.mode != "auto":
        return

    node.actuators.fan = node.temperature > 28.0 or node.co2_ppm > 1000
    node.actuators.pump = node.soil_moisture < 45.0
    node.actuators.light = node.light_lux < 12000

    if node.temperature > 30.0 or node.humidity > 82.0:
        node.actuators.vent = 85
    elif node.temperature > 27.0:
        node.actuators.vent = 55
    else:
        node.actuators.vent = 25


def simulate(nodes: list[GreenhouseNode]) -> list[GreenhouseNode]:
    minute = time.time() / 60
    for index, node in enumerate(nodes):
        wave = math.sin(minute + index)
        node.temperature = clamp(node.temperature + random.uniform(-0.5, 0.7) + wave * 0.15, 16, 38)
        node.humidity = clamp(node.humidity + random.uniform(-1.4, 1.2), 35, 95)
        node.soil_moisture = clamp(node.soil_moisture + random.uniform(-1.8, 1.0), 20, 90)
        node.light_lux = int(clamp(node.light_lux + random.randint(-2200, 2600), 2000, 52000))
        node.co2_ppm = int(clamp(node.co2_ppm + random.randint(-45, 65), 350, 1800))

        if node.actuators.pump:
            node.soil_moisture = clamp(node.soil_moisture + 3.2, 20, 90)
        if node.actuators.fan:
            node.temperature = clamp(node.temperature - 0.7, 16, 38)
            node.co2_ppm = int(clamp(node.co2_ppm - 70, 350, 1800))
        if node.actuators.light:
            node.light_lux = int(clamp(node.light_lux + 3500, 2000, 52000))

        node.temperature = round(node.temperature, 1)
        node.humidity = round(node.humidity, 1)
        node.soil_moisture = round(node.soil_moisture, 1)
        node.updated_at = time.time()
        apply_control_strategy(node)
    save_nodes(nodes)
    return nodes


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def build_alerts(nodes: list[GreenhouseNode]) -> list[dict[str, Any]]:
    alerts: list[dict[str, Any]] = []
    for node in nodes:
        if node.temperature > 30:
            alerts.append(alert(node, "高温预警", f"{node.name} 温度 {node.temperature}℃，建议加强通风。", "danger"))
        if node.soil_moisture < 42:
            alerts.append(alert(node, "缺水预警", f"{node.name} 土壤湿度 {node.soil_moisture}%，建议灌溉。", "warning"))
        if node.light_lux < 10000:
            alerts.append(alert(node, "光照不足", f"{node.name} 光照 {node.light_lux} lux，建议开启补光。", "info"))
        if node.co2_ppm > 1200:
            alerts.append(alert(node, "CO2 偏高", f"{node.name} CO2 {node.co2_ppm} ppm，建议通风。", "warning"))
    return alerts


def alert(node: GreenhouseNode, title: str, message: str, level: str) -> dict[str, Any]:
    return {
        "id": f"{node.node_id}-{title}",
        "node_id": node.node_id,
        "title": title,
        "message": message,
        "level": level,
        "created_at": now_ms(),
    }


class GreenhouseHandler(SimpleHTTPRequestHandler):
    def translate_path(self, path: str) -> str:
        parsed = urlparse(path)
        if parsed.path == "/":
            return str(WEB_ROOT / "index.html")
        return str(WEB_ROOT / parsed.path.lstrip("/"))

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path == "/api/nodes":
            nodes = simulate(load_nodes())
            self.send_json({"nodes": [asdict(node) for node in nodes], "timestamp": now_ms()})
            return
        if parsed.path == "/api/alerts":
            nodes = load_nodes()
            self.send_json({"alerts": build_alerts(nodes), "timestamp": now_ms()})
            return
        if parsed.path == "/api/summary":
            nodes = load_nodes()
            self.send_json(build_summary(nodes))
            return
        return super().do_GET()

    def do_POST(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path == "/api/simulate":
            nodes = simulate(load_nodes())
            self.send_json({"ok": True, "nodes": [asdict(node) for node in nodes]})
            return

        if parsed.path.startswith("/api/nodes/") and parsed.path.endswith("/command"):
            node_id = parsed.path.split("/")[3]
            payload = self.read_json()
            nodes = load_nodes()
            target = next((node for node in nodes if node.node_id == node_id), None)
            if target is None:
                self.send_json({"ok": False, "message": "节点不存在"}, HTTPStatus.NOT_FOUND)
                return

            target.mode = payload.get("mode", target.mode)
            actuator = payload.get("actuator")
            value = payload.get("value")
            if actuator in {"fan", "pump", "light"}:
                setattr(target.actuators, actuator, bool(value))
            if actuator == "vent":
                target.actuators.vent = int(clamp(float(value), 0, 100))
            target.updated_at = time.time()
            save_nodes(nodes)
            self.send_json({"ok": True, "node": asdict(target)})
            return

        self.send_json({"ok": False, "message": "接口不存在"}, HTTPStatus.NOT_FOUND)

    def read_json(self) -> dict[str, Any]:
        length = int(self.headers.get("Content-Length", "0") or 0)
        if length <= 0:
            return {}
        raw = self.rfile.read(length)
        return json.loads(raw.decode("utf-8"))

    def send_json(self, payload: dict[str, Any], status: HTTPStatus = HTTPStatus.OK) -> None:
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(body)


def build_summary(nodes: list[GreenhouseNode]) -> dict[str, Any]:
    alerts = build_alerts(nodes)
    return {
        "node_count": len(nodes),
        "alert_count": len(alerts),
        "avg_temperature": round(sum(node.temperature for node in nodes) / len(nodes), 1),
        "avg_soil_moisture": round(sum(node.soil_moisture for node in nodes) / len(nodes), 1),
        "running_devices": sum(
            int(node.actuators.fan) + int(node.actuators.pump) + int(node.actuators.light)
            for node in nodes
        ),
        "timestamp": now_ms(),
    }


def main() -> None:
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    if not STATE_FILE.exists():
        save_nodes(create_initial_nodes())

    host = "127.0.0.1"
    port = 8080
    server = ThreadingHTTPServer((host, port), GreenhouseHandler)
    print(f"智能大棚集群服务已启动：http://{host}:{port}")
    print("按 Ctrl+C 停止服务")
    server.serve_forever()


if __name__ == "__main__":
    main()
