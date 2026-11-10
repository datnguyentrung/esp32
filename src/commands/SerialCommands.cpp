#include "commands/SerialCommands.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "mqtt/MqttManager.h"
#include "provisioning/ProvisioningStore.h"
#include "wifi/WifiManager.h"

SerialCommands::SerialCommands(MqttManager& mqttManager, WifiManager& wifiManager, ProvisioningStore& store)
    : mqttManager_(mqttManager), wifiManager_(wifiManager), store_(store) {}

void SerialCommands::tick() {
  if (Serial.available() <= 0) return;

  String command = Serial.readStringUntil('\n');
  command.trim();
  if (command.equalsIgnoreCase("SCAN")) {
    Serial.println("[NHAN LENH] SCAN");
    mqttManager_.publishSensorSample();
  } else if (command.equalsIgnoreCase("STATUS")) {
    Serial.println(mqttManager_.isProvisioned() ? "PROVISIONED" : "NOT_PROVISIONED");
  } else if (command.equalsIgnoreCase("RESET_PROVISIONING")) {
    store_.clear(); Serial.println("Provisioning cleared; restart device.");
  } else if (command.startsWith("PROVISION ")) {
    JsonDocument document;
    if (deserializeJson(document, command.substring(10))) { Serial.println("Invalid provisioning JSON."); return; }
    const String ssid = document["ssid"] | ""; const String password = document["password"] | "";
    const String fogUrl = document["fogUrl"] | ""; const String code = document["code"] | "";
    if (ssid.isEmpty() || password.isEmpty() || fogUrl.isEmpty() || code.isEmpty()) { Serial.println("Provisioning fields missing."); return; }
    wifiManager_.begin(ssid, password);
    // Phone hotspots can take longer than a normal access point to complete
    // association/DHCP. Do not reject a provisioning code while Wi-Fi is
    // still connecting: claim only begins after this loop succeeds.
    const unsigned long deadline = millis() + 45000;
    while (!wifiManager_.isConnected() && millis() < deadline) { wifiManager_.tick(); delay(100); }
    DeviceProfile profile;
    if (!wifiManager_.isConnected() || !store_.claim(ssid, password, fogUrl, code, String((uint32_t)ESP.getEfuseMac(), HEX), profile) || !store_.save(profile)) { Serial.println("Provisioning failed."); return; }
    Serial.println("Provisioning succeeded; restarting."); delay(500); ESP.restart();
  } else if (command.length() > 0) {
    Serial.print("[LENH KHONG DUNG] Ban vua nhap: ");
    Serial.println(command);
    Serial.println("Use PROVISION {json}, STATUS, SCAN, or RESET_PROVISIONING.");
  }
}
