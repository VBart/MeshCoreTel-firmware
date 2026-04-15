#include "WebPrefs.h"

#include <string.h>

void WebPrefsStore::setDefaults(WebPrefs& prefs) {
  memset(&prefs, 0, sizeof(prefs));
  prefs.magic = kMagic;
  prefs.web_enabled = 1;
  prefs.web_stats_enabled = 1;
}

bool WebPrefsStore::load(FILESYSTEM* fs, WebPrefs& prefs) {
  setDefaults(prefs);
  if (fs == nullptr) {
    return false;
  }

  if (!fs->exists(kFilename)) {
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

  WebPrefs persisted{};
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
  prefs.web_enabled = prefs.web_enabled ? 1 : 0;
  prefs.web_stats_enabled = prefs.web_stats_enabled ? 1 : 0;
  return true;
}

bool WebPrefsStore::save(FILESYSTEM* fs, const WebPrefs& prefs) {
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
