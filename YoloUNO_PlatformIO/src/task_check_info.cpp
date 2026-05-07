#include "task_check_info.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

void Load_info_File()
{
  // TASK 3: File này giờ chỉ dùng để check xem file có tồn tại không.
  // Các biến toàn cục (WIFI_SSID, WIFI_PASS...) đã bị loại bỏ (chuyển sang biến cục bộ để tiết kiệm RAM).
  File file = LittleFS.open("/info.dat", "r");
  if (!file)
  {
    Serial.println("Chưa có file info.dat");
    return;
  }
  file.close(); // Nhớ đóng file để giải phóng vùng nhớ đệm (buffer)
}

// Hàm xóa file cấu hình (thường được gọi khi nhấn giữ nút BOOT để reset thiết bị về mặc định)
void Delete_info_File()
{
  if (LittleFS.exists("/info.dat"))
  {
    LittleFS.remove("/info.dat"); // Xóa file cấu hình trong bộ nhớ Flash
  }
  ESP.restart(); // Khởi động lại vi điều khiển ngay lập tức để áp dụng trạng thái mới
}

// Hàm lưu cấu hình mới (mạng WiFi và máy chủ CoreIOT) xuống bộ nhớ Flash dưới dạng file JSON
void Save_info_File(String wifi_ssid, String wifi_pass, String core_token, String core_server, String core_port)
{
  Serial.println("Đang lưu cấu hình mới...");
  Serial.println("SSID: " + wifi_ssid);
  Serial.println("PASS: " + wifi_pass);

  // Cấp phát vùng nhớ động JSON.
  // Giảm RAM xuống 1024 để chống tràn bộ nhớ (Stack Overflow) khi xử lý chuỗi
  DynamicJsonDocument doc(1024);
  doc["WIFI_SSID"] = wifi_ssid;
  doc["WIFI_PASS"] = wifi_pass;
  doc["CORE_IOT_TOKEN"] = core_token;
  doc["CORE_IOT_SERVER"] = core_server;
  doc["CORE_IOT_PORT"] = core_port;

  // Mở file ở chế độ ghi đè ("w" - write)
  File configFile = LittleFS.open("/info.dat", "w");
  if (!configFile)
  {
    Serial.println("❌ Lỗi: Không thể mở file config để ghi");
    return;
  }

  // Chuyển đổi đối tượng JSON thành chuỗi văn bản và ghi vào file
  serializeJson(doc, configFile);
  configFile.close();
  
  // Thêm dòng này để Monitor báo thành công
  Serial.println("✅ Lưu cấu hình vào LittleFS thành công!"); 
}

// Hàm đọc file cấu hình và kiểm tra xem có chứa thông tin WiFi hợp lệ không
// @return true nếu có cấu hình mạng hợp lệ, false nếu file rỗng hoặc không tồn tại
bool Check_info_File()
{
  String local_ssid = "";
  String local_pass = "";

  File file = LittleFS.open("/info.dat", "r"); // Mở file ở chế độ đọc ("r" - read)
  if (file) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, file);
    if (!error) { // Nếu parse (phân tích) JSON thành công
        local_ssid = doc["WIFI_SSID"].as<String>();
        local_pass = doc["WIFI_PASS"].as<String>();
    }
    file.close();
  }
  
  // Nếu dữ liệu chuỗi rỗng tức là chưa được cài đặt cấu hình mạng
  if (local_ssid == "" || local_pass == "") {
      return false; 
  }
  return true; 
}