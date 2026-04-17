#include "StatsHistory.h"

#include <stdio.h>
#include <string.h>

#if defined(ESP32)
  #include <esp_heap_caps.h>
#endif

#ifndef ARCHIVE_DEBUG
  #if defined(MQTT_DEBUG) && MQTT_DEBUG
    #define ARCHIVE_DEBUG 1
  #else
    #define ARCHIVE_DEBUG 0
  #endif
#endif

#if ARCHIVE_DEBUG
  #define ARCHIVE_LOG(fmt, ...) Serial.printf("[ARCHIVE] " fmt "\n", ##__VA_ARGS__)
#else
  #define ARCHIVE_LOG(...) do { } while (0)
#endif

namespace {

constexpr uint32_t kSummaryFlushIntervalMs = 5UL * 60UL * 1000UL;
constexpr uint32_t kEventFlushIntervalMs = 60UL * 1000UL;
constexpr size_t kMaxSeriesPoints = 64;
constexpr size_t kSummaryRestoreWindowBytes = 16384;
constexpr size_t kEventsRestoreWindowBytes = 4096;
constexpr uint32_t kBootAutoCapturePsramMinBytes = 2000000UL;
constexpr size_t kLiveOnlySampleCapacity = 24;
constexpr size_t kLiveOnlyEventCapacity = 8;

struct HistoryCapacityBucket {
  size_t sample_capacity;
  size_t event_capacity;
};

HistoryCapacityBucket getHistoryCapacityBucket(bool want_psram) {
#if defined(ESP32)
  if (!want_psram) {
    return {kLiveOnlySampleCapacity, kLiveOnlyEventCapacity};
  }

  const uint32_t psram_size = ESP.getPsramSize();
  if (psram_size >= (8UL * 1024UL * 1024UL)) {
    return {720, 288};
  }
  if (psram_size >= (4UL * 1024UL * 1024UL)) {
    return {480, 192};
  }
  return {240, 96};
#else
  (void)want_psram;
  return {96, 32};
#endif
}

bool shouldUseLiveOnlyStats() {
#if defined(ESP32)
  return !psramFound();
#else
  return false;
#endif
}

bool hasBootAutoCapturePsram() {
#if defined(ESP32)
  return psramFound() && ESP.getPsramSize() >= kBootAutoCapturePsramMinBytes;
#else
  return false;
#endif
}

template <typename T>
T* allocHistoryBuffer(size_t count) {
#if defined(ESP32)
  if (psramFound()) {
    void* ptr = heap_caps_calloc(count, sizeof(T), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (ptr != nullptr) {
      return static_cast<T*>(ptr);
    }
  }
  return static_cast<T*>(calloc(count, sizeof(T)));
#else
  return static_cast<T*>(calloc(count, sizeof(T)));
#endif
}

void freeHistoryBuffer(void* ptr) {
#if defined(ESP32)
  if (ptr != nullptr) {
    free(ptr);
  }
#else
  free(ptr);
#endif
}

File openArchiveRead(FILESYSTEM* fs, const char* filename) {
#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM) || defined(RP2040_PLATFORM)
  return fs->open(filename, "r");
#else
  return fs->open(filename, FILE_READ);
#endif
}

File openArchiveAppend(FILESYSTEM* fs, const char* filename) {
#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM) || defined(RP2040_PLATFORM)
  return fs->open(filename, "a");
#else
  return fs->open(filename, FILE_APPEND, true);
#endif
}

File openArchiveWrite(FILESYSTEM* fs, const char* filename) {
#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  if (fs->exists(filename)) {
    fs->remove(filename);
  }
  return fs->open(filename, FILE_O_WRITE);
#elif defined(RP2040_PLATFORM)
  return fs->open(filename, "w");
#else
  if (fs->exists(filename)) {
    fs->remove(filename);
  }
  return fs->open(filename, FILE_WRITE);
#endif
}

File openArchiveReadWithRecovery(ArchiveStorage* archive, const char* filename) {
  if (archive == nullptr) {
    return File();
  }
  FILESYSTEM* fs = archive->getFS();
  if (fs == nullptr) {
    return File();
  }
  File file = openArchiveRead(fs, filename);
  if (file) {
    return file;
  }
  if (!archive->recover()) {
    return File();
  }
  fs = archive->getFS();
  return fs != nullptr ? openArchiveRead(fs, filename) : File();
}

File openArchiveAppendWithRecovery(ArchiveStorage* archive, const char* filename) {
  if (archive == nullptr) {
    return File();
  }
  FILESYSTEM* fs = archive->getFS();
  if (fs == nullptr) {
    return File();
  }
  File file = openArchiveAppend(fs, filename);
  if (file) {
    return file;
  }
  if (!archive->recover()) {
    return File();
  }
  fs = archive->getFS();
  return fs != nullptr ? openArchiveAppend(fs, filename) : File();
}

File openArchiveWriteWithRecovery(ArchiveStorage* archive, const char* filename) {
  if (archive == nullptr) {
    return File();
  }
  FILESYSTEM* fs = archive->getFS();
  if (fs == nullptr) {
    return File();
  }
  File file = openArchiveWrite(fs, filename);
  if (file) {
    return file;
  }
  if (!archive->recover()) {
    return File();
  }
  fs = archive->getFS();
  return fs != nullptr ? openArchiveWrite(fs, filename) : File();
}

int buildPointValue(const HistorySample& sample, const HistorySample* previous, const char* series) {
  if (strcmp(series, "battery") == 0) {
    return static_cast<int>(sample.battery_mv);
  }
  if (strcmp(series, "memory") == 0) {
    return static_cast<int>(sample.heap_free);
  }
  if (strcmp(series, "signal") == 0) {
    return static_cast<int>(sample.last_rssi_x4);
  }
  if (strcmp(series, "noise_floor") == 0) {
    return static_cast<int>(sample.noise_floor * 4);
  }
  if (strcmp(series, "packets") == 0) {
    if (previous == nullptr) {
      return 0;
    }
    const uint32_t curr_total = sample.packets_sent + sample.packets_recv;
    const uint32_t prev_total = previous->packets_sent + previous->packets_recv;
    return static_cast<int>(curr_total >= prev_total ? (curr_total - prev_total) : 0);
  }
  return 0;
}

const char* seriesTitle(const char* series) {
  if (strcmp(series, "battery") == 0) {
    return "Battery";
  }
  if (strcmp(series, "memory") == 0) {
    return "Heap Free";
  }
  if (strcmp(series, "packets") == 0) {
    return "Packet Activity";
  }
  if (strcmp(series, "signal") == 0) {
    return "Signal";
  }
  if (strcmp(series, "noise_floor") == 0) {
    return "Noise Floor";
  }
  return "";
}

const char* seriesUnit(const char* series) {
  if (strcmp(series, "battery") == 0) {
    return "mV";
  }
  if (strcmp(series, "memory") == 0) {
    return "bytes";
  }
  if (strcmp(series, "packets") == 0) {
    return "pkts";
  }
  if (strcmp(series, "signal") == 0) {
    return "rssi_x4";
  }
  if (strcmp(series, "noise_floor") == 0) {
    return "noise_floor_x4";
  }
  return "";
}

uint32_t sampleAgeSecs(const HistorySample& sample, uint32_t now_epoch_secs, uint32_t now_uptime_secs) {
  if (sample.epoch_secs > 0 && now_epoch_secs >= sample.epoch_secs) {
    return now_epoch_secs - sample.epoch_secs;
  }
  if (now_uptime_secs >= sample.uptime_secs) {
    return now_uptime_secs - sample.uptime_secs;
  }
  return sample.uptime_secs;
}

uint8_t eventTypeFromName(const char* name) {
  if (name == nullptr || name[0] == 0) {
    return 0;
  }
  if (strcmp(name, "boot") == 0) return HISTORY_EVENT_BOOT;
  if (strcmp(name, "web_started") == 0) return HISTORY_EVENT_WEB_STARTED;
  if (strcmp(name, "web_stopped") == 0) return HISTORY_EVENT_WEB_STOPPED;
  if (strcmp(name, "mqtt_connected") == 0) return HISTORY_EVENT_MQTT_CONNECTED;
  if (strcmp(name, "mqtt_disconnected") == 0) return HISTORY_EVENT_MQTT_DISCONNECTED;
  if (strcmp(name, "archive_mounted") == 0) return HISTORY_EVENT_ARCHIVE_MOUNTED;
  if (strcmp(name, "archive_unavailable") == 0) return HISTORY_EVENT_ARCHIVE_UNAVAILABLE;
  if (strcmp(name, "low_memory") == 0) return HISTORY_EVENT_LOW_MEMORY;
  if (strcmp(name, "stats_enabled") == 0) return HISTORY_EVENT_STATS_ENABLED;
  if (strcmp(name, "stats_disabled") == 0) return HISTORY_EVENT_STATS_DISABLED;
  return 0;
}

}  // namespace

