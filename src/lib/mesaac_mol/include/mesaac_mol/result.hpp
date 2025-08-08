#pragma once

#include <optional>
#include <string>

namespace mesaac::mol {

template <typename T, typename E> struct ResultBase {
  static ResultBase Ok(T val) {
    return ResultBase(std::move(val), std::nullopt);
  }
  static ResultBase Err(E err) {
    return ResultBase(std::nullopt, std::move(err));
  }

  bool is_ok() const { return m_value.has_value(); }

  const T value() const { return m_value.value(); }
  const E error() const { return m_error.value(); }

private:
  const std::optional<T> m_value;
  const std::optional<E> m_error;

  ResultBase(std::optional<T> value, std::optional<E> error)
      : m_value(std::move(value)), m_error(std::move(error)) {}
};

template <typename T> using Result = ResultBase<T, std::string>;

} // namespace mesaac::mol
