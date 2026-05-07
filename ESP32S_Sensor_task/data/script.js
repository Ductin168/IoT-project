// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', onLoad);

function onLoad(event) { initWebSocket(); }

function updateConnectionStatus(isConnected) {
    const statusDiv = document.getElementById("ws-status");
    if (statusDiv) {
        statusDiv.className = isConnected ? "ws-connected" : "ws-disconnected";
        statusDiv.innerHTML = isConnected ? '<i class="fa-solid fa-wifi"></i> <span>Đã kết nối</span>' : '<i class="fa-solid fa-triangle-exclamation"></i> <span>Mất kết nối</span>';
    }
}

function onOpen(event) {
    console.log('✅ WebSocket Connection opened');
    updateConnectionStatus(true);
    Send_Data(JSON.stringify({ page: "get_status" }));
}

function onClose(event) {
    console.log('🔴 WebSocket Connection closed');
    updateConnectionStatus(false);
    setTimeout(initWebSocket, 2000);
}

function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(data);
        console.log("📤 Gửi đi:", data);
    }
}

function onMessage(event) {
    console.log("📩 Nhận về:", event.data);
    try {
        var data = JSON.parse(event.data);
        
if (data.page === "home") {
            if (data.temp !== undefined && gaugeTemp) gaugeTemp.refresh(data.temp);
            if (data.humi !== undefined && gaugeHumi) gaugeHumi.refresh(data.humi);
            
            // 👉 THÊM ĐOẠN XỬ LÝ DỮ LIỆU TINYML NÀY VÀO
            if (data.ai_score !== undefined) {
                document.getElementById("ai-score").innerText = parseFloat(data.ai_score).toFixed(4);
                const aiCard = document.getElementById("ai-alert");
                const aiText = document.getElementById("ai-status-text");

                if (data.is_anomaly) {
                    aiCard.className = "ai-card danger";
                    aiText.innerHTML = "⚠️ CẢNH BÁO: Phát hiện thông số bất thường!";
                    aiText.style.color = "#c62828";
                    aiText.style.fontWeight = "bold";
                } else {
                    aiCard.className = "ai-card safe";
                    aiText.innerHTML = "✅ Môi trường ổn định, hệ thống bình thường.";
                    aiText.style.color = "#2e7d32";
                    aiText.style.fontWeight = "normal";
                }
            }
        }
        
        // ĐỔI MÀU NÚT KHI BẤM TRÊN WEB
        if (data.page === "device_status") {
            const relay = relayList.find(r => parseInt(r.gpio) === parseInt(data.gpio));
            if (relay) {
                relay.state = (data.status === "ON");
                relay.isWaiting = false; 
                localStorage.setItem('relays', JSON.stringify(relayList)); 
                renderRelays(); 
            }
        }

        // 👉 TUYỆT CHIÊU: XỬ LÝ LỆNH TỪ MÂY CORE IOT DỘI XUỐNG
        if (data.page === "cloud_update") {
            // So sánh tên (bỏ khoảng trắng, đưa về chữ thường giống logic trên C++)
            const cloudName = data.name.toLowerCase().replace(/\s/g, "");
            const relay = relayList.find(r => r.name.toLowerCase().replace(/\s/g, "") === cloudName);

            if (relay) {
                // 1. Web tự lật màu cho khớp mây
                relay.state = (data.status === "ON");
                relay.isWaiting = false;
                localStorage.setItem('relays', JSON.stringify(relayList));
                renderRelays();

                // 2. Web cứu giá: Gửi ngược lệnh xuống báo cho ESP32 biết phải bật GPIO nào
                const relayJSON = JSON.stringify({
                    page: "device",
                    value: {
                        name: relay.name,
                        status: data.status,
                        gpio: relay.gpio
                    }
                });
                Send_Data(relayJSON);
                console.log("⚡ Web đã làm cầu nối kích hoạt GPIO " + relay.gpio + " cho Mây!");
            }
        }

    } catch (e) { console.warn("Lỗi:", e); }
}

// ==================== UI NAVIGATION ====================
let relayList = JSON.parse(localStorage.getItem('relays')) || [];
let deleteTarget = null;

function showSection(id, event) {
    document.querySelectorAll('.section').forEach(sec => sec.style.display = 'none');
    document.getElementById(id).style.display = id === 'settings' ? 'flex' : 'block';
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    event.currentTarget.classList.add('active');
}

