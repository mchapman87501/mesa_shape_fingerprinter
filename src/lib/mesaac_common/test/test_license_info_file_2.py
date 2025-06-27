#!/usr/bin/env python
"""Nose unit tests for license_info_file.
   Copyright (c) 2008 Mesa Analytics & Computing, Inc.  All rights reserved
"""

import sys
import os
from os.path import abspath, dirname
sys.path.insert(0, dirname(dirname(abspath(__file__))))
import logging
import re
import subprocess
from contextlib import contextmanager
import datetime
import unittest

class Error(Exception): pass

_thisdir = os.path.abspath(os.path.dirname(__file__))
def _relpath(p):
    return os.path.abspath(os.path.join(_thisdir, p))


def getCmdOutput(args, returnStatus=False):
    stdout = stderr = ""
    status = -1
    try:
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()
        status = p.wait()
        if status and not returnStatus:
            raise Error("Status %d for args %s, stderr=%s" % (status, args, stderr))
        if not returnStatus:
            return stdout
    except Exception, info:
        logging.error("Error while running %s: %s" % (args, info))
        
    return status, stdout, stderr
    
LICENSE_INFO_FILE_CMD = os.environ.get("EXE", _relpath("license_info_file_cmd"))
def getOutput(option, returnStatus=False):
    return getCmdOutput([LICENSE_INFO_FILE_CMD, option], returnStatus=returnStatus)
    
def absPathname():
    return getOutput("pathname")
    
def validationMsg():
    return getOutput("validate", True)
    
def genLicense(pathname, valid, lineEnding="\n", extraLines=0):
    """Generate a valid or bogus license with the provided lineEnding."""
    if valid:
        today = datetime.datetime.today()
        content = getCmdOutput(["../gen_license/gen_license", 
                                str(today.year + 1), str(today.month), 
                                "Mitch Chapman", "saesar", "fingerprinter"])
    else:
        content = "Invalid Owner\nInvalid License Str\n"
    lines = content.splitlines()
    with open(pathname, "w") as outf:
        outf.write(lineEnding.join(lines) + lineEnding * extraLines)
        
@contextmanager
def license_file_hider():
    def hideFiles(pathnames):
        for p in pathnames:
            hidden = p + ".hidden"
            if os.path.exists(p):
                os.rename(p, hidden)
    def unhideFiles(pathnames):
        for p in pathnames:
            hidden = p + ".hidden"
            if os.path.exists(hidden):
                try:
                    os.rename(hidden, p)
                except Exception, info:
                    sys.stderr.write("Could not unhide %s: %s" % (hidden, info))
                
    basename = getOutput("basename")
    MESAAC_LICENSE = os.environ.get("MESAAC_LICENSE", None)
    
    MESAAC_DIR = os.environ.get("MESAAC_DIR", os.path.expanduser("~/mesaac"))
    if not os.path.isdir(MESAAC_DIR):
        logging.warn("MESAAC_DIR (%s) does not exist -- trying to create it." %
                     (MESAAC_DIR,))
        os.makedirs(MESAAC_DIR)
    pathnames = [basename,
                 os.path.join(os.getcwd(), basename),
                 os.path.expanduser(os.path.join("~", basename)),
                 os.path.join(MESAAC_DIR, basename)]
    if MESAAC_LICENSE is not None:
        pathnames.append(MESAAC_LICENSE)
        
    try:
        hideFiles(pathnames)
        yield pathnames
    except:
        unhideFiles(pathnames)
        raise
    else:
        unhideFiles(pathnames)
        