StatsHistory::StatsHistory()
    : _enabled(false), _activated(false), _psram_backed(false), _degraded(false), _live_only(false),
      _summary_dirty(false), _next_summary_flush_ms(0), _next_event_flush_ms(0), _last_access_ms(0),
      _sample_capacity(0), _sample_head(0), _sample_count(0), _event_capacity(0), _event_head(0), _event_count(0),
      _samples(nullptr), _events(nullptr), _archive(nullptr), _restored_sample_count(0), _pending_event_count(0) {
}

StatsHistory::~StatsHistory() {
  freeHistoryBuffer(_samples);
  freeHistoryBuffer(_events);
}

void StatsHistory::begin(bool enabled, ArchiveStorage* archive) {
  _archive = archive;
  _live_only = shouldUseLiveOnlyStats();
  _degraded = _live_only;
  _psram_backed = !_live_only;
  _activated = false;
  _last_access_ms = 0;
  _enabled = enabled;
  _next_summary_flush_ms = millis() + kSummaryFlushIntervalMs;
  _next_event_flush_ms = millis() + kEventFlushIntervalMs;
  if (_enabled && shouldAutoActivateFromBoot()) {
    activate();
  }
}

void StatsHistory::setArchive(ArchiveStorage* archive) {
  _archive = archive;
}

