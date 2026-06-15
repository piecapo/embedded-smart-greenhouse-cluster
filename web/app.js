const summaryEl = document.querySelector("#summary");
const nodesEl = document.querySelector("#nodes");
const alertsEl = document.querySelector("#alerts");
const refreshButton = document.querySelector("#refreshButton");

const api = {
  nodes: "/api/nodes",
  alerts: "/api/alerts",
  summary: "/api/summary",
  simulate: "/api/simulate",
};

async function fetchJson(url, options) {
  const response = await fetch(url, options);
  if (!response.ok) {
    throw new Error(`请求失败：${response.status}`);
  }
  return response.json();
}

function metric(label, value, unit = "") {
  return `<div class="metric"><span>${label}</span><strong>${value}</strong>${unit}</div>`;
}

function renderSummary(summary) {
  summaryEl.innerHTML = [
    metric("大棚数量", summary.node_count, " 个"),
    metric("预警数量", summary.alert_count, " 条"),
    metric("平均温度", summary.avg_temperature, " ℃"),
    metric("平均土壤湿度", summary.avg_soil_moisture, " %"),
    metric("运行设备", summary.running_devices, " 台"),
  ].join("");
}

function renderNodes(nodes) {
  nodesEl.innerHTML = nodes.map((node) => {
    const actuators = node.actuators;
    return `
      <article class="node-card">
        <div class="node-head">
          <div>
            <div class="node-title">${node.name}</div>
            <div class="crop">${node.node_id} · 当前作物：${node.crop}</div>
          </div>
          <span class="badge">${node.mode === "auto" ? "自动" : "手动"}</span>
        </div>
        <div class="sensor-grid">
          ${sensor("温度", node.temperature, "℃")}
          ${sensor("空气湿度", node.humidity, "%")}
          ${sensor("土壤湿度", node.soil_moisture, "%")}
          ${sensor("光照", node.light_lux, "lux")}
          ${sensor("CO2", node.co2_ppm, "ppm")}
          ${sensor("通风窗", actuators.vent, "%")}
        </div>
        <div class="actuators">
          ${chip("风机", actuators.fan)}
          ${chip("水泵", actuators.pump)}
          ${chip("补光灯", actuators.light)}
          <button class="secondary" onclick="toggleDevice('${node.node_id}', 'fan', ${!actuators.fan})">风机</button>
          <button class="secondary" onclick="toggleDevice('${node.node_id}', 'pump', ${!actuators.pump})">水泵</button>
          <button class="secondary" onclick="toggleDevice('${node.node_id}', 'light', ${!actuators.light})">补光</button>
        </div>
      </article>
    `;
  }).join("");
}

function sensor(label, value, unit) {
  return `<div class="sensor"><span>${label}</span><strong>${value}</strong> ${unit}</div>`;
}

function chip(label, on) {
  return `<span class="chip ${on ? "on" : ""}">${label}${on ? "运行中" : "关闭"}</span>`;
}

function renderAlerts(alerts) {
  if (!alerts.length) {
    alertsEl.innerHTML = `<div class="empty">当前没有异常预警。</div>`;
    return;
  }

  alertsEl.innerHTML = alerts.map((item) => `
    <div class="alert ${item.level}">
      <div class="alert-title">${item.title}</div>
      <div>${item.message}</div>
    </div>
  `).join("");
}

async function toggleDevice(nodeId, actuator, value) {
  await fetchJson(`/api/nodes/${nodeId}/command`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ mode: "manual", actuator, value }),
  });
  await loadDashboard();
}

async function loadDashboard() {
  const [summary, nodesData, alertsData] = await Promise.all([
    fetchJson(api.summary),
    fetchJson(api.nodes),
    fetchJson(api.alerts),
  ]);
  renderSummary(summary);
  renderNodes(nodesData.nodes);
  renderAlerts(alertsData.alerts);
}

refreshButton.addEventListener("click", async () => {
  await fetchJson(api.simulate, { method: "POST" });
  await loadDashboard();
});

loadDashboard();
setInterval(loadDashboard, 5000);
