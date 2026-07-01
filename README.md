# Smart Home IoT Web MQTT

Du an mau ve nha thong minh IoT gom frontend, backend va Web MQTT qua WebSocket. Backend tu tao broker publish/subscribe theo topic kieu MQTT, web dashboard ket noi truc tiep bang `ws://localhost:3000/mqtt`.

## Chuc nang

- Hien thi du lieu cam bien: nhiet do, do am, khi gas, chuyen dong, anh sang.
- Dieu khien thiet bi: den phong khach, quat, khoa cua, may bom nuoc.
- Truyen du lieu thoi gian thuc bang topic MQTT qua WebSocket.
- Backend co API lay trang thai ban dau.
- Co canh bao khi nhiet do hoac khi gas cao.

## Cau truc thu muc

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

## Cai dat va chay

Can cai Node.js 18 tro len. Project nay khong can cai them thu vien ngoai.

```bash
cd smart-home-iot-mqtt
npm start
```

Mo trinh duyet:

```text
http://localhost:3000
```

Web MQTT endpoint:

```text
ws://localhost:3000/mqtt
```

## Web MQTT topics

| Muc dich | Topic | Payload mau |
|---|---|---|
| Du lieu cam bien | `home/sensor/telemetry` | `{"temperature":29.5,"humidity":65,"gas":150,"motion":false,"light":52}` |
| Gui lenh dieu khien | `home/device/{deviceId}/set` | `{"state":"ON"}` |
| Nhan trang thai thiet bi | `home/device/{deviceId}/state` | `{"id":"fan","name":"Quat thong minh","state":"ON"}` |
| Canh bao | `home/alert` | `{"level":"warning","message":"Canh bao nhiet do hoac khi gas cao"}` |

## Device ID

| Device ID | Thiet bi | Trang thai |
|---|---|---|
| `living_light` | Den phong khach | `ON` / `OFF` |
| `fan` | Quat thong minh | `ON` / `OFF` |
| `door_lock` | Khoa cua | `LOCKED` / `UNLOCKED` |
| `pump` | May bom nuoc | `ON` / `OFF` |

## Giai thich nhanh

- `backend/server.js`: tao HTTP server, tao WebSocket broker tai `/mqtt`, mo phong cam bien va xu ly lenh dieu khien.
- `frontend/app.js`: ket noi Web MQTT, subscribe cac topic, cap nhat giao dien va publish lenh dieu khien.
- `frontend/index.html` va `frontend/style.css`: giao dien dashboard nha thong minh.

## Cach demo voi giao vien

1. Chay `npm start`.
2. Mo `http://localhost:3000`.
3. Bam bat/tat den, quat, bom nuoc hoac khoa cua.
4. Giai thich: Web publish lenh vao topic `home/device/{id}/set`, backend nhan lenh, cap nhat trang thai va publish lai topic `home/device/{id}/state`.
5. Cam bien duoc backend mo phong va gui lien tuc qua topic `home/sensor/telemetry`.

## Chay voi ESP32 cam bien that

File firmware nam tai:

```text
firmware/esp32_smart_home_iot.cpp
```

Thu vien can cai trong Arduino IDE hoac PlatformIO:

| Thu vien | Muc dich |
|---|---|
| `DHT sensor library` | Doc DHT11/DHT22 |
| `ArduinoJson` | Tao va doc goi JSON |
| `WebSockets` by Markus Sattler | Ket noi ESP32 den `ws://server:3000/mqtt` |

So do chan mac dinh:

| Thiet bi | Chan ESP32 |
|---|---|
| DHT11/DHT22 data | GPIO 4 |
| MQ-2 analog | GPIO 34 |
| PIR motion | GPIO 27 |
| LDR analog | GPIO 35 |
| Relay den | GPIO 18 |
| Relay quat | GPIO 19 |
| Relay khoa cua | GPIO 21 |
| Relay bom nuoc | GPIO 22 |

Can sua 3 dong trong file C++:

```cpp
const char* WIFI_SSID = "TEN_WIFI";
const char* WIFI_PASSWORD = "MAT_KHAU_WIFI";
const char* SERVER_HOST = "192.168.1.10";
```

`SERVER_HOST` la IP may tinh dang chay lenh `npm start`. ESP32 va may tinh phai cung mang WiFi.