void StatsHistory::setEnabled(bool enabled) {
  const bool was_enabled = _enabled;
  _live_only = shouldUseLiveOnlyStats();
  _degraded = _live_only;
  _psram_backed = !_live_only;

  if (!enabled) {
    _enabled = false;
    releaseBuffers();
    return;
  }

  _enabled = true;
  if (!was_enabled) {
    _activated = false;
    _last_access_ms = 0;
    _next_summary_flush_ms = millis() + kSummaryFlushIntervalMs;
    _next_event_flush_ms = millis() + kEventFlushIntervalMs;
    if (shouldAutoActivateFromBoot()) {
      activate();
    }
  }
}

bool StatsHistory::isArchiveAvailable() const {
  return _archive != nullptr && _archive->isMounted();
}

bool StatsHistory::activate() {
  if (_activated) {
    return true;
  }
  if (!ensureBuffers()) {
    return false;
  }
  _activated = true;
  if (supportsPersistence() && isArchiveAvailable()) {
    if (_sample_count == 0) {
      restoreSummaryLog();
    }
    if (_event_count == 0) {
      restoreEventsLog();
    }
  }
  for (size_t i = 0; i < _pending_event_count; ++i) {
    storeEvent(_pending_events[i], false);
  }
  return true;
}

bool StatsHistory::ensureBuffers() {
  if (_samples != nullptr && (_event_capacity == 0 || _events != nullptr)) {
    return true;
  }

  const bool want_psram = !shouldUseLiveOnlyStats();
  _live_only = !want_psram;
  _degraded = _live_only;
  _psram_backed = want_psram;

  const HistoryCapacityBucket bucket = getHistoryCapacityBucket(want_psram);
  _sample_capacity = bucket.sample_capacity;
  _event_capacity = bucket.event_capacity;
  _samples = allocHistoryBuffer<HistorySample>(_sample_capacity);
  _events = (_event_capacity > 0) ? allocHistoryBuffer<HistoryEvent>(_event_capacity) : nullptr;

  if (_samples == nullptr || (_event_capacity > 0 && _events == nullptr)) {
    freeHistoryBuffer(_samples);
    freeHistoryBuffer(_events);
    _samples = allocHistoryBuffer<HistorySample>(64);
    _events = allocHistoryBuffer<HistoryEvent>(24);
    _sample_capacity = (_samples != nullptr) ? 64 : 0;
    _event_capacity = (_events != nullptr) ? 24 : 0;
    _psram_backed = false;
    _degraded = true;
    _live_only = true;
  }

  return _samples != nullptr && (_event_capacity == 0 || _events != nullptr);
}

void StatsHistory::releaseBuffers() {
  freeHistoryBuffer(_samples);
  freeHistoryBuffer(_events);
  _samples = nullptr;
  _events = nullptr;
  _activated = false;
  _last_access_ms = 0;
  _sample_capacity = 0;
  _sample_head = 0;
  _sample_count = 0;
  _event_capacity = 0;
  _event_head = 0;
  _event_count = 0;
  _summary_dirty = false;
  _pending_event_count = 0;
  _restored_sample_count = 0;
}

