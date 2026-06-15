from __future__ import annotations

import csv
import random
from datetime import datetime, timedelta
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "data" / "sample_history.csv"


def main() -> None:
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    start = datetime.now() - timedelta(hours=12)
    rows = []
    crops = ["番茄", "黄瓜", "生菜", "辣椒", "草莓", "小白菜"]

    for minute in range(0, 12 * 60, 10):
        ts = start + timedelta(minutes=minute)
        for index, crop in enumerate(crops, start=1):
            rows.append(
                {
                    "time": ts.isoformat(timespec="minutes"),
                    "node_id": f"GH-{index:02d}",
                    "crop": crop,
                    "temperature": round(random.uniform(21, 32), 1),
                    "humidity": round(random.uniform(52, 85), 1),
                    "soil_moisture": round(random.uniform(35, 78), 1),
                    "light_lux": random.randint(7000, 42000),
                    "co2_ppm": random.randint(480, 1300),
                }
            )

    with OUTPUT.open("w", newline="", encoding="utf-8-sig") as file:
        writer = csv.DictWriter(file, fieldnames=list(rows[0].keys()))
        writer.writeheader()
        writer.writerows(rows)
    print(f"已生成演示数据：{OUTPUT}")


if __name__ == "__main__":
    main()
