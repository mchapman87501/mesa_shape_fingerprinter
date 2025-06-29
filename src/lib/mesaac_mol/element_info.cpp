//
// Provides atomic radius information, scraped from:
// http://manyeyes.alphaworks.ibm.com/manyeyes/datasets/atomic-radius-vs-atomic-number-radiu/versions/1.txt

// As of 2010/03/09, the radius information consists of van Der Waals radii from
// http://www.ccdc.cam.ac.uk/products/csd/radii/table.php4
//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/element_info.hpp"
#include <iostream>
#include <locale>
#include <map>
#include <sstream>

using namespace std;

namespace mesaac::mol {
namespace {
static map<int, double> atomic_radius;
static map<string, double> radius_by_symbol;
static map<string, unsigned char> num_by_symbol;
// TODO:  Re-use num_by_symbol
static map<int, string> symbol_by_num;

bool init_radii() {
  if (atomic_radius.size() <= 0) {
    atomic_radius[1] = 1.09;
    atomic_radius[2] = 1.4;
    atomic_radius[3] = 1.82;
    atomic_radius[4] = 2.0;
    atomic_radius[5] = 2.0;
    atomic_radius[6] = 1.7;
    atomic_radius[7] = 1.55;
    atomic_radius[8] = 1.52;
    atomic_radius[9] = 1.47;
    atomic_radius[10] = 1.54;
    atomic_radius[11] = 2.27;
    atomic_radius[12] = 1.73;
    atomic_radius[13] = 2.0;
    atomic_radius[14] = 2.1;
    atomic_radius[15] = 1.8;
    atomic_radius[16] = 1.8;
    atomic_radius[17] = 1.75;
    atomic_radius[18] = 1.88;
    atomic_radius[19] = 2.75;
    atomic_radius[20] = 2.0;
    atomic_radius[21] = 2.0;
    atomic_radius[22] = 2.0;
    atomic_radius[23] = 2.0;
    atomic_radius[24] = 2.0;
    atomic_radius[25] = 2.0;
    atomic_radius[26] = 2.0;
    atomic_radius[27] = 2.0;
    atomic_radius[28] = 1.63;
    atomic_radius[29] = 1.4;
    atomic_radius[30] = 1.39;
    atomic_radius[31] = 1.87;
    atomic_radius[32] = 2.0;
    atomic_radius[33] = 1.85;
    atomic_radius[34] = 1.9;
    atomic_radius[35] = 1.85;
    atomic_radius[36] = 2.02;
    atomic_radius[37] = 2.0;
    atomic_radius[38] = 2.0;
    atomic_radius[39] = 2.0;
    atomic_radius[40] = 2.0;
    atomic_radius[41] = 2.0;
    atomic_radius[42] = 2.0;
    atomic_radius[43] = 2.0;
    atomic_radius[44] = 2.0;
    atomic_radius[45] = 2.0;
    atomic_radius[46] = 1.63;
    atomic_radius[47] = 1.72;
    atomic_radius[48] = 1.58;
    atomic_radius[49] = 1.93;
    atomic_radius[50] = 2.17;
    atomic_radius[51] = 2.0;
    atomic_radius[52] = 2.06;
    atomic_radius[53] = 1.98;
    atomic_radius[54] = 2.16;
    atomic_radius[55] = 2.0;
    atomic_radius[56] = 2.0;
    atomic_radius[57] = 2.0;
    atomic_radius[58] = 2.0;
    atomic_radius[59] = 2.0;
    atomic_radius[60] = 2.0;
    atomic_radius[61] = 2.0;
    atomic_radius[62] = 2.0;
    atomic_radius[63] = 2.0;
    atomic_radius[64] = 2.0;
    atomic_radius[65] = 2.0;
    atomic_radius[66] = 2.0;
    atomic_radius[67] = 2.0;
    atomic_radius[68] = 2.0;
    atomic_radius[69] = 2.0;
    atomic_radius[70] = 2.0;
    atomic_radius[71] = 2.0;
    atomic_radius[72] = 2.0;
    atomic_radius[73] = 2.0;
    atomic_radius[74] = 2.0;
    atomic_radius[75] = 2.0;
    atomic_radius[76] = 2.0;
    atomic_radius[77] = 2.0;
    atomic_radius[78] = 1.72;
    atomic_radius[79] = 1.66;
    atomic_radius[80] = 1.55;
    atomic_radius[81] = 1.96;
    atomic_radius[82] = 2.02;
    atomic_radius[83] = 2.0;
    atomic_radius[84] = 2.0;
    atomic_radius[85] = 2.0;
    atomic_radius[86] = 2.0;
    atomic_radius[87] = 2.0;
    atomic_radius[88] = 2.0;
    atomic_radius[89] = 2.0;
    atomic_radius[90] = 2.0;
    atomic_radius[91] = 2.0;
    atomic_radius[92] = 1.86;
    atomic_radius[93] = 2.0;
    atomic_radius[94] = 2.0;
    atomic_radius[95] = 2.0;
    atomic_radius[96] = 2.0;
    atomic_radius[97] = 2.0;
    atomic_radius[98] = 2.0;
    atomic_radius[99] = 2.0;
    atomic_radius[100] = 2.0;
    atomic_radius[101] = 2.0;
    atomic_radius[102] = 2.0;
    atomic_radius[103] = 2.0;
    atomic_radius[104] = 2.0;
    atomic_radius[105] = 2.0;
    atomic_radius[106] = 2.0;
    atomic_radius[107] = 2.0;
    atomic_radius[108] = 2.0;
    atomic_radius[109] = 2.0;
    atomic_radius[110] = 2.0;

    radius_by_symbol["Ac"] = 2.0;
    radius_by_symbol["Ag"] = 1.72;
    radius_by_symbol["Al"] = 2.0;
    radius_by_symbol["Am"] = 2.0;
    radius_by_symbol["Ar"] = 1.88;
    radius_by_symbol["As"] = 1.85;
    radius_by_symbol["At"] = 2.0;
    radius_by_symbol["Au"] = 1.66;
    radius_by_symbol["B"] = 2.0;
    radius_by_symbol["Ba"] = 2.0;
    radius_by_symbol["Be"] = 2.0;
    radius_by_symbol["Bh"] = 2.0;
    radius_by_symbol["Bi"] = 2.0;
    radius_by_symbol["Bk"] = 2.0;
    radius_by_symbol["Br"] = 1.85;
    radius_by_symbol["C"] = 1.7;
    radius_by_symbol["Ca"] = 2.0;
    radius_by_symbol["Cd"] = 1.58;
    radius_by_symbol["Ce"] = 2.0;
    radius_by_symbol["Cf"] = 2.0;
    radius_by_symbol["Cl"] = 1.75;
    radius_by_symbol["Cm"] = 2.0;
    radius_by_symbol["Co"] = 2.0;
    radius_by_symbol["Cr"] = 2.0;
    radius_by_symbol["Cs"] = 2.0;
    radius_by_symbol["Cu"] = 1.4;
    radius_by_symbol["Db"] = 2.0;
    radius_by_symbol["Ds"] = 2.0;
    radius_by_symbol["Dy"] = 2.0;
    radius_by_symbol["Er"] = 2.0;
    radius_by_symbol["Es"] = 2.0;
    radius_by_symbol["Eu"] = 2.0;
    radius_by_symbol["F"] = 1.47;
    radius_by_symbol["Fe"] = 2.0;
    radius_by_symbol["Fm"] = 2.0;
    radius_by_symbol["Fr"] = 2.0;
    radius_by_symbol["Ga"] = 1.87;
    radius_by_symbol["Gd"] = 2.0;
    radius_by_symbol["Ge"] = 2.0;
    radius_by_symbol["H"] = 1.09;
    radius_by_symbol["He"] = 1.4;
    radius_by_symbol["Hf"] = 2.0;
    radius_by_symbol["Hg"] = 1.55;
    radius_by_symbol["Ho"] = 2.0;
    radius_by_symbol["Hs"] = 2.0;
    radius_by_symbol["I"] = 1.98;
    radius_by_symbol["In"] = 1.93;
    radius_by_symbol["Ir"] = 2.0;
    radius_by_symbol["K"] = 2.75;
    radius_by_symbol["Kr"] = 2.02;
    radius_by_symbol["La"] = 2.0;
    radius_by_symbol["Li"] = 1.82;
    radius_by_symbol["Lr"] = 2.0;
    radius_by_symbol["Lu"] = 2.0;
    radius_by_symbol["Lw"] = 2.0;
    radius_by_symbol["Md"] = 2.0;
    radius_by_symbol["Mg"] = 1.73;
    radius_by_symbol["Mn"] = 2.0;
    radius_by_symbol["Mo"] = 2.0;
    radius_by_symbol["Mt"] = 2.0;
    radius_by_symbol["N"] = 1.55;
    radius_by_symbol["Na"] = 2.27;
    radius_by_symbol["Nb"] = 2.0;
    radius_by_symbol["Nd"] = 2.0;
    radius_by_symbol["Ne"] = 1.54;
    radius_by_symbol["Ni"] = 1.63;
    radius_by_symbol["No"] = 2.0;
    radius_by_symbol["Np"] = 2.0;
    radius_by_symbol["O"] = 1.52;
    radius_by_symbol["Os"] = 2.0;
    radius_by_symbol["P"] = 1.8;
    radius_by_symbol["Pa"] = 2.0;
    radius_by_symbol["Pb"] = 2.02;
    radius_by_symbol["Pd"] = 1.63;
    radius_by_symbol["Pm"] = 2.0;
    radius_by_symbol["Po"] = 2.0;
    radius_by_symbol["Pr"] = 2.0;
    radius_by_symbol["Pt"] = 1.72;
    radius_by_symbol["Pu"] = 2.0;
    radius_by_symbol["Ra"] = 2.0;
    radius_by_symbol["Rb"] = 2.0;
    radius_by_symbol["Re"] = 2.0;
    radius_by_symbol["Rf"] = 2.0;
    radius_by_symbol["Rh"] = 2.0;
    radius_by_symbol["Rn"] = 2.0;
    radius_by_symbol["Ru"] = 2.0;
    radius_by_symbol["S"] = 1.8;
    radius_by_symbol["Sb"] = 2.0;
    radius_by_symbol["Sc"] = 2.0;
    radius_by_symbol["Se"] = 1.9;
    radius_by_symbol["Sg"] = 2.0;
    radius_by_symbol["Si"] = 2.1;
    radius_by_symbol["Sm"] = 2.0;
    radius_by_symbol["Sn"] = 2.17;
    radius_by_symbol["Sr"] = 2.0;
    radius_by_symbol["Ta"] = 2.0;
    radius_by_symbol["Tb"] = 2.0;
    radius_by_symbol["Tc"] = 2.0;
    radius_by_symbol["Te"] = 2.06;
    radius_by_symbol["Th"] = 2.0;
    radius_by_symbol["Ti"] = 2.0;
    radius_by_symbol["Tl"] = 1.96;
    radius_by_symbol["Tm"] = 2.0;
    radius_by_symbol["U"] = 1.86;
    radius_by_symbol["V"] = 2.0;
    radius_by_symbol["W"] = 2.0;
    radius_by_symbol["Xe"] = 2.16;
    radius_by_symbol["Y"] = 2.0;
    radius_by_symbol["Yb"] = 2.0;
    radius_by_symbol["Zn"] = 1.39;
    radius_by_symbol["Zr"] = 2.0;

    num_by_symbol["Ac"] = 89;
    num_by_symbol["Ag"] = 47;
    num_by_symbol["Al"] = 13;
    num_by_symbol["Am"] = 95;
    num_by_symbol["Ar"] = 18;
    num_by_symbol["As"] = 33;
    num_by_symbol["At"] = 85;
    num_by_symbol["Au"] = 79;
    num_by_symbol["B"] = 5;
    num_by_symbol["Ba"] = 56;
    num_by_symbol["Be"] = 4;
    num_by_symbol["Bh"] = 107;
    num_by_symbol["Bi"] = 83;
    num_by_symbol["Bk"] = 97;
    num_by_symbol["Br"] = 35;
    num_by_symbol["C"] = 6;
    num_by_symbol["Ca"] = 20;
    num_by_symbol["Cd"] = 48;
    num_by_symbol["Ce"] = 58;
    num_by_symbol["Cf"] = 98;
    num_by_symbol["Cl"] = 17;
    num_by_symbol["Cm"] = 96;
    num_by_symbol["Co"] = 27;
    num_by_symbol["Cr"] = 24;
    num_by_symbol["Cs"] = 55;
    num_by_symbol["Cu"] = 29;
    num_by_symbol["Db"] = 105;
    num_by_symbol["Ds"] = 110;
    num_by_symbol["Dy"] = 66;
    num_by_symbol["Er"] = 68;
    num_by_symbol["Es"] = 99;
    num_by_symbol["Eu"] = 63;
    num_by_symbol["F"] = 9;
    num_by_symbol["Fe"] = 26;
    num_by_symbol["Fm"] = 100;
    num_by_symbol["Fr"] = 87;
    num_by_symbol["Ga"] = 31;
    num_by_symbol["Gd"] = 64;
    num_by_symbol["Ge"] = 32;
    num_by_symbol["H"] = 1;
    num_by_symbol["He"] = 2;
    num_by_symbol["Hf"] = 72;
    num_by_symbol["Hg"] = 80;
    num_by_symbol["Ho"] = 67;
    num_by_symbol["Hs"] = 108;
    num_by_symbol["I"] = 53;
    num_by_symbol["In"] = 49;
    num_by_symbol["Ir"] = 77;
    num_by_symbol["K"] = 19;
    num_by_symbol["Kr"] = 36;
    num_by_symbol["La"] = 57;
    num_by_symbol["Li"] = 3;
    num_by_symbol["Lr (Lw)"] = 103;
    num_by_symbol["Lu"] = 71;
    num_by_symbol["Md"] = 101;
    num_by_symbol["Mg"] = 12;
    num_by_symbol["Mn"] = 25;
    num_by_symbol["Mo"] = 42;
    num_by_symbol["Mt"] = 109;
    num_by_symbol["N"] = 7;
    num_by_symbol["Na"] = 11;
    num_by_symbol["Nb"] = 41;
    num_by_symbol["Nd"] = 60;
    num_by_symbol["Ne"] = 10;
    num_by_symbol["Ni"] = 28;
    num_by_symbol["No"] = 102;
    num_by_symbol["Np"] = 93;
    num_by_symbol["O"] = 8;
    num_by_symbol["Os"] = 76;
    num_by_symbol["P"] = 15;
    num_by_symbol["Pa"] = 91;
    num_by_symbol["Pb"] = 82;
    num_by_symbol["Pd"] = 46;
    num_by_symbol["Pm"] = 61;
    num_by_symbol["Po"] = 84;
    num_by_symbol["Pr"] = 59;
    num_by_symbol["Pt"] = 78;
    num_by_symbol["Pu"] = 94;
    num_by_symbol["Ra"] = 88;
    num_by_symbol["Rb"] = 37;
    num_by_symbol["Re"] = 75;
    num_by_symbol["Rf"] = 104;
    num_by_symbol["Rh"] = 45;
    num_by_symbol["Rn"] = 86;
    num_by_symbol["Ru"] = 44;
    num_by_symbol["S"] = 16;
    num_by_symbol["Sb"] = 51;
    num_by_symbol["Sc"] = 21;
    num_by_symbol["Se"] = 34;
    num_by_symbol["Sg"] = 106;
    num_by_symbol["Si"] = 14;
    num_by_symbol["Sm"] = 62;
    num_by_symbol["Sn"] = 50;
    num_by_symbol["Sr"] = 38;
    num_by_symbol["Ta"] = 73;
    num_by_symbol["Tb"] = 65;
    num_by_symbol["Tc"] = 43;
    num_by_symbol["Te"] = 52;
    num_by_symbol["Th"] = 90;
    num_by_symbol["Ti"] = 22;
    num_by_symbol["Tl"] = 81;
    num_by_symbol["Tm"] = 69;
    num_by_symbol["U"] = 92;
    num_by_symbol["V"] = 23;
    num_by_symbol["W"] = 74;
    num_by_symbol["Xe"] = 54;
    num_by_symbol["Y"] = 39;
    num_by_symbol["Yb"] = 70;
    num_by_symbol["Zn"] = 30;
    num_by_symbol["Zr"] = 40;

    // TODO:  Figure out how to reuse num_by_symbol
    symbol_by_num[1] = "H";
    symbol_by_num[2] = "He";
    symbol_by_num[3] = "Li";
    symbol_by_num[4] = "Be";
    symbol_by_num[5] = "B";
    symbol_by_num[6] = "C";
    symbol_by_num[7] = "N";
    symbol_by_num[8] = "O";
    symbol_by_num[9] = "F";
    symbol_by_num[10] = "Ne";
    symbol_by_num[11] = "Na";
    symbol_by_num[12] = "Mg";
    symbol_by_num[13] = "Al";
    symbol_by_num[14] = "Si";
    symbol_by_num[15] = "P";
    symbol_by_num[16] = "S";
    symbol_by_num[17] = "Cl";
    symbol_by_num[18] = "Ar";
    symbol_by_num[19] = "K";
    symbol_by_num[20] = "Ca";
    symbol_by_num[21] = "Sc";
    symbol_by_num[22] = "Ti";
    symbol_by_num[23] = "V";
    symbol_by_num[24] = "Cr";
    symbol_by_num[25] = "Mn";
    symbol_by_num[26] = "Fe";
    symbol_by_num[27] = "Co";
    symbol_by_num[28] = "Ni";
    symbol_by_num[29] = "Cu";
    symbol_by_num[30] = "Zn";
    symbol_by_num[31] = "Ga";
    symbol_by_num[32] = "Ge";
    symbol_by_num[33] = "As";
    symbol_by_num[34] = "Se";
    symbol_by_num[35] = "Br";
    symbol_by_num[36] = "Kr";
    symbol_by_num[37] = "Rb";
    symbol_by_num[38] = "Sr";
    symbol_by_num[39] = "Y";
    symbol_by_num[40] = "Zr";
    symbol_by_num[41] = "Nb";
    symbol_by_num[42] = "Mo";
    symbol_by_num[43] = "Tc";
    symbol_by_num[44] = "Ru";
    symbol_by_num[45] = "Rh";
    symbol_by_num[46] = "Pd";
    symbol_by_num[47] = "Ag";
    symbol_by_num[48] = "Cd";
    symbol_by_num[49] = "In";
    symbol_by_num[50] = "Sn";
    symbol_by_num[51] = "Sb";
    symbol_by_num[52] = "Te";
    symbol_by_num[53] = "I";
    symbol_by_num[54] = "Xe";
    symbol_by_num[55] = "Cs";
    symbol_by_num[56] = "Ba";
    symbol_by_num[57] = "La";
    symbol_by_num[58] = "Ce";
    symbol_by_num[59] = "Pr";
    symbol_by_num[60] = "Nd";
    symbol_by_num[61] = "Pm";
    symbol_by_num[62] = "Sm";
    symbol_by_num[63] = "Eu";
    symbol_by_num[64] = "Gd";
    symbol_by_num[65] = "Tb";
    symbol_by_num[66] = "Dy";
    symbol_by_num[67] = "Ho";
    symbol_by_num[68] = "Er";
    symbol_by_num[69] = "Tm";
    symbol_by_num[70] = "Yb";
    symbol_by_num[71] = "Lu";
    symbol_by_num[72] = "Hf";
    symbol_by_num[73] = "Ta";
    symbol_by_num[74] = "W";
    symbol_by_num[75] = "Re";
    symbol_by_num[76] = "Os";
    symbol_by_num[77] = "Ir";
    symbol_by_num[78] = "Pt";
    symbol_by_num[79] = "Au";
    symbol_by_num[80] = "Hg";
    symbol_by_num[81] = "Tl";
    symbol_by_num[82] = "Pb";
    symbol_by_num[83] = "Bi";
    symbol_by_num[84] = "Po";
    symbol_by_num[85] = "At";
    symbol_by_num[86] = "Rn";
    symbol_by_num[87] = "Fr";
    symbol_by_num[88] = "Ra";
    symbol_by_num[89] = "Ac";
    symbol_by_num[90] = "Th";
    symbol_by_num[91] = "Pa";
    symbol_by_num[92] = "U";
    symbol_by_num[93] = "Np";
    symbol_by_num[94] = "Pu";
    symbol_by_num[95] = "Am";
    symbol_by_num[96] = "Cm";
    symbol_by_num[97] = "Bk";
    symbol_by_num[98] = "Cf";
    symbol_by_num[99] = "Es";
    symbol_by_num[100] = "Fm";
    symbol_by_num[101] = "Md";
    symbol_by_num[102] = "No";
    symbol_by_num[103] = "Lr (Lw)";
    symbol_by_num[104] = "Rf";
    symbol_by_num[105] = "Db";
    symbol_by_num[106] = "Sg";
    symbol_by_num[107] = "Bh";
    symbol_by_num[108] = "Hs";
    symbol_by_num[109] = "Mt";
    symbol_by_num[110] = "Ds";
  }
  return true;
}

string strip(const string &src) {
  string::const_iterator i_start, i_end;
  i_start = src.begin();
  while ((i_start != src.end()) && isspace(*i_start)) {
    i_start++;
  }
  i_end = i_start;
  while ((i_end != src.end()) && !isspace(*i_end)) {
    i_end++;
  }
  string result =
      src.substr(distance(src.begin(), i_start), distance(i_start, i_end));
  return result;
}

bool inited = init_radii();

} // namespace

double get_radius(int atomic_number) {
  double result = atomic_radius[atomic_number];
  const auto entry = atomic_radius.find(atomic_number);
  if (entry == atomic_radius.end()) {
    ostringstream msg;
    msg << "Unknown atomic number " << atomic_number;
    throw std::invalid_argument(msg.str());
  }
  return entry->second;
}

double get_symbol_radius(const string &atomic_symbol) {
  string sym(strip(atomic_symbol));
  const auto entry = radius_by_symbol.find(sym);
  if (entry == radius_by_symbol.end()) {
    ostringstream msg;
    msg << "Unknown atomic symbol '" << sym << "'";
    throw std::invalid_argument(msg.str());
  }
  return entry->second;
}

unsigned char get_atomic_num(const std::string &atomic_symbol) {
  string sym(strip(atomic_symbol));
  const auto entry = num_by_symbol.find(sym);
  if (entry == num_by_symbol.end()) {
    ostringstream msg;
    msg << "Unknown atomic symbol '" << sym << "'";
    throw std::invalid_argument(msg.str());
  }
  return entry->second;
}

string get_symbol(int atomic_number) {
  map<int, string>::iterator entry = symbol_by_num.find(atomic_number);
  if (entry == symbol_by_num.end()) {
    ostringstream msg;
    msg << "Unknown atomic number " << atomic_number;
    throw std::invalid_argument(msg.str());
  }
  return entry->second;
  ;
}
} // namespace mesaac::mol