bool StatsHistory::supportsPersistence() const {
  return !_live_only;
}

bool StatsHistory::shouldAutoActivateFromBoot() const {
  return _enabled && !_live_only && hasBootAutoCapturePsram();
}

bool StatsHistory::isBootAutoCaptureExpected() const {
  return shouldAutoActivateFromBoot();
}

uint32_t StatsHistory::getDetectedPsramSizeBytes() const {
#if defined(ESP32)
  return psramFound() ? ESP.getPsramSize() : 0;
#else
  return 0;
#endif
}

bool StatsHistory::isAccessActive(uint32_t now_ms) const {
  if (!_live_only) {
    return true;
  }
  return _last_access_ms != 0 && (now_ms - _last_access_ms) < kLiveOnlyIdleTimeoutMs;
}

void StatsHistory::noteAccess(uint32_t now_ms) {
  _last_access_ms = now_ms;
  if (_enabled) {
    activate();
  }
}

void StatsHistory::maybeReleaseIdleBuffers(uint32_t now_ms) {
  if (!_enabled || !_live_only || !isRecentHistoryAvailable() || isAccessActive(now_ms)) {
    return;
  }
  releaseBuffers();
}

void StatsHistory::storeSample(const HistorySample& sample, bool mark_dirty) {
  if (!ensureBuffers()) {
    return;
  }

  _samples[_sample_head] = sample;
  _sample_head = (_sample_head + 1) % _sample_capacity;
  if (_sample_count < _sample_capacity) {
    _sample_count++;
  }
  if (mark_dirty) {
    _summary_dirty = true;
  }
}

bool StatsHistory::parseSummaryLine(const char* line, HistorySample& sample) const {
  if (line == nullptr || line[0] == 0) {
    return false;
  }

  unsigned long epoch_secs = 0;
  unsigned long uptime_secs = 0;
  unsigned battery_mv = 0;
  unsigned queue_len = 0;
  int last_rssi_x4 = 0;
  int last_snr_x4 = 0;
  int noise_floor = 0;
  unsigned long packets_sent = 0;
  unsigned long packets_recv = 0;
  unsigned long heap_free = 0;
  unsigned long psram_free = 0;
  unsigned error_flags = 0;
  unsigned recv_errors = 0;
  unsigned neighbour_count = 0;
  unsigned direct_dups = 0;
  unsigned flood_dups = 0;
  unsigned flags = 0;

  int parsed = sscanf(line,
                      "%lu,%lu,%u,%u,%d,%d,%d,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u",
                      &epoch_secs,
                      &uptime_secs,
                      &battery_mv,
                      &queue_len,
                      &last_rssi_x4,
                      &last_snr_x4,
                      &noise_floor,
                      &packets_sent,
                      &packets_recv,
                      &heap_free,
                      &psram_free,
                      &error_flags,
                      &recv_errors,
                      &neighbour_count,
                      &direct_dups,
                      &flood_dups,
                      &flags);
  if (parsed != 17) {
    noise_floor = 0;
    packets_sent = 0;
    packets_recv = 0;
    heap_free = 0;
    psram_free = 0;
    error_flags = 0;
    recv_errors = 0;
    neighbour_count = 0;
    direct_dups = 0;
    flood_dups = 0;
    flags = 0;
    parsed = sscanf(line,
                    "%lu,%lu,%u,%u,%d,%d,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u",
                    &epoch_secs,
                    &uptime_secs,
                    &battery_mv,
                    &queue_len,
                    &last_rssi_x4,
                    &last_snr_x4,
                    &packets_sent,
                    &packets_recv,
                    &heap_free,
                    &psram_free,
                    &error_flags,
                    &recv_errors,
                    &neighbour_count,
                    &direct_dups,
                    &flood_dups,
                    &flags);
    if (parsed != 16) {
      return false;
    }
  }

  memset(&sample, 0, sizeof(sample));
  sample.epoch_secs = static_cast<uint32_t>(epoch_secs);
  sample.uptime_secs = static_cast<uint32_t>(uptime_secs);
  sample.packets_sent = static_cast<uint32_t>(packets_sent);
  sample.packets_recv = static_cast<uint32_t>(packets_recv);
  sample.heap_free = static_cast<uint32_t>(heap_free);
  sample.heap_min = static_cast<uint32_t>(heap_free);
  sample.psram_free = static_cast<uint32_t>(psram_free);
  sample.psram_min = static_cast<uint32_t>(psram_free);
  sample.battery_mv = static_cast<uint16_t>(battery_mv);
  sample.queue_len = static_cast<uint16_t>(queue_len);
  sample.error_flags = static_cast<uint16_t>(error_flags);
  sample.recv_errors = static_cast<uint16_t>(recv_errors);
  sample.neighbour_count = static_cast<uint16_t>(neighbour_count);
  sample.direct_dups = static_cast<uint16_t>(direct_dups);
  sample.flood_dups = static_cast<uint16_t>(flood_dups);
  sample.last_rssi_x4 = static_cast<int16_t>(last_rssi_x4);
  sample.last_snr_x4 = static_cast<int16_t>(last_snr_x4);
  sample.noise_floor = static_cast<int16_t>(noise_floor);
  sample.flags = static_cast<uint8_t>(flags);
  sample.battery_pct = -1;
  return true;
}

