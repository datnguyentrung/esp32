#pragma once

#include <Arduino.h>

namespace Config {
constexpr uint8_t PIR_PIN = 27;
constexpr uint32_t PIR_WARMUP_MS = 10000;

constexpr unsigned long MQTT_RETRY_MS = 5000;
constexpr unsigned long WIFI_STATUS_INTERVAL_MS = 5000;
constexpr unsigned long MQTT_KEEP_ALIVE_MS = 30UL * 1000UL;
constexpr unsigned long MQTT_CONNECTION_TIMEOUT_MS = 5000;
constexpr int MQTT_QOS = 1;
} // namespace Config
