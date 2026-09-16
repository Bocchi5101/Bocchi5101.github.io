/* IoT lab example: NodeMCU ESP8266 + DHT11 + external LED + button.
   Install MQTT by Joel Gaehwiler, DHT sensor library by Adafruit,
   and Adafruit Unified Sensor. Select the actual ESP8266 board.
   Set credentials below. This sketch has NOT been hardware-tested.
   D1=GPIO5 LED (active HIGH), D2=GPIO4 DHT DATA, D5=GPIO14 button.
   If the actual sensor is DHT22, change DHTTYPE. Other sensors need
   their own library and read code. All IO signals must be 3.3V.
*/
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <DHT.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_HOST = "YOUR_CLUSTER.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USERNAME = "YOUR_MQTT_USERNAME";
const char* MQTT_PASSWORD = "YOUR_MQTT_PASSWORD";

// Use the same topics in React. Add your student ID if sharing a broker.
const char* TOPIC_TEMP = "lab/sensor/temp";
const char* TOPIC_HUMIDITY = "lab/sensor/humidity";
const char* TOPIC_STATUS = "lab/led/status";
const char* TOPIC_CONTROL = "lab/led/control";

const uint8_t LED_PIN = 5;      // NodeMCU D1 (external LED)
const uint8_t DHT_PIN = 4;      // NodeMCU D2
const uint8_t BUTTON_PIN = 14;  // NodeMCU D5, button to GND
const bool LED_ACTIVE_LOW = false;
#define DHTTYPE DHT11

WiFiClientSecure networkClient;
MQTTClient mqtt(256);
DHT dht(DHT_PIN, DHTTYPE);
String clientId;
bool ledOn = false;
bool statusPending = true;
bool previousRawButton = HIGH;
bool stableButton = HIGH;
bool wifiWasConnected = false;
unsigned long buttonChangedAt = 0;
unsigned long lastSensorAt = 0;
unsigned long lastWifiAttemptAt = 0;
unsigned long lastMqttAttemptAt = 0;
const unsigned long SENSOR_INTERVAL = 5000;
const unsigned long DEBOUNCE_MS = 40;

void setLed(bool on) {
  ledOn = on;
  digitalWrite(LED_PIN, (on != LED_ACTIVE_LOW) ? HIGH : LOW);
  statusPending = true;
  Serial.print("LED: ");
  Serial.println(on ? "ON" : "OFF");
}

void messageHandler(String &topic, String &payload) {
  Serial.println(topic + " -> " + payload);
  if (topic != TOPIC_CONTROL) return;
  if (payload == "ON") setLed(true);
  else if (payload == "OFF") setLed(false);
  // Publish later in loop(), not inside this callback.
}

void readButton() {
  bool raw = digitalRead(BUTTON_PIN);
  unsigned long now = millis();
  if (raw != previousRawButton) {
    previousRawButton = raw;
    buttonChangedAt = now;
  }
  if (now - buttonChangedAt >= DEBOUNCE_MS && raw != stableButton) {
    stableButton = raw;
    if (stableButton == LOW) setLed(!ledOn);
  }
}

void maintainConnections() {
  unsigned long now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiWasConnected) {
      wifiWasConnected = false;
      networkClient.stop();
      Serial.println("Wi-Fi lost");
    }
    if (now - lastWifiAttemptAt >= 10000UL) {
      lastWifiAttemptAt = now;
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      Serial.println("Retrying Wi-Fi...");
    }
    return;
  }
  if (!wifiWasConnected) {
    wifiWasConnected = true;
    Serial.print("Wi-Fi IP: ");
    Serial.println(WiFi.localIP());
    lastMqttAttemptAt = now - 5000UL;
  }
  if (mqtt.connected() || now - lastMqttAttemptAt < 5000UL) return;
  lastMqttAttemptAt = now;
  Serial.println("Connecting MQTT...");
  // connect() is synchronous: an individual connection attempt may pause
  // button polling. The retry interval avoids an endless retry while-loop.
  if (mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
    if (!mqtt.subscribe(TOPIC_CONTROL)) {
      Serial.println("Subscribe failed; check permissions");
      mqtt.disconnect();
      return;
    }
    statusPending = true;
    Serial.println("MQTT connected, control subscribed");
  } else {
    Serial.print("MQTT failed, error: ");
    Serial.println(mqtt.lastError());
  }
}

void publishSensors() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT read failed: check type, DATA pin and wiring");
    return;
  }
  Serial.print("Temperature C: "); Serial.print(temperature);
  Serial.print(" | Humidity %: "); Serial.println(humidity);
  if (mqtt.connected()) {
    bool tempSent = mqtt.publish(TOPIC_TEMP, String(temperature, 1));
    bool humiditySent = mqtt.publish(TOPIC_HUMIDITY, String(humidity, 1));
    if (!tempSent || !humiditySent) Serial.println("Sensor publish failed");
  }
}

void setup() {
  Serial.begin(115200);
  digitalWrite(LED_PIN, LED_ACTIVE_LOW ? HIGH : LOW);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  previousRawButton = stableButton = digitalRead(BUTTON_PIN);
  dht.begin();
  setLed(false);
  clientId = "esp8266_" + String(ESP.getChipId(), HEX);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWifiAttemptAt = millis();
  // Classroom shortcut matching the slides: encrypted transport but no
  // server certificate authentication. For production use a trusted CA.
  networkClient.setInsecure();
  mqtt.begin(MQTT_HOST, MQTT_PORT, networkClient);
  mqtt.onMessage(messageHandler);
}

void loop() {
  readButton();
  maintainConnections();
  if (mqtt.connected()) mqtt.loop();
  readButton();
  if (mqtt.connected() && statusPending) {
    if (mqtt.publish(TOPIC_STATUS, ledOn ? "ON" : "OFF", true, 0)) {
      statusPending = false;
    }
  }
  unsigned long now = millis();
  if (now - lastSensorAt >= SENSOR_INTERVAL) {
    lastSensorAt = now;
    publishSensors();
  }
  delay(10); // Briefly yield; do not use delay(5000) for sensor timing.
}
