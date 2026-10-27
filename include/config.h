#pragma once

#include <Arduino.h>

namespace Config {
constexpr char WIFI_SSID[] = "Iphone của Đạt";
constexpr char WIFI_PASSWORD[] = "datnt311005";

// Update this when the laptop joins a different Wi-Fi or hotspot.
constexpr char MQTT_BROKER[] = "172.20.10.3";
constexpr uint16_t MQTT_PORT = 1883;
constexpr char DEVICE_ID[] = "esp32-01";
constexpr char SENSOR_TOPIC[] = "esp32/esp32-01/sensor";
constexpr char SENSOR_PAYLOAD[] = "{\"temperature\":29.5,\"humidity\":74}";
constexpr char PIR_TOPIC[] = "esp32/esp32-01/sensor";
constexpr uint8_t PIR_PIN = 27;
constexpr uint32_t PIR_WARMUP_MS = 60000;

constexpr unsigned long MQTT_RETRY_MS = 5000;
constexpr unsigned long WIFI_STATUS_INTERVAL_MS = 5000;
constexpr unsigned long MQTT_KEEP_ALIVE_MS = 30UL * 1000UL;
constexpr unsigned long MQTT_CONNECTION_TIMEOUT_MS = 5000;
constexpr int MQTT_QOS = 1;
} // namespace Config
