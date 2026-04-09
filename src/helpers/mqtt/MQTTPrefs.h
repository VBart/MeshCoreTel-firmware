#pragma once

#ifdef WITH_MQTT_UPLINK

#include <helpers/IdentityStore.h>
#include <stdint.h>

#ifndef MQTT_DEFAULT_IATA
  #define MQTT_DEFAULT_IATA "MEL"
#endif

struct MQTTPrefs {
  uint32_t magic;
  uint8_t enabled_mask;
  uint8_t packets_enabled;
  uint8_t raw_enabled;
  uint8_t status_enabled;
  uint8_t tx_enabled;
  uint8_t web_enabled;
  uint8_t wifi_powersave;
  uint32_t status_interval_ms;
  char iata[8];
  char wifi_ssid[33];
  char wifi_pwd[65];
  char owner_public_key[65];
  char owner_email[96];
};

class MQTTPrefsStore {
public:
  static void setDefaults(MQTTPrefs& prefs);
  static bool load(FILESYSTEM* fs, MQTTPrefs& prefs);
  static bool save(FILESYSTEM* fs, const MQTTPrefs& prefs);

private:
  static constexpr uint32_t kMagic = 0x4D515454;
  static constexpr const char* kFilename = "/mqtt_prefs";
};

#endif