bool StatsHistory::restoreSummaryLog() {
  if (!supportsPersistence() || !isArchiveAvailable() || _sample_capacity == 0) {
    return false;
  }

  FILESYSTEM* fs = _archive->getFS();
  if (fs == nullptr || !fs->exists("/stats/summary.log")) {
    return false;
  }

  File file = openArchiveReadWithRecovery(_archive, "/stats/summary.log");
  if (!file) {
    ARCHIVE_LOG("summary restore open failed path=%s", "/stats/summary.log");
    return false;
  }

  _restored_sample_count = 0;
  const size_t size = static_cast<size_t>(file.size());
  const size_t start = (size > kSummaryRestoreWindowBytes) ? (size - kSummaryRestoreWindowBytes) : 0;
  if (start > 0 && !file.seek(start)) {
    file.close();
    return false;
  }

  char line[192];
  size_t line_len = 0;
  bool skip_partial = (start > 0);
  while (file.available()) {
    const int raw = file.read();
    if (raw < 0) {
      break;
    }
    const char ch = static_cast<char>(raw);
    if (skip_partial) {
      if (ch == '\n') {
        skip_partial = false;
      }
      continue;
    }
    if (ch == '\r') {
      continue;
    }
    if (ch == '\n') {
      line[line_len] = 0;
      HistorySample sample{};
      if (parseSummaryLine(line, sample)) {
        storeSample(sample, false);
        _restored_sample_count++;
      }
      line_len = 0;
      continue;
    }
    if (line_len + 1 < sizeof(line)) {
      line[line_len++] = ch;
    }
  }

  if (!skip_partial && line_len > 0) {
    line[line_len] = 0;
    HistorySample sample{};
    if (parseSummaryLine(line, sample)) {
      storeSample(sample, false);
      _restored_sample_count++;
    }
  }

  file.close();
  return _restored_sample_count > 0;
}

bool StatsHistory::parseEventLine(const char* line, HistoryEvent& event) const {
  if (line == nullptr || line[0] == 0) {
    return false;
  }

  unsigned long epoch_secs = 0;
  unsigned long uptime_secs = 0;
  char type_name[32];
  int value = 0;
  memset(type_name, 0, sizeof(type_name));

  const int parsed = sscanf(line, "%lu,%lu,%31[^,],%d", &epoch_secs, &uptime_secs, type_name, &value);
  if (parsed != 4) {
    return false;
  }

  const uint8_t type = eventTypeFromName(type_name);
  if (type == 0) {
    return false;
  }

  memset(&event, 0, sizeof(event));
  event.type = type;
  event.epoch_secs = static_cast<uint32_t>(epoch_secs);
  event.uptime_secs = static_cast<uint32_t>(uptime_secs);
  event.value = static_cast<int16_t>(value);
  return true;
}

