// Test support program, exposes license_info_file functions via the
// command line.
// Copyright (c) 2008 Mesa Analytics & Computing, Inc.  All rights reserved

#include <string>
#include <iostream>

#include "license_info_file.h"
#include "mesaac_common/mesaac_common.h"

using namespace std;
using namespace mesaac;

int main (int argc, char const *argv[])
{
    if (argc > 1) {
        string param(argv[1]);
        if (param == "basename") {
            cout << license_info_file::getBasename();
        } else if (param == "pathname") {
            cout << license_info_file::getPathname();
        } else if (param == "license_info") {
            LicenseInfo *info = 0;
            string error;
            license_info_file::getLicenseInfo(&info, error);
            bool isValid = false;
            if (info) {
                isValid = info->isValid();
            }
            cout << "Valid: " << (isValid ? "Yes" : "No") << " "
                 << "Error: " << error << " "
                 << "Pathname: " << (info ? info->pathname() : "");
            delete info;
        } else if (param == "validate") {
            cout << "Pathname: " << license_info_file::getPathname();
            // Simulate a Mesa command-line utility.
            mesaac::MesaacFeatures f;
            f.set(mesaac::MFE_FingerprinterModule);
            mesaac::initCommon(f);
            // Indicate license check survival by outputting an additional
            // message.
            cout << " Valid: Yes";
        }
    }
    cout.flush();
    return 0;
}
