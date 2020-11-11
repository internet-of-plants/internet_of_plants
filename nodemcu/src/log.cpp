#include <log.hpp>

void Log::trace(const StaticString & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(TRACE, msg, logType, lineTermination.get());
}

void Log::debug(const StaticString & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(DEBUG, msg, logType, lineTermination.get());
}

void Log::info(const StaticString & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(INFO, msg, logType, lineTermination.get());
}

void Log::warn(const StaticString & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(WARN, msg, logType, lineTermination.get());
}

void Log::error(const StaticString & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(ERROR, msg, logType, lineTermination.get());
}

void Log::crit(const StaticString & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(CRIT, msg, logType, lineTermination.get());
}

void Log::trace(const String & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(TRACE, msg.c_str(), logType, lineTermination.get());
}

void Log::debug(const String & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(DEBUG, msg.c_str(), logType, lineTermination.get());
}

void Log::info(const String & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(INFO, msg.c_str(), logType, lineTermination.get());
}

void Log::warn(const String & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(WARN, msg.c_str(), logType, lineTermination.get());
}

void Log::error(const String & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(ERROR, msg.c_str(), logType, lineTermination.get());
}

void Log::crit(const String & msg, const enum LogType logType, const StaticString lineTermination) const {
  log(CRIT, msg.c_str(), logType, lineTermination.get());
}

#ifndef IOP_LOG_DISABLED
#include <Arduino.h>

void Log::setup() const {
  Serial.begin(9600);
  this->info(STATIC_STRING("Setup"));
}

void Log::printLogType(const enum LogType logType, const LogLevel level) const {
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
      Serial.print(FPSTR(this->targetLogger.get()));
      Serial.print(F(": "));
  };
}

void Log::log(const enum LogLevel level, const StaticString msg, const enum LogType logType, const char * const lineTermination) const {
  if (this->logLevel > level) return;
  if (this->flush) Serial.flush();
  this->printLogType(logType, level);
  Serial.print(msg.get());
  // TODO: We can't do that right now because we depend on StaticString to emulate a StringView for now
  // We should use a technique like of F and FPTR, using this helper class to only allow construction
  // of StaticString from PROCMEM
  //Serial.print(FPSTR(msg.get()));
  Serial.print(FPSTR(lineTermination));
  if (this->flush) Serial.flush();
}

void Log::log(const enum LogLevel level, const char * const msg, const enum LogType logType, const char * const lineTermination) const {
  if (this->logLevel > level) return;
  if (this->flush) Serial.flush();
  this->printLogType(logType, level);
  Serial.print(msg);
  Serial.print(FPSTR(lineTermination));
  if (this->flush) Serial.flush();
}
#endif

#ifdef IOP_LOG_DISABLED
void Log::setup() const {}
void Log::log(const enum LogLevel level, const char * const msg, const enum LogType logType, const char * const lineTermination) const {
  (void) level;
  (void) msg;
  (void) logType;
  (void) lineTermination;
}
void Log::log(const enum LogLevel level, const StaticString msg, const enum LogType logType, const char * const lineTermination) const {
  (void) level;
  (void) msg;
  (void) logType;
  (void) lineTermination;
}
#endif
