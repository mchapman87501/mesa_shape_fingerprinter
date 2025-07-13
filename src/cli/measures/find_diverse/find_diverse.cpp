// For Mesa internal use only.
// Hence the lack of usage and license checking, etc.
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <libgen.h>

#include "mesaac_measures/measures.h"
#include "fingerprint_reader.h"

using namespace std;
using namespace mesaac;

static void fail(string msg="") {
    if (msg.size() > 0) {
        cerr << endl << msg << endl;
    }
    exit(1);
}

static void show_blurb(const string /*exename*/) {
}

static void show_usage(const string exename, string msg="") {
    cerr << "Usage: " << exename << " target_file database_file threshold" 
         << endl
         << "    target_file -- contains fingerprints of targets" << endl
         << "    database_file -- database of fingerprints to search" << endl
         << "    threshold -- Tanimoto similarity threshold (0..1 inclusive)"
         << endl << endl
         << "Search a database for fingerprints similar to those in target_file"
         << endl
         << "Print the 0-based indices of all target_file entries which" << endl
         << "are not similar to any database_file entries" << endl
         << endl;
    fail(msg);
}

static void show_usage(string exename, ostringstream& msg) {
    show_usage(exename, msg.str());
}

static void parse_args(int argc, char **argv,
                string& target_pathname, string& database_pathname, 
                float& threshold)
{
    string exename(basename(argv[0]));
    
    if (argc != 4) {
        show_usage(exename, "Wrong number of arguments.");
    }
    
    target_pathname = argv[1];
    database_pathname = argv[2];
    
    istringstream ins(argv[3]);
    if (!(ins >> threshold)) {
        ostringstream msg;
        msg << "Threshold ('" << argv[3] << "') is not a valid number.";
        show_usage(exename, msg);
    }
    if ((0 > threshold) || (threshold > 1)) {
        ostringstream msg;
        msg << "Threshold (" << threshold 
            << ") must be in the range 0..1 inclusive";
        show_usage(exename, msg);
    }
}

static inline bool is_unique(BitVector& target, ArrayBitVectors& database, float threshold) {
    measures m;

    ArrayBitVectors::iterator i;
    for (i = database.begin(); i != database.end(); ++i) {
        // Anything >= threshold is sufficiently similar to not be unique.
        if (m(target, *i) >= threshold) {
            return false;
        }
    }
    return true;
}

void print_diverse_targets(ostream& outs, string& target_pathname, 
                           string& database_pathname, float& threshold)
{
    FingerprintReader database_fps(database_pathname);
    ArrayBitVectors database;
    database_fps.read_all(database);
    
    FingerprintReader target_fps(target_pathname);
    BitVector target;
    unsigned int i = 0;
    while (target_fps.next(target)) {
        i++;
        if (is_unique(target, database, threshold)) {
            outs << i << endl;
        }
    }
}

int main (int argc, char *argv[])
{
    show_blurb(basename(argv[0]));
    
    string target_pathname = "";
    string database_pathname = "";
    float threshold = 0;
    parse_args(argc, argv, target_pathname, database_pathname, threshold);
    print_diverse_targets(cout, target_pathname, database_pathname, threshold);
    return 0;
}
