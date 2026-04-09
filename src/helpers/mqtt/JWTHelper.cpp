#include "JWTHelper.h"

#ifdef WITH_MQTT_UPLINK

#include <ctype.h>
#include <ed_25519.h>
#include <mbedtls/base64.h>
#include <string.h>

size_t JWTHelper::base64UrlEncode(const uint8_t* input, size_t input_len, char* output, size_t output_size) {
  if (input == nullptr || output == nullptr || output_size < 2) {
    return 0;
  }

  size_t encoded_len = 0;
  if (mbedtls_base64_encode(reinterpret_cast<unsigned char*>(output), output_size - 1, &encoded_len, input,
                            input_len) != 0) {
    return 0;
  }

  for (size_t i = 0; i < encoded_len; ++i) {
    if (output[i] == '+') {
      output[i] = '-';
    } else if (output[i] == '/') {
      output[i] = '_';
    }
  }

  while (encoded_len > 0 && output[encoded_len - 1] == '=') {
    encoded_len--;
  }
  output[encoded_len] = 0;
  return encoded_len;
}

bool JWTHelper::createAuthToken(const mesh::LocalIdentity& identity, const char* audience, time_t issued_at,
                                time_t expires_at, char* token, size_t token_size, const char* owner,
                                const char* email) {
  if (audience == nullptr || token == nullptr || token_size < 32 || issued_at <= 0 || expires_at <= issued_at) {
    return false;
  }

  static const char header_json[] = "{\"alg\":\"Ed25519\",\"typ\":\"JWT\"}";
  char header[96];
  size_t header_len = base64UrlEncode(reinterpret_cast<const uint8_t*>(header_json), strlen(header_json), header,
                                      sizeof(header));
  if (header_len == 0) {
    return false;
  }

  char public_key_hex[65];
  mesh::Utils::toHex(public_key_hex, identity.pub_key, PUB_KEY_SIZE);
  for (size_t i = 0; public_key_hex[i] != 0; ++i) {
    public_key_hex[i] = toupper(static_cast<unsigned char>(public_key_hex[i]));
  }

  char payload_json[384];
  int payload_json_len = snprintf(payload_json, sizeof(payload_json),
                                  "{\"publicKey\":\"%s\",\"aud\":\"%s\",\"iat\":%lu,\"exp\":%lu",
                                  public_key_hex, audience, static_cast<unsigned long>(issued_at),
                                  static_cast<unsigned long>(expires_at));
  if (payload_json_len <= 0 || static_cast<size_t>(payload_json_len) >= sizeof(payload_json)) {
    return false;
  }
  if (owner != nullptr && owner[0] != 0) {
    payload_json_len += snprintf(payload_json + payload_json_len, sizeof(payload_json) - payload_json_len,
                                 ",\"owner\":\"%s\"", owner);
  }
  if (email != nullptr && email[0] != 0) {
    payload_json_len += snprintf(payload_json + payload_json_len, sizeof(payload_json) - payload_json_len,
                                 ",\"email\":\"%s\"", email);
  }
  if (payload_json_len <= 0 || static_cast<size_t>(payload_json_len) + 2 >= sizeof(payload_json)) {
    return false;
  }
  payload_json[payload_json_len++] = '}';
  payload_json[payload_json_len] = 0;

  char payload[384];
  size_t payload_len = base64UrlEncode(reinterpret_cast<const uint8_t*>(payload_json), payload_json_len, payload,
                                       sizeof(payload));
  if (payload_len == 0) {
    return false;
  }

  char signing_input[512];
  int signing_input_len = snprintf(signing_input, sizeof(signing_input), "%s.%s", header, payload);
  if (signing_input_len <= 0 || static_cast<size_t>(signing_input_len) >= sizeof(signing_input)) {
    return false;
  }

  uint8_t signature[64];
  identity.sign(signature, reinterpret_cast<const uint8_t*>(signing_input), signing_input_len);

  char signature_hex[129];
  for (size_t i = 0; i < sizeof(signature); ++i) {
    snprintf(&signature_hex[i * 2], 3, "%02X", signature[i]);
  }
  signature_hex[128] = 0;

  int total_len = snprintf(token, token_size, "%s.%s", signing_input, signature_hex);
  return total_len > 0 && static_cast<size_t>(total_len) < token_size;
}

#endif
