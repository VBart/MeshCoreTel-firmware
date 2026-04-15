#pragma once

#include <Arduino.h>
#include <helpers/IdentityStore.h>
#include <helpers/NetworkStateProvider.h>

#if defined(ESP_PLATFORM)
  #include <WiFi.h>
#endif

#include "NetworkPrefs.h"

class NetworkService : public NetworkStateProvider {
public:
  NetworkService();

  void begin(FILESYSTEM* fs,
             uint8_t legacy_wifi_powersave = 0,
             const char* legacy_wifi_ssid = nullptr,
             const char* legacy_wifi_pwd = nullptr);
  void end();
  void loop(bool network_required);

  bool setWifiSSID(const char* ssid);
  bool setWifiPassword(const char* pwd);
  const char* getWifiSSID() const { return _prefs.wifi_ssid; }
  bool setWifiPowerSave(const char* mode);
  const char* getWifiPowerSave() const;
  void formatWifiStatusReply(char* reply, size_t reply_size) const;
  void reconnectWifi();

  bool isWifiConnected() const override;
  bool hasTimeSync() const override { return _have_time_sync; }

private:
#if defined(ESP_PLATFORM)
  static wifi_ps_type_t toEspPowerSave(uint8_t mode);
  static const char* getPowerSaveLabel(uint8_t mode);
  void ensureWifi(bool network_required);
  void updateTimeSync();
#endif
  bool savePrefs();

  FILESYSTEM* _fs;
  NetworkPrefs _prefs;
  bool _wifi_started;
  bool _sntp_started;
  bool _have_time_sync;
  unsigned long _last_wifi_attempt;
};
