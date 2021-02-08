#ifndef IOP_MODELS_HPP
#define IOP_MODELS_HPP

#include "storage.hpp"
#include "tracer.hpp"

#include <cstdint>

// Those are basically utils::Storage but typesafe
TYPED_STORAGE(AuthToken, 64);
TYPED_STORAGE(NetworkName, 32);
TYPED_STORAGE(NetworkPassword, 64);
TYPED_STORAGE(MD5Hash, 32);
TYPED_STORAGE(MacAddress, 17);

class Log;
class CowString;

// TODO: move this
namespace utils {
auto hashSketch() noexcept -> const MD5Hash &;
auto macAddress() noexcept -> const MacAddress &;
void ICACHE_RAM_ATTR scheduleInterrupt(InterruptEvent ev) noexcept;
auto ICACHE_RAM_ATTR descheduleInterrupt() noexcept -> InterruptEvent;
void logMemory(const Log &logger) noexcept;
auto scapeNonPrintable(StringView msg) noexcept -> CowString;
} // namespace utils

struct PanicData {
  // TODO: this could have a StaticString alternative to be able to use `_P`
  // PROGMEM methods
  StringView msg;
  StaticString file;
  uint32_t line;
  StringView func;
};

class WifiCredentials {
public:
  NetworkName ssid;
  NetworkPassword password;

  ~WifiCredentials() noexcept = default;
  WifiCredentials(NetworkName ssid, NetworkPassword pass) noexcept
      : ssid(std::move(ssid)), password(std::move(pass)) {}
  WifiCredentials(const WifiCredentials &cred) noexcept = default;
  WifiCredentials(WifiCredentials &&cred) noexcept = default;
  auto operator=(const WifiCredentials &cred) noexcept
      -> WifiCredentials & = default;
  auto operator=(WifiCredentials &&cred) noexcept
      -> WifiCredentials & = default;
};

struct EventStorage {
  float airTemperatureCelsius;
  float airHumidityPercentage;
  float airHeatIndexCelsius;
  uint16_t soilResistivityRaw;
  float soilTemperatureCelsius;
};

// TODO: Remove this and use EventStorage directly
class Event {
public:
  EventStorage storage;
  ~Event() noexcept { IOP_TRACE(); }
  explicit Event(EventStorage storage) noexcept : storage(storage) {
    IOP_TRACE();
  }
  Event(Event const &ev) noexcept : storage(ev.storage) { IOP_TRACE(); }
  Event(Event &&ev) noexcept : storage(ev.storage) { IOP_TRACE(); }
  auto operator=(Event const &ev) noexcept -> Event & {
    IOP_TRACE();
    if (this == &ev)
      return *this;
    this->storage = ev.storage;
    return *this;
  }
  auto operator=(Event &&ev) noexcept -> Event & {
    IOP_TRACE();
    this->storage = ev.storage;
    return *this;
  }
};

#endif
