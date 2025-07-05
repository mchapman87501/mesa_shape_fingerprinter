#pragma once

#include "mesaac_arg_parser/option_base.hpp"

namespace mesaac::arg_parser {

// Single-valued option:
template <typename ValueType> struct Option : public OptionBase<ValueType, 1> {
  using Ptr = std::shared_ptr<Option<ValueType>>;
  static Ptr create(std::string_view short_name, std::string_view name,
                    std::string_view help) {
    return std::make_shared<Option<ValueType>>(short_name, name, help);
  }
  Option(std::string_view short_name, std::string_view name,
         std::string_view help)
      : OptionBase<ValueType, 1>(short_name, name, help) {}

  bool has_value() const { return this->m_value.has_value(); }
  ValueType value() const { return this->m_value.value()[0]; }
  ValueType value_or(const ValueType &default_value) {
    if (this->m_value.has_value()) {
      return this->m_value.value()[0];
    }
    return default_value;
  }
};

} // namespace mesaac::arg_parser