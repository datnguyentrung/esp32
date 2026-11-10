#include <Arduino.h>

#include "commands/SerialCommands.h"
#include "config.h"
#include "mqtt/MqttManager.h"
#include "pir/PirSensor.h"
#include "provisioning/ProvisioningStore.h"
#include "wifi/WifiManager.h"

namespace {
WifiManager wifiManager;
MqttManager mqttManager;
ProvisioningStore provisioningStore;
SerialCommands serialCommands(mqttManager, wifiManager, provisioningStore);
PirSensor pirSensor(Config::PIR_PIN, Config::PIR_WARMUP_MS);
DeviceProfile deviceProfile;

// Luu trang thai PIR lan truoc de chi xu ly khi co thay doi
bool lastPirState = false;
} // namespace

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(1000);

  Serial.println();
  Serial.println("========== ESP32 SENSOR MQTT ==========");
  Serial.println("Serial Monitor: 115200 baud, Newline.");
  Serial.println("Commands: PROVISION {json}, STATUS, SCAN, RESET_PROVISIONING");

  if (provisioningStore.load(deviceProfile)) {
    wifiManager.begin(deviceProfile.ssid, deviceProfile.password);
    mqttManager.begin(deviceProfile);
  } else {
    Serial.println("Device is not provisioned; MQTT is disabled.");
  }
  pirSensor.begin();
}

void loop() {
  wifiManager.tick();
  mqttManager.tick(wifiManager.isConnected());
  mqttManager.publishHeartbeat();
  serialCommands.tick();

  pirSensor.update();

  // Chi publish khi PIR da san sang VA trang thai thay doi
  if (pirSensor.isReady()) {
    bool currentState = pirSensor.isMotionDetected();
    if (currentState != lastPirState) {
      lastPirState = currentState;

      if (currentState) {
        Serial.println("[PIR] LOW -> HIGH, gui gia tri random 1-10.");
        mqttManager.publishSensorSample();
      } else {
        Serial.println("[PIR] HIGH -> LOW.");
      }
    }
  }

  delay(10);
}
