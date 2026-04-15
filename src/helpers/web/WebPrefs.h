#pragma once

#include <helpers/IdentityStore.h>
#include <stdint.h>

struct WebPrefs {
  uint32_t magic;
  uint8_t web_enabled;
  uint8_t web_stats_enabled;
  uint8_t reserved[2];
};

class WebPrefsStore {
public:
  static void setDefaults(WebPrefs& prefs);
  static bool load(FILESYSTEM* fs, WebPrefs& prefs);
  static bool save(FILESYSTEM* fs, const WebPrefs& prefs);

private:
  static constexpr uint32_t kMagic = 0x57454250;
  static constexpr const char* kFilename = "/web_prefs";
};
