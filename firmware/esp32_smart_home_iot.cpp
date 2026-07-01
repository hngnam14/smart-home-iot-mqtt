#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Sua theo WiFi va IP may tinh dang chay backend Node.js.
const char* WIFI_SSID = "TEN_WIFI";
const char* WIFI_PASSWORD = "MAT_KHAU_WIFI";
const char* SERVER_HOST = "192.168.1.10";
const uint16_t SERVER_PORT = 3000;
const char* SERVER_PATH = "/mqtt";

// Chan cam bien.
#define DHT_PIN 4
#define DHT_TYPE DHT11
#define GAS_PIN 34
#define PIR_PIN 27
#define LDR_PIN 35

// Chan relay dieu khien thiet bi.
#define RELAY_LIGHT 18
#define RELAY_FAN 19
#define RELAY_LOCK 21
#define RELAY_PUMP 22

DHT dht(DHT_PIN, DHT_TYPE);
WebSocketsClient webSocket;

unsigned long lastTelemetryMs = 0;
const unsigned long TELEMETRY_INTERVAL_MS = 2500;

void setRelay(uint8_t pin, bool on) {
  // Nhieu module relay kich muc LOW. Neu relay cua ban kich muc HIGH,
  // doi LOW/HIGH o hai dong duoi.
  digitalWrite(pin, on ? LOW : HIGH);
}

void publishJson(const char* topic, JsonDocument& payload) {
  StaticJsonDocument<384> packet;
  packet["type"] = "publish";
  packet["topic"] = topic;
  packet["payload"] = payload.as<JsonObject>();

  String message;
  serializeJson(packet, message);
  webSocket.sendTXT(message);
}

void subscribeTopic(const char* topic) {
  StaticJsonDocument<128> packet;
  packet["type"] = "subscribe";
  packet["topic"] = topic;

  String message;
  serializeJson(packet, message);
  webSocket.sendTXT(message);
}

void publishDeviceState(const char* id, const char* name, const char* icon, const char* state) {
  char topic[80];
  snprintf(topic, sizeof(topic), "home/device/%s/state", id);

  StaticJsonDocument<192> payload;
  payload["id"] = id;
  payload["name"] = name;
  payload["icon"] = icon;
  payload["state"] = state;
  payload["ts"] = millis();

  publishJson(topic, payload);
}

void applyDeviceCommand(const char* deviceId, const char* state) {
  if (strcmp(deviceId, "living_light") == 0) {
    setRelay(RELAY_LIGHT, strcmp(state, "ON") == 0);
    publishDeviceState("living_light", "Den phong khach", "lightbulb", state);
  } else if (strcmp(deviceId, "fan") == 0) {
    setRelay(RELAY_FAN, strcmp(state, "ON") == 0);
    publishDeviceState("fan", "Quat thong minh", "fan", state);
  } else if (strcmp(deviceId, "door_lock") == 0) {
    setRelay(RELAY_LOCK, strcmp(state, "UNLOCKED") == 0);
    publishDeviceState("door_lock", "Khoa cua", "lock", state);
  } else if (strcmp(deviceId, "pump") == 0) {
    setRelay(RELAY_PUMP, strcmp(state, "ON") == 0);
    publishDeviceState("pump", "May bom nuoc", "droplets", state);
  }
}

void handleWebMqttMessage(const String& text) {
  StaticJsonDocument<768> packet;
  DeserializationError error = deserializeJson(packet, text);
  if (error) return;

  const char* type = packet["type"] | "";
  const char* topic = packet["topic"] | "";
  if (strcmp(type, "message") != 0) return;

  char deviceId[32];
  if (sscanf(topic, "home/device/%31[^/]/set", deviceId) == 1) {
    const char* state = packet["payload"]["state"] | "";
    applyDeviceCommand(deviceId, state);
  }
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("[WS] connected");
      subscribeTopic("home/device/+/set");
      publishDeviceState("living_light", "Den phong khach", "lightbulb", "OFF");
      publishDeviceState("fan", "Quat thong minh", "fan", "OFF");
      publishDeviceState("door_lock", "Khoa cua", "lock", "LOCKED");
      publishDeviceState("pump", "May bom nuoc", "droplets", "OFF");
      break;

    case WStype_TEXT:
      {
        String text;
        text.reserve(length);
        for (size_t i = 0; i < length; i += 1) {
          text += (char)payload[i];
        }
        handleWebMqttMessage(text);
      }
      break;

    case WStype_DISCONNECTED:
      Serial.println("[WS] disconnected");
      break;

    default:
      break;
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("[WiFi] connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());
}

void publishTelemetry() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature)) temperature = 0;
  if (isnan(humidity)) humidity = 0;

  int gasRaw = analogRead(GAS_PIN);
  int ldrRaw = analogRead(LDR_PIN);
  bool motion = digitalRead(PIR_PIN) == HIGH;

  int gasPpm = map(gasRaw, 0, 4095, 0, 1000);
  int lightPercent = map(ldrRaw, 0, 4095, 100, 0);
  lightPercent = constrain(lightPercent, 0, 100);

  StaticJsonDocument<256> payload;
  payload["temperature"] = roundf(temperature * 10) / 10.0;
  payload["humidity"] = roundf(humidity * 10) / 10.0;
  payload["gas"] = gasPpm;
  payload["motion"] = motion;
  payload["light"] = lightPercent;
  payload["ts"] = millis();

  publishJson("home/sensor/telemetry", payload);

  if (gasPpm > 650 || temperature > 40) {
    StaticJsonDocument<256> alert;
    alert["level"] = "warning";
    alert["message"] = "Canh bao nhiet do hoac khi gas cao";
    alert["ts"] = millis();
    publishJson("home/alert", alert);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_LIGHT, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_LOCK, OUTPUT);
  pinMode(RELAY_PUMP, OUTPUT);

  setRelay(RELAY_LIGHT, false);
  setRelay(RELAY_FAN, false);
  setRelay(RELAY_LOCK, false);
  setRelay(RELAY_PUMP, false);

  dht.begin();
  connectWiFi();

  webSocket.begin(SERVER_HOST, SERVER_PORT, SERVER_PATH);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(2000);
}

void loop() {
  webSocket.loop();

  if (millis() - lastTelemetryMs >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = millis();
    publishTelemetry();
  }
}
