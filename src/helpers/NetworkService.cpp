#include "NetworkService.h"

#include <helpers/TxtDataHelpers.h>
#include <string.h>
#include <time.h>

#if defined(ESP_PLATFORM)
  #include <WiFi.h>
  #include <esp_sntp.h>
#endif

namespace {

#if defined(ESP_PLATFORM)
constexpr unsigned long kWifiRetryMillis = 15000;
constexpr unsigned long kWifiConnectTimeoutMillis = 45000;
constexpr time_t kMinSaneEpoch = 1735689600;  // 2025-01-01T00:00:00Z

int getWifiQualityPercent(int rssi_dbm) {
  if (rssi_dbm <= -100) {
    return 0;
  }
  if (rssi_dbm >= -50) {
    return 100;
  }
  return 2 * (rssi_dbm + 100);
}

const char* getWifiQualityLabel(int rssi_dbm) {
  if (rssi_dbm >= -60) {
    return "excellent";
  }
  if (rssi_dbm >= -67) {
    return "good";
  }
  if (rssi_dbm >= -75) {
    return "fair";
  }
  return "poor";
}
#endif

}  // namespace

NetworkService::NetworkService()
    : _fs(nullptr), _prefs{}, _wifi_started(false), _sntp_started(false), _have_time_sync(false), _last_wifi_attempt(0) {
  NetworkPrefsStore::setDefaults(_prefs);
}

void NetworkService::begin(FILESYSTEM* fs,
                           uint8_t legacy_wifi_powersave,
                           const char* legacy_wifi_ssid,
                           const char* legacy_wifi_pwd) {
  _fs = fs;
  NetworkPrefsStore::load(_fs, _prefs, legacy_wifi_powersave, legacy_wifi_ssid, legacy_wifi_pwd);
}

void NetworkService::end() {
#if defined(ESP_PLATFORM)
  if (_wifi_started) {
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
  }
#endif
  _wifi_started = false;
  _sntp_started = false;
  _have_time_sync = false;
  _last_wifi_attempt = 0;
}

void NetworkService::loop(bool network_required) {
#if defined(ESP_PLATFORM)
  ensureWifi(network_required);
  updateTimeSync();
#else
  (void)network_required;
#endif
}

bool NetworkService::savePrefs() {
  return NetworkPrefsStore::save(_fs, _prefs);
}

bool NetworkService::setWifiSSID(const char* ssid) {
  if (ssid == nullptr) {
    return false;
  }
  StrHelper::strncpy(_prefs.wifi_ssid, ssid, sizeof(_prefs.wifi_ssid));
  bool ok = savePrefs();
  reconnectWifi();
  return ok;
}

bool NetworkService::setWifiPassword(const char* pwd) {
  if (pwd == nullptr) {
    return false;
  }
  StrHelper::strncpy(_prefs.wifi_pwd, pwd, sizeof(_prefs.wifi_pwd));
  bool ok = savePrefs();
  reconnectWifi();
  return ok;
}

bool NetworkService::setWifiPowerSave(const char* mode) {
  if (mode == nullptr) {
    return false;
  }

  uint8_t next_mode;
  if (strcmp(mode, "none") == 0) {
    next_mode = 0;
  } else if (strcmp(mode, "min") == 0) {
    next_mode = 1;
  } else if (strcmp(mode, "max") == 0) {
    next_mode = 2;
  } else {
    return false;
  }

  _prefs.wifi_powersave = next_mode;
  bool ok = savePrefs();
#if defined(ESP_PLATFORM)
  if (_wifi_started) {
    ok = WiFi.setSleep(toEspPowerSave(_prefs.wifi_powersave)) && ok;
  }
#endif
  return ok;
}

const char* NetworkService::getWifiPowerSave() const {
#if defined(ESP_PLATFORM)
  return getPowerSaveLabel(_prefs.wifi_powersave);
#else
  return "unsupported";
#endif
}

