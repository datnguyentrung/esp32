#include "mqtt/MqttManager.h"

#include "config.h"

MqttManager::MqttManager() : mqttClient_(wifiClient_) {}

void MqttManager::begin(const DeviceProfile& profile) {
  profile_ = profile;
  provisioned_ = !profile_.deviceCode.isEmpty() && !profile_.mqttSecret.isEmpty();
  if (!provisioned_) return;
  mqttClient_.setId(profile_.deviceCode.c_str());
  mqttClient_.setUsernamePassword(profile_.deviceCode.c_str(), profile_.mqttSecret.c_str());
  mqttClient_.setKeepAliveInterval(Config::MQTT_KEEP_ALIVE_MS);
  mqttClient_.setConnectionTimeout(Config::MQTT_CONNECTION_TIMEOUT_MS);
}

void MqttManager::tick(bool wifiConnected) {
  if (!provisioned_) return;
  if (!wifiConnected) {
    if (mqttClient_.connected())
      mqttClient_.stop();
    attempted_ = false;
    return;
  }

  if (mqttClient_.connected()) {
    mqttClient_.poll();
    return;
  }

  const unsigned long now = millis();
  if (attempted_ && now - lastAttemptMs_ < Config::MQTT_RETRY_MS)
    return;

  attempted_ = true;
  lastAttemptMs_ = now;
  Serial.print("[MQTT] Dang ket noi broker ");
  Serial.print(profile_.mqttHost);
  Serial.print(":");
  Serial.println(profile_.mqttPort);

  if (mqttClient_.connect(profile_.mqttHost.c_str(), profile_.mqttPort)) {
    Serial.println("[MQTT] DA KET NOI broker.");
    Serial.print("[MQTT] San sang gui toi topic: ");
    Serial.println(profile_.sensorTopic);
    return;
  }

  Serial.print("[MQTT] Ket noi that bai, ma loi: ");
  Serial.println(mqttClient_.connectError());
  Serial.println("Goi y: kiem tra IPv4 broker, port 1883, cung Wi-Fi va "
                 "Windows Firewall.");
}

bool MqttManager::publishSensorSample() {
  if (!mqttClient_.connected()) {
    Serial.println("[LOI] ESP32 chua ket noi MQTT broker.");
    return false;
  }

  const int randomValue = random(1, 11);
  char payload[24];
  snprintf(payload, sizeof(payload), "{\"value\":%d}", randomValue);
  const size_t payloadLength = strlen(payload);
  if (!mqttClient_.beginMessage(profile_.sensorTopic,
                                static_cast<unsigned long>(payloadLength),
                                false, Config::MQTT_QOS)) {
    Serial.println(
        "[LOI MQTT] beginMessage that bai; dong ket noi de thu lai.");
    mqttClient_.stop();
    return false;
  }
  if (mqttClient_.write(reinterpret_cast<const uint8_t *>(payload),
                        payloadLength) != payloadLength) {
    Serial.println("[LOI MQTT] Ghi payload that bai; dong ket noi de thu lai.");
    mqttClient_.stop();
    return false;
  }
  if (!mqttClient_.endMessage()) {
    Serial.println("[LOI MQTT] Publish that bai hoac khong nhan duoc PUBACK.");
    mqttClient_.stop();
    return false;
  }

  Serial.println("[THANH CONG] DA GUI MQTT (QoS 1, da nhan PUBACK).");
  Serial.print("Topic: ");
  Serial.println(profile_.sensorTopic);
  Serial.print("Payload: ");
  Serial.println(payload);
  return true;
}

bool MqttManager::publishPirState(bool motionDetected) {
  if (!mqttClient_.connected()) {
    Serial.println("[LOI] ESP32 chua ket noi MQTT broker.");
    return false;
  }

  // Tao payload JSON: {"motion":true} hoac {"motion":false}
  char payload[24];
  snprintf(payload, sizeof(payload), "{\"motion\":%s}",
           motionDetected ? "true" : "false");
  const size_t payloadLength = strlen(payload);

  if (!mqttClient_.beginMessage(profile_.sensorTopic,
                                static_cast<unsigned long>(payloadLength),
                                false, Config::MQTT_QOS)) {
    Serial.println("[LOI MQTT] PIR beginMessage that bai.");
    mqttClient_.stop();
    return false;
  }
  if (mqttClient_.write(reinterpret_cast<const uint8_t *>(payload),
                        payloadLength) != payloadLength) {
    Serial.println("[LOI MQTT] PIR ghi payload that bai.");
    mqttClient_.stop();
    return false;
  }
  if (!mqttClient_.endMessage()) {
    Serial.println(
        "[LOI MQTT] PIR publish that bai hoac khong nhan duoc PUBACK.");
    mqttClient_.stop();
    return false;
  }

  Serial.println("[PIR] DA GUI MQTT.");
  Serial.print("Topic: ");
  Serial.println(profile_.sensorTopic);
  Serial.print("Payload: ");
  Serial.println(payload);
  return true;
}

void MqttManager::publishHeartbeat() {
  if (!mqttClient_.connected() || profile_.heartbeatTopic.isEmpty()) return;
  const unsigned long now = millis();
  if (now - lastHeartbeatMs_ < profile_.heartbeatSeconds * 1000UL) return;
  lastHeartbeatMs_ = now;
  char payload[192];
  snprintf(payload, sizeof(payload), "{\"deviceCode\":\"%s\",\"firmwareVersion\":\"%s\",\"appliedConfigVersion\":%d,\"rssi\":%d,\"uptimeMs\":%lu,\"health\":\"HEALTHY\"}", profile_.deviceCode.c_str(), profile_.firmwareVersion.c_str(), profile_.desiredConfigVersion, WiFi.RSSI(), now);
  mqttClient_.beginMessage(profile_.heartbeatTopic, strlen(payload), false, Config::MQTT_QOS);
  mqttClient_.print(payload);
  mqttClient_.endMessage();
}
