#pragma once

#include "mesaac_arg_parser/option_base.hpp"

namespace mesaac::arg_parser {
// Multi-valued option:
template <typename ValueType, int NumValues>
struct MultiValuedOption : public OptionBase<ValueType, NumValues> {
  using Ptr = std::shared_ptr<MultiValuedOption<ValueType, NumValues>>;
  static Ptr create(std::string_view short_name, std::string_view name,
                    std::string_view help) {
    return std::make_shared<MultiValuedOption<ValueType, NumValues>>(
        short_name, name, help);
  }
  MultiValuedOption(std::string_view short_name, std::string_view name,
                    std::string_view help)
      : OptionBase<ValueType, NumValues>(short_name, name, help) {}

  bool has_values() const { return this->m_value.has_value(); }
  std::array<ValueType, NumValues> values() const {
    return this->m_value.value();
  }
};

} // namespace mesaac::arg_parser