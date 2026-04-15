#include "NetworkPrefs.h"

#include <helpers/TxtDataHelpers.h>
#include <string.h>

namespace {

struct LegacyWebPrefsV1 {
  uint32_t magic;
  uint8_t web_enabled;
  uint8_t web_stats_enabled;
  uint8_t wifi_powersave;
  uint8_t reserved;
  char wifi_ssid[33];
  char wifi_pwd[65];
};

bool loadLegacyWebWifiPrefs(FILESYSTEM* fs, NetworkPrefs& prefs) {
  if (fs == nullptr || !fs->exists("/web_prefs")) {
    return false;
  }

#if defined(RP2040_PLATFORM)
  File file = fs->open("/web_prefs", "r");
#else
  File file = fs->open("/web_prefs");
#endif
  if (!file) {
    return false;
  }

  if (static_cast<size_t>(file.size()) < sizeof(LegacyWebPrefsV1)) {
    file.close();
    return false;
  }

  LegacyWebPrefsV1 legacy{};
  bool ok = file.read(reinterpret_cast<uint8_t*>(&legacy), sizeof(legacy)) == sizeof(legacy);
  file.close();
  if (!ok || legacy.magic != 0x57454250) {
    return false;
  }

  prefs.wifi_powersave = legacy.wifi_powersave <= 2 ? legacy.wifi_powersave : 0;
  StrHelper::strncpy(prefs.wifi_ssid, legacy.wifi_ssid, sizeof(prefs.wifi_ssid));
  StrHelper::strncpy(prefs.wifi_pwd, legacy.wifi_pwd, sizeof(prefs.wifi_pwd));
  return true;
}

}  // namespace

void NetworkPrefsStore::setDefaults(NetworkPrefs& prefs) {
  memset(&prefs, 0, sizeof(prefs));
  prefs.magic = kMagic;
  prefs.wifi_powersave = 0;
}

bool NetworkPrefsStore::load(FILESYSTEM* fs, NetworkPrefs& prefs,
                             uint8_t legacy_wifi_powersave,
                             const char* legacy_wifi_ssid,
                             const char* legacy_wifi_pwd) {
  setDefaults(prefs);
  if (fs == nullptr) {
    return false;
  }

  if (!fs->exists(kFilename)) {
    prefs.wifi_powersave = legacy_wifi_powersave <= 2 ? legacy_wifi_powersave : 0;
    if (legacy_wifi_ssid != nullptr) {
      StrHelper::strncpy(prefs.wifi_ssid, legacy_wifi_ssid, sizeof(prefs.wifi_ssid));
    }
    if (legacy_wifi_pwd != nullptr) {
      StrHelper::strncpy(prefs.wifi_pwd, legacy_wifi_pwd, sizeof(prefs.wifi_pwd));
    }
    loadLegacyWebWifiPrefs(fs, prefs);
    save(fs, prefs);
    return false;
  }

#if defined(RP2040_PLATFORM)
  File file = fs->open(kFilename, "r");
#else
  File file = fs->open(kFilename);
#endif
  if (!file) {
    return false;
  }

  NetworkPrefs persisted{};
  size_t bytes_to_read = min(static_cast<size_t>(file.size()), sizeof(persisted));
  bool ok = bytes_to_read >= sizeof(persisted.magic) &&
            file.read(reinterpret_cast<uint8_t*>(&persisted), bytes_to_read) == bytes_to_read;
  file.close();

  if (!ok || persisted.magic != kMagic) {
    fs->remove(kFilename);
    save(fs, prefs);
    return false;
  }

  prefs = persisted;
  if (prefs.wifi_powersave > 2) {
    prefs.wifi_powersave = 0;
  }
  return true;
}

bool NetworkPrefsStore::save(FILESYSTEM* fs, const NetworkPrefs& prefs) {
  if (fs == nullptr) {
    return false;
  }
  if (fs->exists(kFilename) && !fs->remove(kFilename)) {
    return false;
  }
#if defined(RP2040_PLATFORM)
  File file = fs->open(kFilename, "w");
#else
  File file = fs->open(kFilename, "w", true);
#endif
  if (!file) {
    return false;
  }
  bool ok = file.write(reinterpret_cast<const uint8_t*>(&prefs), sizeof(prefs)) == sizeof(prefs);
  file.close();
  return ok;
}