void NetworkService::formatWifiStatusReply(char* reply, size_t reply_size) const {
#if defined(ESP_PLATFORM)
  const char* status = "disconnected";
  const char* state = "disconnected";
  wl_status_t wifi_status = WiFi.status();
  if (_prefs.wifi_ssid[0] == 0) {
    status = "unconfigured";
    state = "unconfigured";
  } else if (wifi_status == WL_CONNECTED) {
    status = "connected";
    state = "connected";
  } else if (_wifi_started) {
    status = "connecting";
  }

  switch (wifi_status) {
    case WL_IDLE_STATUS:
      state = "idle";
      break;
    case WL_NO_SSID_AVAIL:
      state = "no_ssid";
      break;
    case WL_SCAN_COMPLETED:
      state = "scan_completed";
      break;
    case WL_CONNECTED:
      state = "connected";
      break;
    case WL_CONNECT_FAILED:
      state = "connect_failed";
      break;
    case WL_CONNECTION_LOST:
      state = "connection_lost";
      break;
    case WL_DISCONNECTED:
      state = "disconnected";
      break;
    default:
      state = "unknown";
      break;
  }

  if (wifi_status == WL_CONNECTED) {
    const int rssi_dbm = WiFi.RSSI();
    snprintf(reply, reply_size,
             "> ssid:%s status:%s code:%d state:%s ip:%s rssi:%d quality:%d%% signal:%s",
             _prefs.wifi_ssid, status, static_cast<int>(wifi_status), state, WiFi.localIP().toString().c_str(),
             rssi_dbm, getWifiQualityPercent(rssi_dbm), getWifiQualityLabel(rssi_dbm));
  } else {
    snprintf(reply, reply_size, "> ssid:%s status:%s code:%d state:%s", _prefs.wifi_ssid[0] ? _prefs.wifi_ssid : "-",
             status, static_cast<int>(wifi_status), state);
  }
#else
  snprintf(reply, reply_size, "> wifi:unsupported");
#endif
}

void NetworkService::reconnectWifi() {
#if defined(ESP_PLATFORM)
  if (_wifi_started) {
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
  }
#endif
  _wifi_started = false;
  _sntp_started = false;
  _have_time_sync = false;
  _last_wifi_attempt = 0;
}

bool NetworkService::isWifiConnected() const {
#if defined(ESP_PLATFORM)
  return _wifi_started && WiFi.status() == WL_CONNECTED;
#else
  return false;
#endif
}

#if defined(ESP_PLATFORM)
wifi_ps_type_t NetworkService::toEspPowerSave(uint8_t mode) {
  switch (mode) {
    case 1:
      return WIFI_PS_MIN_MODEM;
    case 2:
      return WIFI_PS_MAX_MODEM;
    default:
      return WIFI_PS_NONE;
  }
}

const char* NetworkService::getPowerSaveLabel(uint8_t mode) {
  switch (mode) {
    case 1:
      return "min";
    case 2:
      return "max";
    default:
      return "none";
  }
}

void NetworkService::ensureWifi(bool network_required) {
  if (!network_required) {
    if (_wifi_started) {
      WiFi.disconnect(true, true);
      WiFi.mode(WIFI_OFF);
      _wifi_started = false;
      _sntp_started = false;
      _have_time_sync = false;
    }
    return;
  }

  if (_prefs.wifi_ssid[0] == 0) {
    reconnectWifi();
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  unsigned long now_ms = millis();
  wl_status_t status = WiFi.status();
  if (_wifi_started) {
    if (_last_wifi_attempt != 0 && status == WL_IDLE_STATUS && now_ms - _last_wifi_attempt < kWifiConnectTimeoutMillis) {
      return;
    }
    if (now_ms - _last_wifi_attempt < kWifiRetryMillis) {
      return;
    }
  }

  if (!_wifi_started) {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(toEspPowerSave(_prefs.wifi_powersave));
    _wifi_started = true;
  }

  _last_wifi_attempt = now_ms;
  WiFi.begin(_prefs.wifi_ssid, _prefs.wifi_pwd);
}

void NetworkService::updateTimeSync() {
  bool prev_have_time_sync = _have_time_sync;
  if (!_wifi_started || WiFi.status() != WL_CONNECTED) {
    _have_time_sync = false;
    return;
  }

  if (!_sntp_started) {
    configTzTime("UTC0", "au.pool.ntp.org", "time.google.com", "time.cloudflare.com");
    _sntp_started = true;
  }

  sntp_sync_status_t sync_status = sntp_get_sync_status();
  time_t now = time(nullptr);
  bool sane_time = now >= kMinSaneEpoch;
  bool sync_ready = sync_status == SNTP_SYNC_STATUS_COMPLETED || sync_status == SNTP_SYNC_STATUS_IN_PROGRESS;
  _have_time_sync = sane_time && (sync_ready || prev_have_time_sync);
}
#endif
