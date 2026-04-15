#pragma once

class NetworkStateProvider {
public:
  virtual ~NetworkStateProvider() = default;
  virtual bool isWifiConnected() const = 0;
  virtual bool hasTimeSync() const = 0;
};
