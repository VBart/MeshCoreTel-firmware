#pragma once

#include <helpers/IdentityStore.h>
#include <stdint.h>

struct NetworkPrefs {
  uint32_t magic;
  uint8_t wifi_powersave;
  uint8_t reserved[3];
  char wifi_ssid[33];
  char wifi_pwd[65];
};

class NetworkPrefsStore {
public:
  static void setDefaults(NetworkPrefs& prefs);
  static bool load(FILESYSTEM* fs, NetworkPrefs& prefs,
                   uint8_t legacy_wifi_powersave = 0,
                   const char* legacy_wifi_ssid = nullptr,
                   const char* legacy_wifi_pwd = nullptr);
  static bool save(FILESYSTEM* fs, const NetworkPrefs& prefs);

private:
  static constexpr uint32_t kMagic = 0x4E455450;
  static constexpr const char* kFilename = "/network_prefs";
};
