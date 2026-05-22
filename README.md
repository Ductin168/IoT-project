# Enhanced IoT Monitoring and Control System

Hệ thống giám sát và điều khiển môi trường thông minh phân tán (**Edge-to-Cloud**), kết hợp tối ưu giữa **Hệ điều hành thời gian thực (FreeRTOS)**, **Trí tuệ nhân tạo biên (TinyML)** và kiến trúc **Edge Gateway** tin cậy. Dự án được phát triển nhằm cải tiến và nâng cấp toàn diện (>50%) so với nền tảng YoloUNO gốc.

## 🚀 Tính Năng Cốt Lõi

- **Kiến trúc mạng phân tán (Hybrid Topology):** Giao tiếp nội bộ tốc độ cao qua **ESP-NOW** tầng MAC, đồng bộ diện rộng lên cloud qua **MQTT Gateway**.
- **Quản lý đa nhiệm chuẩn RTOS:** Loại bỏ 100% biến toàn cục. Đóng gói dữ liệu và đồng bộ an toàn qua FreeRTOS **Queue** và **Binary Semaphore**, triệt tiêu hoàn toàn lỗi *Race Condition*.
- **Trí tuệ nhân tạo tại biên (TinyML):** Tích hợp mô hình mạng nơ-ron nhúng qua thư viện **TensorFlow Lite Micro** trực tiếp trên chip ESP32-S3. Suy luận thời gian thực nhằm chẩn đoán và phân loại trạng thái môi trường thông minh.
- **Local Web Server & Điều khiển Real-time:** Khởi tạo Web Server bất đối xứng bằng **LittleFS** và truyền dữ liệu song công **WebSocket**, cập nhật Dashboard local tức thời (<10ms).
- **Cầu nối dữ liệu thông minh (Edge Gateway):** Hệ thống trạm trung chuyển Python trung gian giúp phân loại telemetry, phản hồi nhanh lệnh **RPC** hai chiều và lưu trữ thuộc tính thiết bị.
- **Tự động hóa thông minh:** Tích hợp **Rule Engine** trên Cloud CoreIOT kết hợp lưu trữ bền vững trạng thái thiết bị chấp hành qua **Preferences (NVS)**, tự phục hồi trạng thái khi mất điện.

## 📂 Cấu Trúc Thư Mục

```text
IoT-project/
├── ESP32S_Sensor_task/       # Firmware cho Sensor Node (Mạch A - ESP32)
│   ├── src/                  # Đọc DHT11, xử lý nút nhấn, phát ESP-NOW
│   └── platformio.ini        # Cấu hình build cho ESP32
├── YoloUNO_PlatformIO/       # Firmware cho Gateway Node (Mạch B - Yolo Uno)
│   ├── src/                  # RTOS Tasks, TinyML Inference, WebServer, WebSocket
│   ├── data/                 # Giao diện Dashboard (HTML/CSS/JS) lưu tại LittleFS
│   └── platformio.ini        # Cấu hình build cho Yolo Uno (ESP32-S3)
└── TinyML_ESP32/             # Local Edge Gateway (PC) & Scripts
    ├── TinyMQTT.py           # Local MQTT Broker (Port 1883)
    └── TinyGateway.py        # Bridge đồng bộ dữ liệu (Telemetry & Cloud RPC)


🛠️ Hướng Dẫn Cài Đặt & Triển Khai
1. Triển Khai Firmware (PlatformIO)
Sử dụng Visual Studio Code với extension PlatformIO:

Với Mạch A (ESP32S_Sensor_task): Build & Upload firmware lên NodeMCU-32S.

Với Mạch B (YoloUNO_PlatformIO): - Chạy lệnh PlatformIO: Upload Filesystem Image để nạp giao diện Web vào LittleFS.

Build & Upload firmware chính lên bo mạch Yolo Uno (ESP32-S3).

2. Khởi Chạy Local Edge Gateway (Trên PC)
Đảm bảo máy tính nằm chung mạng LAN với Mạch B:

Di chuyển vào thư mục gateway: cd TinyML_ESP32

Khởi chạy Broker: python TinyMQTT.py

Khởi chạy Script cầu nối: python TinyGateway.py

3. Vận Hành Hệ Thống
Cấu hình: Khi cấp nguồn, nếu thiết bị chưa kết nối WiFi, nó sẽ phát AP. Truy cập 192.168.4.1 để nhập SSID, mật khẩu và cấu hình IP Gateway.

Giám sát: Truy cập IP đã cấu hình trên trình duyệt để sử dụng Dashboard. Hệ thống sẽ tự động đồng bộ dữ liệu lên CoreIOT Cloud.

🕹️ Chế Độ Kiểm Thử (Test Mode)
Hệ thống tích hợp cơ chế kiểm thử tại Mạch A:

Nhấn giữ 10s: Bật/Tắt chế độ giả lập dữ liệu (Test Mode).

Nhấn nhả nhanh (Click): Tăng giá trị mô phỏng (+1°C/Nhiệt độ, +5%/Độ ẩm) để kiểm chứng thuật toán TinyML và các thiết bị cảnh báo.

👥 Nhóm Thực Hiện
Hồ Đức Tín (2213486) - Thiết kế phần cứng, RTOS, triển khai TinyML.

Nguyễn Trần Như Phước (2212719) - Web Server, WebSocket UI, Python Local Gateway.

Trương Phan Hoàng Vỹ (2214062) - Kết nối CoreIOT Cloud, Dashboard & Rule Chains.

Giáo viên hướng dẫn: PGS.TS. Lê Trọng Nhân
