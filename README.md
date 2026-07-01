# Smart Home IoT Web MQTT

Dự án mẫu về nhà thông minh IoT gồm frontend, backend và Web MQTT qua WebSocket. Backend tự tạo broker publish/subscribe theo topic kiểu MQTT, web dashboard kết nối trực tiếp bằng `ws://localhost:3000/mqtt`.

## Chức năng

- Hiển thị dữ liệu cảm biến: nhiệt độ, độ ẩm, khí gas, chuyển động, ánh sáng.
- Điều khiển thiết bị: đèn phòng khách, quạt, khóa cửa, máy bơm nước.
- Truyền dữ liệu thời gian thực bằng topic MQTT qua WebSocket.
- Backend có API lấy trạng thái ban đầu.
- Có cảnh báo khi nhiệt độ hoặc khí gas cao.

## Cấu trúc thư mục

```text
smart-home-iot-mqtt/
├── backend/
│   └── server.js
├── firmware/
│   └── esp32_smart_home_iot.cpp
├── frontend/
│   ├── index.html
│   ├── style.css
│   └── app.js
├── package.json
└── README.md
```

## Cài đặt và chạy

Cần cài Node.js 18 trở lên. Project này không cần cài thêm thư viện ngoài.

```bash
cd smart-home-iot-mqtt
npm start
```

Mở trình duyệt:

```text
http://localhost:3000
```

Web MQTT endpoint:

```text
ws://localhost:3000/mqtt
```

## Web MQTT Topics

| Mục đích | Topic | Payload mẫu |
|---|---|---|
| Dữ liệu cảm biến | `home/sensor/telemetry` | `{"temperature":29.5,"humidity":65,"gas":150,"motion":false,"light":52}` |
| Gửi lệnh điều khiển | `home/device/{deviceId}/set` | `{"state":"ON"}` |
| Nhận trạng thái thiết bị | `home/device/{deviceId}/state` | `{"id":"fan","name":"Quạt thông minh","state":"ON"}` |
| Cảnh báo | `home/alert` | `{"level":"warning","message":"Cảnh báo nhiệt độ hoặc khí gas cao"}` |

## Device ID

| Device ID | Thiết bị | Trạng thái |
|---|---|---|
| `living_light` | Đèn phòng khách | `ON` / `OFF` |
| `fan` | Quạt thông minh | `ON` / `OFF` |
| `door_lock` | Khóa cửa | `LOCKED` / `UNLOCKED` |
| `pump` | Máy bơm nước | `ON` / `OFF` |

## Giải thích nhanh

- `backend/server.js`: tạo HTTP server, tạo WebSocket broker tại `/mqtt`, mô phỏng cảm biến và xử lý lệnh điều khiển.
- `frontend/app.js`: kết nối Web MQTT, subscribe các topic, cập nhật giao diện và publish lệnh điều khiển.
- `frontend/index.html` và `frontend/style.css`: giao diện dashboard nhà thông minh.

## Cách demo 

1. Chạy `npm start`.
2. Mở `http://localhost:3000`.
3. Bấm bật/tắt đèn, quạt, bơm nước hoặc khóa cửa.
4. Giải thích: Web publish lệnh vào topic `home/device/{id}/set`, backend nhận lệnh, cập nhật trạng thái và publish lại topic `home/device/{id}/state`.
5. Cảm biến được backend mô phỏng và gửi liên tục qua topic `home/sensor/telemetry`.

## Chạy với ESP32 và cảm biến thật

File firmware nằm tại:

```text
firmware/esp32_smart_home_iot.cpp
```

Thư viện cần cài trong Arduino IDE hoặc PlatformIO:

| Thư viện | Mục đích |
|---|---|
| `DHT sensor library` | Đọc DHT11/DHT22 |
| `ArduinoJson` | Tạo và đọc gói JSON |
| `WebSockets` by Markus Sattler | Kết nối ESP32 đến `ws://server:3000/mqtt` |

Sơ đồ chân mặc định:

| Thiết bị | Chân ESP32 |
|---|---|
| DHT11/DHT22 data | GPIO 4 |
| MQ-2 analog | GPIO 34 |
| PIR motion | GPIO 27 |
| LDR analog | GPIO 35 |
| Relay đèn | GPIO 18 |
| Relay quạt | GPIO 19 |
| Relay khóa cửa | GPIO 21 |
| Relay bơm nước | GPIO 22 |

Cần sửa 3 dòng trong file C++:

```cpp
const char* WIFI_SSID = "TEN_WIFI";
const char* WIFI_PASSWORD = "MAT_KHAU_WIFI";
const char* SERVER_HOST = "192.168.1.10";
```

`SERVER_HOST` là IP máy tính đang chạy lệnh `npm start`. ESP32 và máy tính phải cùng mạng WiFi.
