#include "log.hpp"

#ifndef IOP_LOG_DISABLED
#include <Arduino.h>

void Log::setup() const noexcept {
  Serial.begin(9600);
  const auto end = millis() + 30000;
  while (!Serial && millis() < end)
    yield();
  this->infoln(F("Setup"));
}

void Log::printLogType(const enum LogType logType,
                       const LogLevel level) const noexcept {
  switch (logType) {
  case CONTINUITY:
    break;
  case START:
    Serial.print(F("["));
    switch (level) {
    case TRACE:
      Serial.print(F("TRACE"));
      break;
    case DEBUG:
      Serial.print(F("DEBUG"));
      break;
    case INFO:
      Serial.print(F("INFO"));
      break;
    case WARN:
      Serial.print(F("WARN"));
      break;
    case ERROR:
      Serial.print(F("ERROR"));
      break;
    case CRIT:
      Serial.print(F("CRIT"));
      break;
    }
    Serial.print(F("] "));
    Serial.print(this->targetLogger.get());
    Serial.print(F(": "));
  };
}

void Log::log(const enum LogLevel level, const StaticString msg,
              const enum LogType logType,
              const StaticString lineTermination) const noexcept {
  if (this->logLevel > level)
    return;
  if (this->flush)
    Serial.flush();
  this->printLogType(logType, level);
  Serial.print(msg.get());
  Serial.print(lineTermination.get());
  if (this->flush)
    Serial.flush();
}

void Log::log(const enum LogLevel level, const StringView msg,
              const enum LogType logType,
              const StaticString lineTermination) const noexcept {
  if (this->logLevel > level)
    return;
  if (this->flush)
    Serial.flush();
  this->printLogType(logType, level);
  Serial.print(msg.get());
  Serial.print(lineTermination.get());
  if (this->flush)
    Serial.flush();
}
#endif

#ifdef IOP_LOG_DISABLED
void Log::setup() const {}
void Log::printLogType(const enum LogType logType,
                       const LogLevel level) const noexcept {
  (void)logType;
  (void)level;
}
void Log::log(const enum LogLevel level, const StringView msg,
              const enum LogType logType,
              const StaticString lineTermination) const noexcept {
  (void)level;
  (void)msg;
  (void)logType;
  (void)lineTermination;
}
void Log::log(const enum LogLevel level, const StaticString msg,
              const enum LogType logType,
              const StaticString lineTermination) const noexcept {
  (void)level;
  (void)msg;
  (void)logType;
  (void)lineTermination;
}
#endif
