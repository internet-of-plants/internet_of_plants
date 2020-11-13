#include <utils.hpp>
#include <models.hpp>

#include <Arduino.h>
#include <esp8266_peri.h>
#include <string>
#include <string.h>
#include <WString.h>
#include <bits/basic_string.h>
#include <MD5Builder.h>

namespace utils {
  // FNV hash
  uint64_t hashString(const StringView string) {
    const auto bytes = reinterpret_cast<const uint8_t*>(string.get());
    const uint64_t p = 16777619;
    uint64_t hash = 2166136261;

    for (uint32_t i = 0; i < string.length(); ++i) {
      hash = (hash ^ bytes[i]) * p;
    }

    hash += hash << 13;
    hash ^= hash >> 7;
    hash += hash << 3;
    hash ^= hash >> 17;
    hash += hash << 5;
    return hash;
  }
}