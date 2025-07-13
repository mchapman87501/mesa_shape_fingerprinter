#include <sstream>

#include "mesaac_measures/measures_factory.hpp"

#include "mesaac_measures/bub.hpp"
#include "mesaac_measures/cosine.hpp"
#include "mesaac_measures/euclidean.hpp"
#include "mesaac_measures/hamann.hpp"
#include "mesaac_measures/measures_base.hpp"
#include "mesaac_measures/tanimoto.hpp"
#include "mesaac_measures/tversky.hpp"

namespace mesaac::measures {

MeasuresBase::Ptr get_measures(const MeasureType measure_type,
                               const float tversky_alpha) {
  switch (measure_type) {
  case MeasureType::bub:
    return std::make_shared<BUB>();

  case MeasureType::cosine:
    return std::make_shared<Cosine>();

  case MeasureType::euclidean:
    return std::make_shared<Euclidean>();

  case MeasureType::hamann:
    return std::make_shared<Hamann>();

  case MeasureType::tanimoto:
    return std::make_shared<Tanimoto>();

  case MeasureType::tversky:
    return std::make_shared<Tversky>(tversky_alpha);
  }

  auto i_measure_type =
      static_cast<std::underlying_type_t<MeasureType>>(measure_type);
  std::ostringstream msg;
  msg << "Unsupported MeasureType value " << i_measure_type;
  throw std::invalid_argument(msg.str());
}

} // namespace mesaac::measures