bool StatsHistory::restoreEventsLog() {
  if (!supportsPersistence() || !isArchiveAvailable() || _event_capacity == 0) {
    return false;
  }

  FILESYSTEM* fs = _archive->getFS();
  if (fs == nullptr || !fs->exists("/stats/events.log")) {
    return false;
  }

  File file = openArchiveRead(fs, "/stats/events.log");
  if (!file) {
    return false;
  }

  const size_t size = static_cast<size_t>(file.size());
  const size_t start = (size > kEventsRestoreWindowBytes) ? (size - kEventsRestoreWindowBytes) : 0;
  if (start > 0 && !file.seek(start)) {
    file.close();
    return false;
  }

  char line[160];
  size_t line_len = 0;
  bool skip_partial = (start > 0);
  while (file.available()) {
    const int raw = file.read();
    if (raw < 0) {
      break;
    }
    const char ch = static_cast<char>(raw);
    if (skip_partial) {
      if (ch == '\n') {
        skip_partial = false;
      }
      continue;
    }
    if (ch == '\r') {
      continue;
    }
    if (ch == '\n') {
      line[line_len] = 0;
      HistoryEvent event{};
      if (parseEventLine(line, event)) {
        storeEvent(event, false);
      }
      line_len = 0;
      continue;
    }
    if (line_len + 1 < sizeof(line)) {
      line[line_len++] = ch;
    }
  }

  if (!skip_partial && line_len > 0) {
    line[line_len] = 0;
    HistoryEvent event{};
    if (parseEventLine(line, event)) {
      storeEvent(event, false);
    }
  }

  file.close();
  return _event_count > 0;
}

void StatsHistory::pushSample(const HistorySample& sample) {
  if (!_enabled || !_activated || (_live_only && !isAccessActive(millis())) || !ensureBuffers()) {
    return;
  }
  storeSample(sample, true);
}

void StatsHistory::appendPendingEvent(const HistoryEvent& event) {
  if (_pending_event_count < (sizeof(_pending_events) / sizeof(_pending_events[0]))) {
    _pending_events[_pending_event_count++] = event;
    return;
  }

  memmove(&_pending_events[0], &_pending_events[1], sizeof(_pending_events) - sizeof(_pending_events[0]));
  _pending_events[(sizeof(_pending_events) / sizeof(_pending_events[0])) - 1] = event;
}

void StatsHistory::storeEvent(const HistoryEvent& event, bool queue_pending) {
  if (_event_capacity == 0 || !ensureBuffers()) {
    return;
  }

  _events[_event_head] = event;
  _event_head = (_event_head + 1) % _event_capacity;
  if (_event_count < _event_capacity) {
    _event_count++;
  }

  if (queue_pending) {
    appendPendingEvent(event);
  }
}

void StatsHistory::recordEvent(uint8_t type, uint32_t epoch_secs, uint32_t uptime_secs, int16_t value) {
  if (!_enabled) {
    return;
  }

  HistoryEvent event{};
  event.type = type;
  event.epoch_secs = epoch_secs;
  event.uptime_secs = uptime_secs;
  event.value = value;

  if (!_activated || (_live_only && !isAccessActive(millis())) || _event_capacity == 0 || !ensureBuffers()) {
    appendPendingEvent(event);
    return;
  }
  storeEvent(event, true);
}

void StatsHistory::maybeFlush(uint32_t now_ms) {
  if (!_enabled || !_activated || !supportsPersistence() || !isArchiveAvailable()) {
    return;
  }

  if (_summary_dirty && (now_ms >= _next_summary_flush_ms || _pending_event_count >= 8)) {
    flushSummaryLog();
    _summary_dirty = false;
    _next_summary_flush_ms = now_ms + kSummaryFlushIntervalMs;
  }

  if (_pending_event_count > 0 && now_ms >= _next_event_flush_ms) {
    flushEventsLog();
    _next_event_flush_ms = now_ms + kEventFlushIntervalMs;
  }
}

void StatsHistory::flushSummaryLog() {
  if (!supportsPersistence() || _archive == nullptr || _sample_count == 0) {
    return;
  }

  FILESYSTEM* fs = _archive->getFS();
  if (fs == nullptr) {
    return;
  }

  HistorySample latest{};
  if (!getSampleFromOldest(_sample_count - 1, latest)) {
    return;
  }

  File file = openArchiveAppendWithRecovery(_archive, "/stats/summary.log");
  if (!file) {
    ARCHIVE_LOG("summary open failed path=%s", "/stats/summary.log");
    return;
  }

  char line[256];
  snprintf(line, sizeof(line),
           "%lu,%lu,%u,%u,%d,%d,%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
           static_cast<unsigned long>(latest.epoch_secs),
           static_cast<unsigned long>(latest.uptime_secs),
           static_cast<unsigned>(latest.battery_mv),
           static_cast<unsigned>(latest.queue_len),
           static_cast<int>(latest.last_rssi_x4),
           static_cast<int>(latest.last_snr_x4),
           static_cast<int>(latest.noise_floor),
           static_cast<unsigned>(latest.packets_sent),
           static_cast<unsigned>(latest.packets_recv),
           static_cast<unsigned>(latest.heap_free),
           static_cast<unsigned>(latest.psram_free),
           static_cast<unsigned>(latest.error_flags),
           static_cast<unsigned>(latest.recv_errors),
           static_cast<unsigned>(latest.neighbour_count),
           static_cast<unsigned>(latest.direct_dups),
           static_cast<unsigned>(latest.flood_dups),
           static_cast<unsigned>(latest.flags));
  const size_t written = file.print(line);
  file.flush();
  file.close();
  ARCHIVE_LOG("summary flushed path=%s bytes=%u sample_count=%u",
              "/stats/summary.log",
              static_cast<unsigned>(written),
              static_cast<unsigned>(_sample_count));
  writeMetaFile();
}