// ==================== HOME GAUGES ====================
var gaugeTemp;
var gaugeHumi;

window.onload = function () {
    gaugeTemp = new JustGage({
        id: "gauge_temp", value: 0, min: -10, max: 60,
        donut: true, pointer: false, gaugeWidthScale: 0.25, gaugeColor: "#eee",
        levelColorsGradient: true, levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"]
    });
    gaugeHumi = new JustGage({
        id: "gauge_humi", value: 0, min: 0, max: 100,
        donut: true, pointer: false, gaugeWidthScale: 0.25, gaugeColor: "#eee",
        levelColorsGradient: true, levelColors: ["#42A5F5", "#00BCD4", "#0288D1"]
    });
    renderRelays(); 
};

// ==================== DEVICE FUNCTIONS ====================
function openAddRelayDialog() { document.getElementById('addRelayDialog').style.display = 'flex'; }
function closeAddRelayDialog() { document.getElementById('addRelayDialog').style.display = 'none'; }

function saveRelay() {
    const name = document.getElementById('relayName').value.trim();
    const gpio = document.getElementById('relayGPIO').value.trim();
    if (!name || !gpio) return alert("⚠️ Vui lòng nhập đủ tên và GPIO!");
    
    relayList.push({ id: Date.now(), name, gpio: parseInt(gpio), state: false, isWaiting: false });
    localStorage.setItem('relays', JSON.stringify(relayList)); 
    renderRelays();
    closeAddRelayDialog();
}

function renderRelays() {
    const container = document.getElementById('relayContainer');
    if (!container) return; 
    container.innerHTML = "";
    relayList.forEach(r => {
        const card = document.createElement('div');
        card.className = `device-card ${r.state ? 'active-card' : ''}`; 
        let btnClass = r.state ? "on" : "";
        let btnText = r.state ? '<i class="fa-solid fa-power-off"></i> BẬT' : '<i class="fa-solid fa-power-off"></i> TẮT';
        
        if (r.isWaiting) {
            btnClass = "waiting";
            btnText = '<i class="fa-solid fa-spinner fa-spin"></i> Chờ...';
        }
        card.innerHTML = `
          <i class="fa-regular fa-lightbulb device-icon"></i>
          <h3>${r.name}</h3>
          <p>Chân GPIO: ${r.gpio}</p>
          <button class="toggle-btn ${btnClass}" onclick="toggleRelay(${r.id})">${btnText}</button>
          <i class="fa-solid fa-trash delete-icon" onclick="showDeleteDialog(${r.id})"></i>
        `;
        container.appendChild(card);
    });
}

function toggleRelay(id) {
    const relay = relayList.find(r => r.id === id);
    if (relay && !relay.isWaiting) {
        const requestedStatus = relay.state ? "OFF" : "ON";
        relay.isWaiting = true; 
        renderRelays();
        Send_Data(JSON.stringify({
            page: "device",
            value: { name: relay.name, status: requestedStatus, gpio: relay.gpio }
        }));
        setTimeout(() => {
            if (relay.isWaiting) {
                relay.isWaiting = false;
                renderRelays();
            }
        }, 3000);
    }
}

function showDeleteDialog(id) {
    deleteTarget = id;
    document.getElementById('confirmDeleteDialog').style.display = 'flex';
}
function closeConfirmDelete() { document.getElementById('confirmDeleteDialog').style.display = 'none'; }
function confirmDelete() {
    relayList = relayList.filter(r => r.id !== deleteTarget);
    localStorage.setItem('relays', JSON.stringify(relayList));
    renderRelays();
    closeConfirmDelete();
}

// ==================== SETTINGS FORM ====================
document.getElementById("settingsForm").addEventListener("submit", function (e) {
    e.preventDefault();
    Send_Data(JSON.stringify({
        page: "setting",
        value: {
            ssid: document.getElementById("ssid").value.trim(),
            password: document.getElementById("password").value.trim(),
            token: document.getElementById("token").value.trim(),
            server: document.getElementById("server").value.trim(),
            port: document.getElementById("port").value.trim()
        }
    }));
    alert("✅ Cấu hình đã được gửi đến mạch ESP32!");
});