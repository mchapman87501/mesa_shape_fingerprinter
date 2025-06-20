// Shape fingerprint generator, suitable for use with conformers which are
// already consistently aligned.
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#ifndef _FINGERPRINTER4C7C2CAD_H_
#define _FINGERPRINTER4C7C2CAD_H_

#include "Globals.h"
#include "mesaac_mol/atom.h"
#include "mesaac_shape/vol_box.h"

namespace mesaac {
    namespace shape {

        class Fingerprinter {
        public:
            Fingerprinter(const VolBox& volbox);
            virtual ~Fingerprinter();
            
            void compute(const mol::AtomVector& atoms,
                ShapeFingerprint& result);

        protected:
            const VolBox& m_volbox;
            
            void compute_for_flip(const PointList& points,
                                  unsigned int i_flip, Fingerprint& result);

        private:
            Fingerprinter(const Fingerprinter& src);
            Fingerprinter& operator=(const Fingerprinter& src);
        };

    } // namespace shape
} // namespace mesaac
#endif // _FINGERPRINTER4C7C2CAD_H_
