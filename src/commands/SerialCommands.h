#pragma once

class MqttManager;
class WifiManager;
class ProvisioningStore;

class SerialCommands {
 public:
  explicit SerialCommands(MqttManager& mqttManager, WifiManager& wifiManager, ProvisioningStore& store);
  void tick();

 private:
  MqttManager& mqttManager_;
  WifiManager& wifiManager_;
  ProvisioningStore& store_;
};
