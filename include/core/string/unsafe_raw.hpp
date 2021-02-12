#ifndef IOP_CORE_STRING_UNSAFE_RAW_HPP
#define IOP_CORE_STRING_UNSAFE_RAW_HPP

namespace iop {
class StringView;

/// Type-safe runtime string pointers, it's the user telling the system
/// That they should be trusted and this UnsafeRawString will not contain a NULL
/// pointer And will not outlive the internal storage
///
/// It's basically a helper class to construct a StringView from a char*
/// Without implicit conversion byting us later.
/// It has to be explicit and with a greppable ` Unsafe` in the declaration
class UnsafeRawString {
private:
  const char *str;

public:
  ~UnsafeRawString() noexcept = default;
  explicit UnsafeRawString(const char *str) noexcept;
  UnsafeRawString(const UnsafeRawString &str) noexcept = default;
  UnsafeRawString(UnsafeRawString &&str) noexcept = default;
  auto operator=(UnsafeRawString const &other) noexcept -> UnsafeRawString &;
  auto operator=(UnsafeRawString &&other) noexcept -> UnsafeRawString &;
  auto get() const noexcept -> const char *;
  auto operator*() const noexcept -> StringView;
};
} // namespace iop

#endif