void StatsHistory::flushEventsLog() {
  if (!supportsPersistence() || _archive == nullptr || _pending_event_count == 0) {
    return;
  }

  FILESYSTEM* fs = _archive->getFS();
  if (fs == nullptr) {
    return;
  }

  File file = openArchiveAppendWithRecovery(_archive, "/stats/events.log");
  if (!file) {
    ARCHIVE_LOG("events open failed path=%s", "/stats/events.log");
    return;
  }

  size_t total_written = 0;
  for (size_t i = 0; i < _pending_event_count; ++i) {
    const HistoryEvent& event = _pending_events[i];
    char line[160];
    snprintf(line, sizeof(line),
             "%lu,%lu,%s,%d\n",
             static_cast<unsigned long>(event.epoch_secs),
             static_cast<unsigned long>(event.uptime_secs),
             getEventTypeName(event.type),
             static_cast<int>(event.value));
    total_written += file.print(line);
  }
  file.flush();
  file.close();
  ARCHIVE_LOG("events flushed path=%s bytes=%u count=%u",
              "/stats/events.log",
              static_cast<unsigned>(total_written),
              static_cast<unsigned>(_pending_event_count));
  _pending_event_count = 0;
  writeMetaFile();
}

void StatsHistory::writeMetaFile() const {
  if (!supportsPersistence() || _archive == nullptr) {
    return;
  }
  FILESYSTEM* fs = _archive->getFS();
  if (fs == nullptr) {
    return;
  }

  File file = openArchiveWriteWithRecovery(_archive, "/stats/meta.json");
  if (!file) {
    ARCHIVE_LOG("meta open failed path=%s", "/stats/meta.json");
    return;
  }

  char json[256];
  snprintf(json, sizeof(json),
           "{\"logical\":\"%s\",\"path\":\"%s\",\"samples\":{\"count\":%u,\"capacity\":%u},"
           "\"events\":{\"count\":%u,\"capacity\":%u},\"psram\":%s,\"degraded\":%s}\n",
           _archive->getLogicalName(),
           _archive->getLogicalStatsPath(),
           static_cast<unsigned>(_sample_count),
           static_cast<unsigned>(_sample_capacity),
           static_cast<unsigned>(_event_count),
           static_cast<unsigned>(_event_capacity),
           _psram_backed ? "true" : "false",
           _degraded ? "true" : "false");
  const size_t written = file.print(json);
  file.flush();
  file.close();
  ARCHIVE_LOG("meta flushed path=%s bytes=%u",
              "/stats/meta.json",
              static_cast<unsigned>(written));
}

bool StatsHistory::getSampleFromOldest(size_t index, HistorySample& sample) const {
  if (_samples == nullptr || index >= _sample_count) {
    return false;
  }
  const size_t oldest = (_sample_head + _sample_capacity - _sample_count) % _sample_capacity;
  const size_t slot = (oldest + index) % _sample_capacity;
  sample = _samples[slot];
  return true;
}

