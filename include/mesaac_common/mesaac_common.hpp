#pragma once

#include <string>
#include <time.h>

#ifdef WIN32
#ifdef MESAAC_COMMON_EXPORTS
#define MESAAC_COMMON_API __declspec(dllexport)
#else
#define MESAAC_COMMON_API __declspec(dllimport)
#endif // MESAAC_COMMON_EXPORTS

#else // WIN32
#define MESAAC_COMMON_API
#endif

namespace mesaac {
MESAAC_COMMON_API void initCommon(MesaacFeatures &featureSet);
MESAAC_COMMON_API ::time_t expirationDate();
MESAAC_COMMON_API std::string expirationDateStr();
} // namespace mesaac

#endif /* end of include guard: LICENSING_H_CBA5L8W4 */