class TestCase(unittest.TestCase):
    def testGetBasename(self):
        basename = getOutput("basename")
        self.assertEqual(basename, ".mesaac_license")
        
    def _testGetPathname(self, p, lineEnding, extraLines):
        # logging.debug(" TEST %s, %r, %s" % 
        #               (p, lineEnding, extraLines))

        # Generate a valid license, confirm it is valid;
        # Generate an invalid license, confirm it is not valid.
        genLicense(p, True, lineEnding=lineEnding, extraLines=extraLines)
        self.assertEqual(absPathname(), os.path.abspath(p))
        info = getOutput("license_info")
        os.remove(p)
        # logging.debug("Valid info: %s" % info)
        self.assertTrue("Valid: Yes" in info, "Should be 'Valid: Yes': %s" % info)
        match = re.search(r"Pathname: (.*)$", info)
        self.assertTrue(match is not None)
        self.assertEqual(match.group(1), os.path.abspath(p))

        # Generate invalid license info.
        genLicense(p, False, lineEnding=lineEnding, extraLines=extraLines)
        self.assertEqual(absPathname(), os.path.abspath(p))
        info = getOutput("license_info")
        os.remove(p)
        self.assertTrue("Valid: No" in info, "Should be 'Valid: No': %s" % info)

        
    def testGetPathnames(self):
        # XXX FIX THIS: this test should work regardless of whether or not
        # MESAAC_DIR is defined.
        os.environ.setdefault("MESAAC_DIR", os.path.expanduser("~"))
        
        with license_file_hider() as hiddenPathnames:
            # logging.debug("ALL PATHNAMES: %s" % str(hiddenPathnames))
            for p in hiddenPathnames:
                # Verify that platform-specific line endings, and zero or more
                # trailing line endings, cause no problems for the license
                # parser.
                for lineEnding in ["\n", "\r\n"]:
                    for extraLines in [0, 1, 2]:
                        yield self._testGetPathname, p, lineEnding, extraLines
    
    def _testOneValidation(self, p, lineEnding, extraLines):
        # logging.debug(" VALIDATE %s, %r, %s" % 
        #               (p, lineEnding, extraLines))

        # Generate a valid license, confirm it is valid;
        genLicense(p, True, lineEnding=lineEnding, extraLines=extraLines)
        status, stdout, stderr = validationMsg()
        os.remove(p)
        self.assertEqual(status, 0)
        self.assertTrue("Valid: Yes" in stdout)
        self.assertEqual(stderr, "")

        # Generate an invalid license, confirm it is not valid.
        genLicense(p, False, lineEnding=lineEnding, extraLines=extraLines)
        status, stdout, stderr = validationMsg()
        os.remove(p)

        self.assertTrue(status != 0)
        self.assertEqual(stdout, "Pathname: %s" % os.path.abspath(p))
        # Verify that the error message includes the absolute license path.
        expErrFrag = "License: %s" % os.path.abspath(p)
        self.assertTrue(expErrFrag in stderr)
        
    def testValidation(self):
        # XXX FIX THIS: this test should work regardless of whether or not
        # MESAAC_DIR is defined.
        os.environ.setdefault("MESAAC_DIR", os.path.expanduser("~"))
        
        with license_file_hider() as hiddenPathnames:
            for p in hiddenPathnames:
                # Verify that valid licenses pass, and invalid licenses fail
                # with appropriate license file pathnames.
                for lineEnding in ["\n", "\r\n"]:
                    for extraLines in [0, 1, 2]:
                        yield (self._testOneValidation, p, lineEnding,
                               extraLines)
    
    def _restoreEnv(self, varname, oldValue):
        if oldValue is None:
            try:
                del os.environ[varname]
            except KeyError:
                pass
        else:
            os.environ[varname] = oldValue
            
    def testMesaacLicense(self):
        # Test using a license file specified by MESAAC_LICENSE env var.
        oldML = os.environ.get("MESAAC_LICENSE", None)
        try:
            pathname = _relpath("custom_license.txt")
            os.environ["MESAAC_LICENSE"] = pathname
            # MESAAC_LICENSE should override any settings for MESAAC_DIR
            with license_file_hider() as hiddenPathnames:
                for p in hiddenPathnames:
                    # Verify that valid licenses pass, and invalid licenses fail
                    # with appropriate license file pathnames.
                    for lineEnding in ["\n", "\r\n"]:
                        for extraLines in [0, 1, 2]:
                            yield (self._testOneValidation, p, lineEnding,
                                   extraLines)
            
        finally:
            # Don't screw up other tests.
            self._restoreEnv("MESAAC_LICENSE", oldML)
       
    def _testOneExpiration(self, p, monthsRemaining):
        today = datetime.date.today()
        eYear = today.year
        eMonth = today.month + monthsRemaining
        if eMonth > 12:
            eYear += 1
            eMonth -= 12
        elif eMonth < 1:
            eYear -= 1
            eMonth += 12
        content = getCmdOutput(["../gen_license/gen_license", 
                                str(eYear), str(eMonth), 
                                "Mitch Chapman", "saesar", "fingerprinter"])
        with open(p, "w") as outf:
            outf.write(content)
            
        status, stdout, stderr = validationMsg()
        os.remove(p)
        
        if monthsRemaining >= 1:
            self.assertEqual(status, 0)
            self.assertTrue("Valid: Yes" in stdout)
            self.assertEqual(stderr, "")
        else:
            # How many days *should* be remaining?  Boy, this is fraught
            # with peril...  The license should expire on the first day of
            # the month after eMonth.
            eDay = 1
            eMonth += 1
            if eMonth > 12:
                eYear += 1
                eMonth = 1
            expirationDate = datetime.date(eYear, eMonth, eDay)
            daysRemaining = (expirationDate - today).days - 1
            if (daysRemaining >= 0):
                # So much for software localization...
                msg = ("will expire in %s days" % daysRemaining)
                if not (msg in stderr):
                    logging.error("Stdout: %s" % stdout)
                    logging.error("Stderr: %s" % stderr)
                    self.assertTrue(False, "Expected %r to be in %r" % (msg, stderr))
                self.assertEqual(status, 0)
                self.assertTrue("Valid: Yes" in stdout)
            else:
                # Should have gotten a license expiration error.
                self.assertTrue(status != 0)
                self.assertTrue("has expired" in stderr)
                # The license checker should have exited without printing
                # the "Valid: Yes" message.
                self.assertTrue(not ("Valid: Yes" in stdout))
            
    def testExpirationWarning(self):
        # XXX FIX THIS: this test should work regardless of whether or not
        # MESAAC_DIR is defined.
        os.environ.setdefault("MESAAC_DIR", os.path.expanduser("~"))
        with license_file_hider() as hiddenPathnames:
            for p in hiddenPathnames:
                # Verify that license validation generates warnings
                # if license will expire soon.
                for monthsRemaining in [-3, -1, 0, 1, 3]:
                    yield (self._testOneExpiration, p, monthsRemaining)
                    
    def testMissingLicenseFile(self):
        os.environ.setdefault("MESAAC_DIR", os.path.expanduser("~"))
        # Hide all license files, then try to validate.
        with license_file_hider() as hiddenPathnames:
            status, stdout, stderr = validationMsg()
            # Status should still be zero.  Until we remove the 'old-style'
            # fallback, missing licenses are a warning, not an error.
            # logging.debug(stderr)
            self.assertEqual(status, 0)
            # For perpetual licenses no warning of missing license file
            # should be displayed.
            envPerpetual = os.environ.get("PERPETUAL", "-1")
            perpetual = bool(int(os.environ.get("PERPETUAL", "0")))
            warningMsg = "Could not find Mesa license file"
            self.assertNotEqual(perpetual, warningMsg in stderr)
            self.assertTrue("Valid: Yes" in stdout)
            

def main():
    logging.basicConfig(level=logging.DEBUG,
                        format="%(levelname)s: %(relativeCreated)d: %(message)s")
    unittest.main()

if __name__ == "__main__":
    main()
