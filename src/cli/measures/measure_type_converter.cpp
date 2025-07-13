#include <map>

#include "measure_type_converter.hpp"

namespace mesaac::cli::measures {

namespace {
using namespace mesaac::measures;

static const std::map<char, MeasureType> code_to_measure_type{
    {'B', MeasureType::bub},       {'C', MeasureType::cosine},
    {'E', MeasureType::euclidean}, {'H', MeasureType::hamann},
    {'T', MeasureType::tanimoto},  {'V', MeasureType::tversky},
};
} // namespace

mesaac::measures::MeasureType get_measure_type(const char type_code) {
  try {
    return code_to_measure_type.at(type_code);
  } catch (std::out_of_range &) {
    throw std::invalid_argument("Unsupported measure -" +
                                std::string{type_code});
  }
}

} // namespace mesaac::cli::measures