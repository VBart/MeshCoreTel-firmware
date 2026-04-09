#pragma once

#ifdef WITH_MQTT_UPLINK

#include <Arduino.h>
#include <Identity.h>

class JWTHelper {
public:
  static bool createAuthToken(const mesh::LocalIdentity& identity, const char* audience, time_t issued_at,
                              time_t expires_at, char* token, size_t token_size, const char* owner = nullptr,
                              const char* email = nullptr);

private:
  static size_t base64UrlEncode(const uint8_t* input, size_t input_len, char* output, size_t output_size);
};

#endif
