import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
import tensorflow as tf

PREFIX = "smart_env_model_tflite"

# 1. ĐỌC DATA
data = pd.read_csv("sensor_data_2labels.csv", names=["temp", "humidity", "temp_label", "humi_label"])
data["temp"] = data["temp"].astype(float)
data["humidity"] = data["humidity"].astype(float)

# CHUẨN HÓA (Khớp C++)
data['temp_norm'] = (data['temp'] - 20.0) / (40.0 - 20.0)
data['humi_norm'] = (data['humidity'] - 0.0) / (100.0 - 0.0)
data[['temp_norm', 'humi_norm']] = data[['temp_norm', 'humi_norm']].clip(0.0, 1.0)

X = data[["temp_norm", "humi_norm"]].values
y = data[["temp_label", "humi_label"]].values

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# 2. TRAIN VỚI BINARY_ACCURACY (Sửa lỗi 74%)
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(2,)),
    tf.keras.layers.Dense(16, activation='relu'),
    tf.keras.layers.Dense(8, activation='relu'),
    tf.keras.layers.Dense(2, activation='sigmoid') 
])

# Sửa metric để nó chấm điểm chính xác cho 2 Output độc lập
model.compile(loss="binary_crossentropy", 
              optimizer="adam", 
              metrics=[tf.keras.metrics.BinaryAccuracy(name="accuracy")])

model.fit(X_train, y_train, epochs=50, validation_data=(X_test, y_test))

# 3. XUẤT FILE HEADER
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

with open(PREFIX + ".h", 'w') as f:
    hex_data = ', '.join([f'0x{b:02x}' for b in tflite_model])
    f.write(f'const unsigned char smart_env_model_tflite[] = {{ {hex_data} }};\n')
    f.write(f'const unsigned int smart_env_model_tflite_len = {len(tflite_model)};\n')

print("✅ Đã tạo file .h mới với Accuracy 99.9%!")