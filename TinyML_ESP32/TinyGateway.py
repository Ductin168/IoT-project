import paho.mqtt.client as mqtt
import json
import time

# --- Cấu hình Cloud CoreIoT ---
THINGSBOARD_HOST = 'app.coreiot.io'
ACCESS_TOKEN = 'xzNrOlHS28zJ8DtjOyCe' 

# =================================================================
# 1. HÀM XỬ LÝ LỆNH TỪ CLOUD (RPC)
# =================================================================
def on_cloud_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        device_name = payload.get("device")
        command_data = payload.get("data", {})
        req_id = command_data.get("id", "1")

        print(f"\n⚡ [CORE IOT -> LOCAL] Lệnh điều khiển {device_name}: {command_data}")
        
        # Bắn lệnh xuống Local Broker cho ESP32 nhận
        local_topic = f"v1/devices/me/rpc/request/{req_id}"
        local_client.publish(local_topic, json.dumps(command_data))
        
        # 👉 BÍ QUYẾT: Gửi RPC Response ngay lập tức để Switch trên Cloud không bị Timeout
        reply = {"device": device_name, "id": req_id, "data": {"success": True}}
        cloud_client.publish('v1/gateway/rpc', json.dumps(reply))
            
    except Exception as e:
        print(f"❌ Lỗi xử lý lệnh Cloud: {e}")

# =================================================================
# 2. HÀM XỬ LÝ DATA TỪ LOCAL LÊN
# =================================================================
def on_local_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        
        # Báo danh online
        cloud_client.publish('v1/gateway/connect', json.dumps({"device": "Mach_A_Node"}))
        cloud_client.publish('v1/gateway/connect', json.dumps({"device": "Mach_B_Node"}))

        # A. NHIỆT ĐỘ / ĐỘ ẨM
        if msg.topic == "v1/devices/me/telemetry":
            t = payload.get("temperature", 0.0)
            h = payload.get("humidity", 0.0)
            telemetry = {
                "Mach_A_Node": [{"ts": int(time.time() * 1000), "values": {"temperature": t, "humidity": h}}],
                "Mach_B_Node": [{"ts": int(time.time() * 1000), "values": {"status": "Online"}}]
            }
            cloud_client.publish('v1/gateway/telemetry', json.dumps(telemetry))

        # B. NÚT BẤM TỪ WEB LOCAL / NÚT CỨNG
        elif msg.topic == "v1/devices/me/attributes":
            # 👉 ÉP TOÀN BỘ TRẠNG THÁI VÀO Mach_A_Node ĐỂ ĐỒNG BỘ DASHBOARD
            attr_update = {"Mach_A_Node": payload}
            cloud_client.publish('v1/gateway/attributes', json.dumps(attr_update))
            
            # Gửi kèm Telemetry để đảm bảo Widget Switch nhận tín hiệu mọi lúc
            telemetry_backup = {
                "Mach_A_Node": [{"ts": int(time.time() * 1000), "values": payload}]
            }
            cloud_client.publish('v1/gateway/telemetry', json.dumps(telemetry_backup))
            
            print(f"☁️ Cloud: Đã đồng bộ trạng thái {payload} lên Mach_A_Node thành công!")

    except Exception as e:
        print(f"❌ Lỗi xử lý dữ liệu cục bộ: {e}")

# =================================================================
# 3. KHỞI TẠO VÀ KẾT NỐI
# =================================================================
cloud_client = mqtt.Client()
cloud_client.username_pw_set(ACCESS_TOKEN)
cloud_client.on_message = on_cloud_message
cloud_client.connect(THINGSBOARD_HOST, 1883, 60)
cloud_client.subscribe("v1/gateway/rpc") 
cloud_client.loop_start() 

local_client = mqtt.Client()
local_client.on_message = on_local_message
local_client.connect("127.0.0.1", 1883, 60) 
local_client.subscribe("v1/devices/me/telemetry") 
local_client.subscribe("v1/devices/me/attributes") 

print("=" * 50)
print("🚀 GATEWAY SYNC 2-WAY ĐÃ KHỞI CHẠY!")
print("📡 Sẵn sàng đồng bộ hoàn hảo giữa Web Local & CoreIoT")
print("=" * 50)

try:
    local_client.loop_forever() 
except KeyboardInterrupt:
    print('\n⚠️ Đang đóng Gateway...')
finally:
    cloud_client.loop_stop()