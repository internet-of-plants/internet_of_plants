#ifndef IOP_API_HPP
#define IOP_API_HPP

#include "core/log.hpp"
#include "core/network.hpp"
#include "core/string/fixed.hpp"
#include "core/utils.hpp"
#include "models.hpp"

#include "ArduinoJson.h"

/// High level client, that abstracts IoP API access in a safe and ergonomic way
///
/// Handles all the network internals.
///
/// If some method returns `NetworkStatus::CLIENT_BUFFER_OVERFLOW` the method is
/// broken. The user is responsible for dealing with it however they like.
class Api {
private:
  iop::Log logger;
  iop::Network network_;

public:
  Api(iop::StaticString uri, iop::LogLevel logLevel) noexcept;

  auto setup() const noexcept -> void;
  auto uri() const noexcept -> iop::StaticString;
  auto loggerLevel() const noexcept -> iop::LogLevel;
  auto network() const noexcept -> const iop::Network &;

  /// Sends a log message through the network, currently needs a buffer.
  /// Streaming is a TODO.
  ///
  /// OK: success, this won't be triggered because success returns AuthToken
  /// FORBIDDEN: auth token is invalid
  /// CONNECTION_ISSUES: problems with connection, retry later?
  /// CLIENT_BUFFER_OVERFLOW: this route shouldn't trigger this, ever
  /// BROKEN_SERVER: just wait until server is fixed
  auto reportPanic(const AuthToken &authToken,
                   const PanicData &event) const noexcept -> iop::NetworkStatus;

  /// Register frequent event to server, with measurements and device
  /// information
  ///
  /// Possible responses:
  ///
  /// OK: success
  /// FORBIDDEN: auth token is invalid
  /// CONNECTION_ISSUES: problems with the connection, retry later?
  /// CLIENT_BUFFER_OVERFLOW: something is very broken with this methods's code
  /// MUST_UPGRADE: Well, upgrade your code
  /// BROKEN_SERVER: Must wait until server is fixed
  auto registerEvent(const AuthToken &token, const Event &event) const noexcept
      -> iop::NetworkStatus;

  /// Tries to authenticate with the server getting AuthToken if succeeded
  ///
  /// OK: success, this won't be triggered because success returns AuthToken
  /// FORBIDDEN: auth token is invalid
  /// CONNECTION_ISSUES: problems with connection, retry later?
  /// CLIENT_BUFFER_OVERFLOW: something is very broken with this method's code
  /// BROKEN_SERVER: just wait until server is fixed
  auto authenticate(std::string_view username,
                    std::string_view password) const noexcept
      -> std::variant<AuthToken, iop::NetworkStatus>;

  /// Reports panicHandler message to server. Possible responses:
  ///
  /// OK: panicHandler successfully reported
  /// FORBIDDEN: auth token is invalid
  /// CONNECTION_ISSUES: problems with connection, retry later?
  /// CLIENT_BUFFER_OVERFLOW: something is very broken with this method's code
  /// BROKEN_SERVER: must wait until server is fixeds
  auto registerLog(const AuthToken &authToken,
                   std::string_view log) const noexcept -> iop::NetworkStatus;

  /// Tries to update. Restarts on success. Returns OK if no updates are
  /// available
  ///
  /// No updates being available may mean there simply isn't newer code. Or that
  /// this device was excluded from the update (it's useful to group similar
  /// devices to have the same code).
  ///
  /// TODO: In the future this will accept signed binaries.
  ///
  /// This uses an internal Update API, so we don't have much control over the
  /// return values. The server has to adapt to its quirkiness
  ///
  /// OK: success, this won't be triggered because success returns AuthToken
  /// FORBIDDEN: auth token is invalid
  /// CONNECTION_ISSUES: problems with connection, retry later?
  /// CLIENT_BUFFER_OVERFLOW: this route shouldn't trigger this, ever
  /// BROKEN_SERVER: just wait until server is fixed
  auto upgrade(const AuthToken &token) const noexcept
      -> iop::NetworkStatus;

private:
  using JsonCallback = std::function<void(JsonDocument &)>;

  /// Abstracs safe json serialization. Returns None on overflow
  ///
  /// Overflows will mean the json couldn't be generated fitting the SIZE
  /// provided. This is a critical error and probably will break the system
  ///
  /// Gets a name for logging. And a callback that actually fills the json
  template <uint16_t SIZE>
  auto makeJson(const iop::StaticString name,
                const JsonCallback func) const noexcept
      -> std::optional<iop::FixedString<SIZE>> {
    IOP_TRACE();
    
    static auto doc = iop::try_make_unique<StaticJsonDocument<SIZE>>();
    if (!doc) {
      this->logger.error(F("Unable to allocate "), std::to_string(SIZE),
                         F(" bytes at Api::makeJson for "), name);
      return std::optional<iop::FixedString<SIZE>>();
    }
    doc->clear();
    func(*doc);

    if (doc->overflowed()) {
      const auto s = std::to_string(SIZE);
      this->logger.error(F("Payload doesn't fit Json<"), s, F("> at "), name);
      return std::optional<iop::FixedString<SIZE>>();
    }

    static auto fixed = iop::FixedString<SIZE>::empty();
    fixed.clear();
    serializeJson(*doc, fixed.asMut(), fixed.size);
    //this->logger.debug(F("Json: "), *fixed);
    return std::make_optional(fixed);
  }

public:
  ~Api() noexcept;
  Api(Api const &other);
  Api(Api &&other) = delete;
  auto operator=(Api const &other) -> Api &;
  auto operator=(Api &&other) -> Api & = delete;
};

#include "utils.hpp"
#ifdef IOP_MONITOR
#undef IOP_MOCK_MONITOR
#endif
#ifndef IOP_MOCK_MONITOR
#ifndef IOP_MONITOR
#define IOP_API_DISABLED
#endif
#else
#ifndef IOP_MONITOR
#define IOP_API_DISABLED
#endif
#endif

#endif