bool StatsHistory::buildSeriesJson(const char* series, char* buffer, size_t buffer_size, uint32_t now_epoch_secs, uint32_t now_uptime_secs) const {
  if (buffer == nullptr || buffer_size == 0 || series == nullptr) {
    return false;
  }
  buffer[0] = 0;
  if (seriesTitle(series)[0] == 0) {
    return false;
  }

  if (_sample_count == 0) {
    snprintf(buffer, buffer_size,
             "{\"series\":\"%s\",\"title\":\"%s\",\"unit\":\"%s\",\"interval_secs\":%lu,\"current\":0,\"oldest_age_secs\":0,\"latest_age_secs\":0,\"points\":[]}",
             series,
             seriesTitle(series),
             seriesUnit(series),
             static_cast<unsigned long>(kSampleIntervalSecs));
    return true;
  }

  const size_t points = min(_sample_count, kMaxSeriesPoints);
  const size_t step = (_sample_count > kMaxSeriesPoints) ? ((_sample_count + kMaxSeriesPoints - 1) / kMaxSeriesPoints) : 1;

  size_t offset = 0;
  HistorySample sample{};
  HistorySample previous{};
  bool have_previous = false;
  int current_value = 0;

  offset += snprintf(&buffer[offset], buffer_size - offset,
                     "{\"series\":\"%s\",\"title\":\"%s\",\"unit\":\"%s\",\"interval_secs\":%lu,\"current\":",
                     series,
                     seriesTitle(series),
                     seriesUnit(series),
                     static_cast<unsigned long>(kSampleIntervalSecs));

  getSampleFromOldest(_sample_count - 1, sample);
  if (strcmp(series, "packets") == 0 && _sample_count >= 2) {
    getSampleFromOldest(_sample_count - 2, previous);
    current_value = buildPointValue(sample, &previous, series);
  } else {
    current_value = buildPointValue(sample, nullptr, series);
  }

  size_t last_index = 0;
  for (size_t i = 0, emitted = 0; i < _sample_count && emitted < points; i += step, ++emitted) {
    last_index = i;
  }
  HistorySample oldest_sample{};
  HistorySample latest_emitted_sample{};
  const bool have_oldest_sample = getSampleFromOldest(0, oldest_sample);
  const bool have_latest_emitted_sample = getSampleFromOldest(last_index, latest_emitted_sample);
  const uint32_t oldest_age_secs = have_oldest_sample ? sampleAgeSecs(oldest_sample, now_epoch_secs, now_uptime_secs) : 0;
  const uint32_t latest_age_secs = have_latest_emitted_sample ? sampleAgeSecs(latest_emitted_sample, now_epoch_secs, now_uptime_secs) : 0;

  offset += snprintf(&buffer[offset], buffer_size - offset,
                     "%d,\"oldest_age_secs\":%lu,\"latest_age_secs\":%lu,\"points\":[",
                     current_value,
                     static_cast<unsigned long>(oldest_age_secs),
                     static_cast<unsigned long>(latest_age_secs));

  size_t emitted = 0;
  for (size_t i = 0; i < _sample_count && emitted < points; i += step, ++emitted) {
    if (!getSampleFromOldest(i, sample)) {
      break;
    }
    int value = buildPointValue(sample, have_previous ? &previous : nullptr, series);
    offset += snprintf(&buffer[offset], buffer_size - offset,
                       "%s[%lu,%d]",
                       emitted == 0 ? "" : ",",
                       static_cast<unsigned long>(sample.uptime_secs),
                       value);
    previous = sample;
    have_previous = true;
    if (offset + 24 >= buffer_size) {
      break;
    }
  }

  snprintf(&buffer[offset], buffer_size - offset, "]}");
  return true;
}

bool StatsHistory::getRecentEvent(size_t reverse_index, HistoryEvent& event) const {
  if (_events == nullptr || reverse_index >= _event_count) {
    return false;
  }

  const size_t newest = (_event_head + _event_capacity - 1) % _event_capacity;
  const size_t slot = (newest + _event_capacity - reverse_index) % _event_capacity;
  event = _events[slot];
  return true;
}

const char* StatsHistory::getEventTypeName(uint8_t type) {
  switch (type) {
    case HISTORY_EVENT_BOOT:
      return "boot";
    case HISTORY_EVENT_WEB_STARTED:
      return "web_started";
    case HISTORY_EVENT_WEB_STOPPED:
      return "web_stopped";
    case HISTORY_EVENT_MQTT_CONNECTED:
      return "mqtt_connected";
    case HISTORY_EVENT_MQTT_DISCONNECTED:
      return "mqtt_disconnected";
    case HISTORY_EVENT_ARCHIVE_MOUNTED:
      return "archive_mounted";
    case HISTORY_EVENT_ARCHIVE_UNAVAILABLE:
      return "archive_unavailable";
    case HISTORY_EVENT_LOW_MEMORY:
      return "low_memory";
    case HISTORY_EVENT_STATS_ENABLED:
      return "stats_enabled";
    case HISTORY_EVENT_STATS_DISABLED:
      return "stats_disabled";
    default:
      return "unknown";
  }
}
