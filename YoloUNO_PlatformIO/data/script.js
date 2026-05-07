// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', () => { initWebSocket(); });

function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen = () => { 
        console.log('✅ Open'); 
        updateStatus(true);
        Send_Data(JSON.stringify({ page: "get_status" }));
    };
    websocket.onclose = () => { 
        console.log('🔴 Close'); 
        updateStatus(false); 
        setTimeout(initWebSocket, 2000); 
    };
    websocket.onmessage = onMessage;
}

function updateStatus(isConnected) {
    const el = document.getElementById("ws-status");
    if (el) {
        el.className = isConnected ? "ws-connected" : "ws-disconnected";
        el.querySelector('span').innerText = isConnected ? "Tín hiệu ổn định" : "Mất kết nối hệ thống";
    }
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) websocket.send(data);
}

function onMessage(event) {
    try {
        var data = JSON.parse(event.data);
        if (data.page === "home") {
            if (data.temp !== undefined && gaugeTemp) gaugeTemp.refresh(data.temp);
            if (data.humi !== undefined && gaugeHumi) gaugeHumi.refresh(data.humi);
            
// 👉 ĐOẠN XỬ LÝ DỮ LIỆU TINYML ĐÃ ĐƯỢC SỬA LẠI ĐỂ LẤY AI_STATUS
            if (data.ai_score_temp !== undefined && data.ai_score_humi !== undefined) {
                // Cập nhật 2 điểm riêng biệt
                document.getElementById("ai-score-temp").innerText = parseFloat(data.ai_score_temp).toFixed(4);
                document.getElementById("ai-score-humi").innerText = parseFloat(data.ai_score_humi).toFixed(4);
                
                const aiCard = document.getElementById("ai-alert");
                const aiText = document.getElementById("ai-status-text");

                if (data.is_anomaly) {
                    aiCard.className = "ai-card danger";
                    // HIỂN THỊ ĐÚNG CHUẨN ĐOÁN CỦA AI BẮN TỪ MẠCH LÊN
                    aiText.innerHTML = "⚠️ " + (data.ai_status || "Phát hiện thông số bất thường!");
                    aiText.style.color = "#c62828";
                    aiText.style.fontWeight = "bold";
                } else {
                    aiCard.className = "ai-card safe";
                    aiText.innerHTML = "✅ " + (data.ai_status || "Môi trường ổn định.");
                    aiText.style.color = "#2e7d32";
                    aiText.style.fontWeight = "normal";
                }
            }
        }
        
        if (data.page === "device_status") {
            const relay = relayList.find(r => parseInt(r.gpio) === parseInt(data.gpio));
            if (relay) {
                relay.state = (data.status === "ON");
                relay.isWaiting = false; 
                localStorage.setItem('relays', JSON.stringify(relayList)); 
                renderRelays(); 
            }
        }

        if (data.page === "cloud_update") {
            const cloudName = data.name.toLowerCase().replace(/\s/g, "");
            const relay = relayList.find(r => r.name.toLowerCase().replace(/\s/g, "") === cloudName);
            if (relay) {
                relay.state = (data.status === "ON");
                relay.isWaiting = false;
                localStorage.setItem('relays', JSON.stringify(relayList));
                renderRelays();
                Send_Data(JSON.stringify({
                    page: "device",
                    value: { name: relay.name, status: data.status, gpio: relay.gpio }
                }));
            }
        }
    } catch (e) {}
}

// ==================== NAVIGATION TỐI ƯU HÓA ====================
let relayList = JSON.parse(localStorage.getItem('relays')) || [];
let deleteTarget = null;

function showSection(id, event) {
    document.querySelectorAll('.panel-section').forEach(sec => sec.style.display = 'none');
    document.getElementById(id).style.display = 'block';
    document.querySelectorAll('.nav-item').forEach(btn => btn.classList.remove('active'));
    event.currentTarget.classList.add('active');
}

// ==================== GAUGES RENDER (KHUNG RỘNG) ====================
var gaugeTemp, gaugeHumi;
window.onload = function () {
    gaugeTemp = new JustGage({
        id: "gauge_temp", value: 0, min: -10, max: 60,
        donut: true, pointer: true, gaugeWidthScale: 0.4, gaugeColor: "#e2e8f0",
        levelColors: ["#3b82f6", "#10b981", "#f59e0b", "#ef4444"], pointerOptions: { color: '#0f172a' }
    });
    gaugeHumi = new JustGage({
        id: "gauge_humi", value: 0, min: 0, max: 100,
        donut: true, pointer: true, gaugeWidthScale: 0.4, gaugeColor: "#e2e8f0",
        levelColors: ["#93c5fd", "#3b82f6", "#1e40af"], pointerOptions: { color: '#0f172a' }
    });
    renderRelays(); 
};

// ==================== XỬ LÝ THIẾT BỊ ====================
function openAddRelayDialog() { document.getElementById('addRelayDialog').style.display = 'flex'; }
function closeAddRelayDialog() { document.getElementById('addRelayDialog').style.display = 'none'; }

function saveRelay() {
    const name = document.getElementById('relayName').value.trim();
    const gpio = document.getElementById('relayGPIO').value.trim();
    if (!name || !gpio) return;
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
        card.className = `hw-card ${r.state ? 'active-card' : ''}`; 
        let isFan = r.name.toLowerCase().includes("quạt");
        let iconClass = isFan ? "fa-fan" : "fa-lightbulb";
        let spinClass = (isFan && r.state) ? "fa-spin" : ""; 
        let waitingStyle = r.isWaiting ? 'style="pointer-events: none;"' : '';

        // Tạo thẻ Hardware siêu ngầu
        card.innerHTML = `
            <div class="hw-info">
              <i class="fa-solid ${iconClass} ${spinClass} hw-icon"></i>
              <div class="hw-text">
                <h3>${r.name}</h3>
                <p>Địa chỉ Hardware: GPIO ${r.gpio}</p>
              </div>
            </div>
            <div class="hw-action">
              <label class="switch" ${waitingStyle}>
                <input type="checkbox" ${r.state ? 'checked' : ''} onchange="toggleRelay(${r.id})">
                <span class="slider ${r.isWaiting ? 'waiting' : ''}"></span>
              </label>
              <button class="delete-btn" onclick="showDeleteDialog(${r.id})"><i class="fa-solid fa-trash-can"></i></button>
            </div>
        `;
        container.appendChild(card);
    });
}

function toggleRelay(id) {
    const relay = relayList.find(r => r.id === id);
    if (relay && !relay.isWaiting) {
        const reqStat = relay.state ? "OFF" : "ON";
        relay.isWaiting = true; 
        renderRelays();
        Send_Data(JSON.stringify({
            page: "device",
            value: { name: relay.name, status: reqStat, gpio: relay.gpio }
        }));
        setTimeout(() => { if (relay.isWaiting) { relay.isWaiting = false; renderRelays(); } }, 3000);
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

// ==================== CÀI ĐẶT MẠNG MQTT ====================
document.getElementById("settingsForm").addEventListener("submit", function (e) {
    e.preventDefault();
    Send_Data(JSON.stringify({
        page: "setting",
        value: {
            ssid: document.getElementById("ssid").value.trim(),
            password: document.getElementById("password").value.trim(),
            server: document.getElementById("mqtt_server").value.trim(),
            port: document.getElementById("mqtt_port").value.trim()
        }
    }));
    alert("Dữ liệu mạng đã được đẩy vào ROM của ESP32 thành công!");
});