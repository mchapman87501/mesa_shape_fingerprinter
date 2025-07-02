// Filter an SD conformer file, writing only those conformers whose maximum
// extent lies within a threshold.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved

#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <libgen.h>
#include "mesaac_common.h"
#include "mesaac_mol.h"
#include "mesaac_shape.h"

using namespace std;
using namespace mesaac;

string Version = "1.0";
string CreationDate = "December 21, 2010";

class ArgParser {
public:
    ArgParser(int argc, char **argv): m_argc(argc), m_argv(argv) {
        
    }
    virtual ~ArgParser() {}
    
    void show_blurb() {
        cerr << "Running " << basename(m_argv[0]) << endl
            << "Source code Copyright (c) 2010" << endl 
            << "Mesa Analytics & Computing, Inc." << endl
            << "Version " << Version 
            << " Creation Date: " << CreationDate << endl
            << "Expiration Date: " << mesaac::expirationDateStr() 
            << endl;
    }
    
    void parse_args(ifstream& sdff, string& sdf_pathname, 
                    float& atom_scale, float& max_radius)
    {
        int i = 1;
        while (i < m_argc) {
            const string curr_opt(m_argv[i]);
            if (curr_opt == "-h" || curr_opt == "--help") {
                show_help();
            } else if (curr_opt.substr(1, 1) != "-") {
                break;
            } else {
                ostringstream msg;
                msg << "Invalid option '" << curr_opt << "'.";
                error(msg);
            }
            ++i;
        }
        
        string sphere_pathname;
        get_ifstream(i, "sd_file", sdff, sdf_pathname);
        get_arg(i, "atom_scale", atom_scale);
        if (atom_scale <= 0.0) {
            error("atom_scale must be > 0.0");
        }
        get_arg(i, "max_radius", max_radius);
        if (max_radius <= 0.0) {
            error("max_radius must be > 0.0");
        }
    }
    
protected:
    int m_argc;
    char **m_argv;
    
    void show_usage() {
        cerr << "Usage: " << basename(m_argv[0])
             << " [options] sd_file atom_scale max_radius\n\
Print conformers from sd_file whose major axis radius is <= max_radius.\n\
\n\
sd_file           = a file of conformers in SD format, with 3D coordinates\n\
atom_scale        = the amount, in the range [1.0..2.0], by which to increase\n\
                    atom radii for alignment\n\
max_radius        = only molecules which do not exceed max_radius will be\n\
                    written to stdout\n\
\n\
Options:\n\
-h | --help       = Show this help message and exit"
            << endl
            ;
    }
    
    void show_help() {
        show_usage();
        exit(0);
    }
    
    void error(string msg) {
        cerr << msg << endl << endl
            << "Use \"" << basename(m_argv[0]) << " --help\" for usage details."
            << endl;
        exit(1);
    }
    
    void error(ostringstream& outs) {
        error(outs.str());
    }
    
    void get_arg(int& i, string name, string& value) {
        if (i >= m_argc) {
            ostringstream msg;
            msg << "Missing value for " << name;
            error(msg);
        }
        value = m_argv[i++];
    }
    
    void get_arg(int& i, string name, float& value) {
        string sval;
        get_arg(i, name, sval);
        istringstream ins(sval);
        if (!(ins >> value)) {
            ostringstream msg;
            msg << "Value for " << name << " -- " << sval 
                << " -- is not a valid number.";
            error(msg);
        }
    }
    
    void get_ifstream(int& i, string name, ifstream& inf, string& pathname) {
        get_arg(i, name, pathname);
        inf.open(pathname.c_str());
        if (!inf.good()) {
            ostringstream msg;
            msg << "Cannot open " << name << " file for reading: " << pathname;
            error(msg);
        }
    }
};


// Get the radius of m's major axis.
float get_mol_radius(mol::Mol& m) {
    float result = 0.0;
    if (m.atoms().size() > 0) {
        float xmin, ymin, zmin, xmax, ymax, zmax;
        mol::AtomVector::const_iterator i;
        mol::AtomVector::const_iterator iEnd(m.atoms().end());
        i = m.atoms().begin();
        {
            mol::Atom *a(*i);
            float x = a->x(), 
                  y = a->y(), 
                  z = a->z(), 
                  r = a->radius();
            xmin = x - r;
            xmax = x + r;
            ymin = y - r;
            ymax = y + r;
            zmin = z - r;
            zmax = z + r;
            ++i;
        }
        for (; i != iEnd; ++i) {
            mol::Atom *a(*i);
            float x = a->x(), 
                  y = a->y(), 
                  z = a->z(), 
                  r = a->radius();
            xmin = min(xmin, x - r);
            xmax = max(xmax, x + r);
            ymin = min(ymin, y - r);
            ymax = max(ymax, y + r);
            zmin = min(zmin, z - r);
            zmax = max(zmax, z + r);
        }
        // Find max diameter:
        float d = xmax - xmin;
        d = max(d, ymax - ymin);
        d = max(d, zmax - zmin);
        result = d / 2.0;
    }
    return result;
}

int main(int argc, char **argv){
    mesaac::MesaacFeatures f;
    f.set(mesaac::MFE_FingerprinterModule);
    mesaac::initCommon(f);
  
    ifstream sdf_inf;
    string sdf_pathname;
    float atom_scale;
    float max_radius;
    
    ArgParser parser(argc, argv);
    parser.show_blurb();
    parser.parse_args(sdf_inf, sdf_pathname, atom_scale, max_radius);

    // Align using atom centers only -- an empty point set will suffice.
    shape::PointList empty_sphere_points;
    shape::AxisAligner aligner(empty_sphere_points, atom_scale, true);
    
    
    mol::Mol m;
    mol::SDReader reader(sdf_inf, sdf_pathname);
    mol::SDWriter writer(cout);
    while (reader.read(m)) {
        // Side effect of shape_filter_by_radius:  output conformers
        // are mean-centered and axis aligned
        aligner.align_to_axes(m);
        float major_radius = get_mol_radius(m);
        if (major_radius <= max_radius) {
            // TODO: Tag m with its major axis radius.
            writer.write(m);
        }
    }
    return 0;
}
