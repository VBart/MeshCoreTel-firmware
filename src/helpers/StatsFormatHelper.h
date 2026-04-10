#pragma once

#include "Mesh.h"
#include <Arduino.h>

class StatsFormatHelper {
public:
  static void formatCoreStats(char* reply,
                             size_t reply_size,
                             mesh::MainBoard& board,
                             mesh::MillisecondClock& ms,
                             uint16_t err_flags,
                             mesh::PacketManager* mgr) {
    snprintf(reply, reply_size,
      "{\"battery_mv\":%u,\"uptime_secs\":%u,\"errors\":%u,\"queue_len\":%u}",
      board.getBattMilliVolts(),
      ms.getMillis() / 1000,
      err_flags,
      mgr->getOutboundTotal()
    );
  }

  template<typename RadioDriverType>
  static void formatRadioStats(char* reply,
                              size_t reply_size,
                              mesh::Radio* radio,
                              RadioDriverType& driver,
                              uint32_t total_air_time_ms,
                              uint32_t total_rx_air_time_ms) {
    snprintf(reply, reply_size,
      "{\"noise_floor\":%d,\"last_rssi\":%d,\"last_snr\":%.2f,\"tx_air_secs\":%u,\"rx_air_secs\":%u}",
      (int16_t)radio->getNoiseFloor(),
      (int16_t)driver.getLastRSSI(),
      driver.getLastSNR(),
      total_air_time_ms / 1000,
      total_rx_air_time_ms / 1000
    );
  }

  template<typename RadioDriverType>
  static void formatPacketStats(char* reply,
                               size_t reply_size,
                               RadioDriverType& driver,
                               uint32_t n_sent_flood,
                               uint32_t n_sent_direct,
                               uint32_t n_recv_flood,
                               uint32_t n_recv_direct) {
    snprintf(reply, reply_size,
      "{\"recv\":%u,\"sent\":%u,\"flood_tx\":%u,\"direct_tx\":%u,\"flood_rx\":%u,\"direct_rx\":%u,\"recv_errors\":%u}",
      driver.getPacketsRecv(),
      driver.getPacketsSent(),
      n_sent_flood,
      n_sent_direct,
      n_recv_flood,
      n_recv_direct,
      driver.getPacketsRecvErrors()
    );
  }

  static void formatMemoryStats(char* reply, size_t reply_size) {
    if (reply == nullptr || reply_size == 0) {
      return;
    }
#if defined(ESP32)
    snprintf(reply, reply_size,
      "{\"heap_free\":%u,\"heap_min\":%u,\"heap_max\":%u,\"psram_free\":%u,\"psram_min\":%u,\"psram_max\":%u}",
      ESP.getFreeHeap(),
      ESP.getMinFreeHeap(),
      ESP.getMaxAllocHeap(),
      ESP.getFreePsram(),
      ESP.getMinFreePsram(),
      ESP.getMaxAllocPsram()
    );
#else
    snprintf(reply, reply_size, "{\"supported\":false}");
#endif
  }
};
