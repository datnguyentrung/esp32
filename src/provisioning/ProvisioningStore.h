#pragma once

#include <Arduino.h>

struct DeviceProfile {
  String ssid;
  String password;
  String fogUrl;
  String deviceCode;
  String mqttSecret;
  String mqttHost;
  uint16_t mqttPort = 1883;
  String sensorTopic;
  String heartbeatTopic;
  uint32_t heartbeatSeconds = 30;
  String firmwareVersion;
  int desiredConfigVersion = 0;
};

class ProvisioningStore {
 public:
  bool load(DeviceProfile& profile) const;
  bool save(const DeviceProfile& profile) const;
  void clear() const;
  bool claim(const String& ssid, const String& password, const String& fogUrl,
             const String& code, const String& chipId, DeviceProfile& profile) const;
};
