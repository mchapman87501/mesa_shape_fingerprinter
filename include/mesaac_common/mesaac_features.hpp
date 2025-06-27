// Describes Mesa software available for sale.
// Copyright (c) 2008 Mesa Analytics & Computing, Inc.  All rights reserved
//

#ifndef _MESAAC_FEATURES48E641D8_H_
#define _MESAAC_FEATURES48E641D8_H_

#include <bitset>
#include <string>

namespace mesaac {
    typedef enum {
        MFE_GroupingModule,
        MFE_ParallelGroupingModule,
        MFE_AllOtherGroupingModule,
        MFE_FingerprinterModule,
        MFE_SAESAR,
        MFE_Miscellaneous,
        MFE_Len
    } MesaacFeaturesEnum;
    typedef std::bitset<MFE_Len> MesaacFeatures;
    
    namespace features {
        std::string getFeatureNames();
        std::string getFeatureName(MesaacFeaturesEnum code);
        MesaacFeaturesEnum getFeatureCode(std::string name);
        std::string toString(MesaacFeatures& featureSet);
    }
} // namespace mesaac
#endif // _MESAAC_FEATURES48E641D8_H_
