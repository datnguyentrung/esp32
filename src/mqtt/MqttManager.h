#pragma once

#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include "provisioning/ProvisioningStore.h"

class MqttManager {
 public:
  MqttManager();

  void begin(const DeviceProfile& profile);
  void tick(bool wifiConnected);
  bool publishSensorSample();
  bool publishPirState(bool motionDetected);
  void publishHeartbeat();
  bool isProvisioned() const { return provisioned_; }

 private:
  WiFiClient wifiClient_;
  MqttClient mqttClient_;
  unsigned long lastAttemptMs_ = 0;
  bool attempted_ = false;
  DeviceProfile profile_;
  bool provisioned_ = false;
  unsigned long lastHeartbeatMs_ = 0;
};
