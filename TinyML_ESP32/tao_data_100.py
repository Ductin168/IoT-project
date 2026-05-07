import pandas as pd
import numpy as np

# 1. Số lượng mẫu: 5000 dòng là "Con số Vàng" (Đủ thông minh, không bị học vẹt)
n_samples = 3000

# 2. Random liên tục số thực (float) để ra các giá trị lẻ như thực tế
np.random.seed(42)
temps = np.round(np.random.uniform(10.0, 50.0, n_samples), 2)
humidities = np.round(np.random.uniform(30.0, 100.0, n_samples), 2)

# =================================================================
# 3. BÍ QUYẾT ĐIỂM A+: BƠM NHIỄU (NOISE) VÀO DỮ LIỆU
# =================================================================
# Tạo ra một mảng sai số ngẫu nhiên (nhiễu Gauss) mô phỏng cảm biến thực tế
# Nhiệt độ nhiễu lệch khoảng 0.8 độ. Độ ẩm nhiễu lệch khoảng 2.0%
noise_temp = np.random.normal(0, 0.8, n_samples)
noise_humi = np.random.normal(0, 2.0, n_samples)

# Cộng nhiễu vào trước khi gắn nhãn. (Lưu ý: Mốc nhiệt đã đổi thành 33.0 khớp với C++)
# Việc này khiến vùng giáp ranh 32.5 - 33.5 độ bị "lập lờ", 
# ép AI phải nhả ra số thực tế (VD: 0.68, 0.85) thay vì nhắm mắt phang 1.0000
temp_labels = np.where((temps + noise_temp) > 33.0, 1, 0)
humi_labels = np.where((humidities + noise_humi) > 70.0, 1, 0)

# 4. Gom vào bảng (Bây giờ có 4 cột)
df = pd.DataFrame({
    'temp': temps,
    'humidity': humidities,
    'temp_label': temp_labels,
    'humi_label': humi_labels
})

# Ép format để luôn hiện đúng 2 chữ số thập phân (VD: 34.65, 30.20)
df['temp'] = df['temp'].map('{:.2f}'.format)
df['humidity'] = df['humidity'].map('{:.2f}'.format)

# 5. Xuất ra file
# Lưu ý: header=False nghĩa là file CSV in ra sẽ không có dòng chữ tên cột, 
# chỉ có toàn số là số (giúp file code train AI đọc thẳng vào dễ dàng hơn)
df.to_csv('sensor_data_2labels.csv', index=False, header=False)

print("🚀 Đã tạo thành công Data 5000 dòng có BƠM NHIỄU CHỐNG 1.0000!")
print("-" * 50)
print(f"Tổng số dòng: {len(df)}")
print(f"🔥 Số mẫu NÓNG (> 33°C): {len(df[df['temp_label'] == 1])}")
print(f"💧 Số mẫu ẨM CAO (> 70%): {len(df[df['humi_label'] == 1])}")
print(f"⚠️ Số mẫu NGUY HIỂM KÉP (Cả 2): {len(df[(df['temp_label'] == 1) & (df['humi_label'] == 1)])}")

# Hiển thị 5 dòng đầu tiên để bác xem thử
print("\nDemo 5 dòng đầu tiên trong file:")
print(df.head())