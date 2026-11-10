#include "provisioning/ProvisioningStore.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>

namespace {
constexpr char kNamespace[] = "tkd-device";

String read(Preferences& preferences, const char* key) { return preferences.getString(key, ""); }
}

bool ProvisioningStore::load(DeviceProfile& profile) const {
  Preferences preferences;
  if (!preferences.begin(kNamespace, true)) return false;
  profile.ssid = read(preferences, "ssid");
  profile.password = read(preferences, "wifiPass");
  profile.fogUrl = read(preferences, "fogUrl");
  profile.deviceCode = read(preferences, "deviceCode");
  profile.mqttSecret = read(preferences, "mqttSecret");
  profile.mqttHost = read(preferences, "mqttHost");
  profile.mqttPort = preferences.getUShort("mqttPort", 1883);
  profile.sensorTopic = read(preferences, "sensorTopic");
  profile.heartbeatTopic = read(preferences, "heartTopic");
  profile.heartbeatSeconds = preferences.getUInt("heartSecs", 30);
  profile.firmwareVersion = read(preferences, "firmware");
  profile.desiredConfigVersion = preferences.getInt("configVer", 0);
  preferences.end();
  return !profile.ssid.isEmpty() && !profile.deviceCode.isEmpty() && !profile.mqttSecret.isEmpty();
}

bool ProvisioningStore::save(const DeviceProfile& profile) const {
  Preferences preferences;
  if (!preferences.begin(kNamespace, false)) return false;
  preferences.putString("ssid", profile.ssid); preferences.putString("wifiPass", profile.password);
  preferences.putString("fogUrl", profile.fogUrl); preferences.putString("deviceCode", profile.deviceCode);
  preferences.putString("mqttSecret", profile.mqttSecret); preferences.putString("mqttHost", profile.mqttHost);
  preferences.putUShort("mqttPort", profile.mqttPort); preferences.putString("sensorTopic", profile.sensorTopic);
  preferences.putString("heartTopic", profile.heartbeatTopic); preferences.putUInt("heartSecs", profile.heartbeatSeconds);
  preferences.putString("firmware", profile.firmwareVersion); preferences.putInt("configVer", profile.desiredConfigVersion);
  preferences.end();
  return true;
}

void ProvisioningStore::clear() const { Preferences preferences; if (preferences.begin(kNamespace, false)) { preferences.clear(); preferences.end(); } }

bool ProvisioningStore::claim(const String& ssid, const String& password, const String& fogUrl,
                              const String& code, const String& chipId, DeviceProfile& profile) const {
  HTTPClient http;
  const String endpoint = fogUrl + "/local/device-provisioning/esp32/claim";
  if (!http.begin(endpoint)) return false;
  http.addHeader("Content-Type", "application/json");
  JsonDocument request;
  request["code"] = code; request["hardwareId"] = chipId;
  request["firmwareVersion"] = "1.0.0"; request["hardwareVersion"] = "ESP32";
  String body; serializeJson(request, body);
  const int status = http.POST(body);
  if (status < 200 || status >= 300) { http.end(); return false; }
  JsonDocument response;
  const auto error = deserializeJson(response, http.getString());
  http.end();
  if (error || !response["deviceCode"].is<const char*>() || !response["secret"].is<const char*>()) return false;
  const JsonObject runtime = response["runtime"];
  profile.ssid = ssid; profile.password = password; profile.fogUrl = fogUrl;
  profile.deviceCode = String(response["deviceCode"].as<const char*>());
  profile.mqttSecret = String(response["secret"].as<const char*>());
  profile.mqttHost = String(runtime["mqttTcpHost"] | ""); profile.mqttPort = runtime["mqttTcpPort"] | 1883;
  profile.sensorTopic = "esp32/" + profile.deviceCode + "/sensor";
  profile.heartbeatTopic = String(runtime["heartbeatTopic"] | ""); profile.heartbeatSeconds = runtime["heartbeatSeconds"] | 30;
  profile.firmwareVersion = "1.0.0"; profile.desiredConfigVersion = runtime["desiredConfigVersion"] | 0;
  return !profile.mqttHost.isEmpty() && !profile.heartbeatTopic.isEmpty();
}
