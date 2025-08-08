#include "mesaac_mol/element_info.hpp"
#include <algorithm>
#include <iostream>
#include <locale>
#include <map>
#include <sstream>
#include <vector>

using namespace std;

namespace mesaac::mol {
namespace {
// All of the following values were derived from PubChem's periodic table:
// https://pubchem.ncbi.nlm.nih.gov/periodic-table/
// PubChem's values are in pm - picometers, or 10^{-12}.
// OpenBabel, among other projects, represents atom radii in Angstroms.
// 1 Angstrom = 10^{-10} meter.
// These values have been rescaled to Angstroms.
const std::vector<float> atomic_radius{
    1.20, 1.40, 1.82, 1.53, 1.92, 1.70, 1.55, 1.52, 1.35, 1.54, 2.27,
    1.73, 1.84, 2.10, 1.80, 1.80, 1.75, 1.88, 2.75, 2.31, 2.11, 1.87,
    1.79, 1.89, 1.97, 1.94, 1.92, 1.63, 1.40, 1.39, 1.87, 2.11, 1.85,
    1.90, 1.83, 2.02, 3.03, 2.49, 2.19, 1.86, 2.07, 2.09, 2.09, 2.07,
    1.95, 2.02, 1.72, 1.58, 1.93, 2.17, 2.06, 2.06, 1.98, 2.16, 3.43,
    2.68, 2.40, 2.35, 2.39, 2.29, 2.36, 2.29, 2.33, 2.37, 2.21, 2.29,
    2.16, 2.35, 2.27, 2.42, 2.21, 2.12, 2.17, 2.10, 2.17, 2.16, 2.02,
    2.09, 1.66, 2.09, 1.96, 2.02, 2.07, 1.97, 2.02, 2.20, 3.48, 2.83,
    2.60, 2.37, 2.43, 2.40, 2.21, 2.43, 2.44, 2.45, 2.44, 2.45, 2.45,
};

const std::vector<float> atomic_mass{
    1.008,      4.0026,    7.0,         9.012183,  10.81,      12.011,
    14.007,     15.999,    18.99840316, 20.18,     22.9897693, 24.305,
    26.981538,  28.085,    30.973762,   32.07,     35.45,      39.9,
    39.0983,    40.08,     44.95591,    47.867,    50.9415,    51.996,
    54.93804,   55.84,     58.93319,    58.693,    63.55,      65.4,
    69.723,     72.63,     74.92159,    78.97,     79.9,       83.8,
    85.468,     87.62,     88.90584,    91.22,     92.90637,   95.95,
    96.90636,   101.1,     102.9055,    106.42,    107.868,    112.41,
    114.818,    118.71,    121.76,      127.6,     126.9045,   131.29,
    132.905452, 137.33,    138.9055,    140.116,   140.90766,  144.24,
    144.91276,  150.4,     151.964,     157.25,    158.92535,  162.5,
    164.93033,  167.26,    168.93422,   173.05,    174.9667,   178.49,
    180.9479,   183.84,    186.207,     190.2,     192.22,     195.08,
    196.96657,  200.59,    204.383,     207.0,     208.9804,   208.98243,
    209.98715,  222.01758, 223.01973,   226.02541, 227.02775,  232.038,
    231.03588,  238.0289,  237.048172,  244.0642,  243.06138,  247.07035,
    247.07031,  251.07959, 252.083,     257.09511, 258.09843,  259.101,
    266.12,     267.122,   268.126,     269.128,   270.133,    269.1336,
    277.154,    282.166,   282.169,     286.179,   286.182,    290.192,
    290.196,    293.205,   294.211,     295.216,
};

const std::map<std::string, int> atomic_num_by_atomic_symbol{
    {"Ac", 89},  {"Ag", 47},  {"Al", 13},  {"Am", 95},  {"Ar", 18},
    {"As", 33},  {"At", 85},  {"Au", 79},  {"B", 5},    {"Ba", 56},
    {"Be", 4},   {"Bh", 107}, {"Bi", 83},  {"Bk", 97},  {"Br", 35},
    {"C", 6},    {"Ca", 20},  {"Cd", 48},  {"Ce", 58},  {"Cf", 98},
    {"Cl", 17},  {"Cm", 96},  {"Cn", 112}, {"Co", 27},  {"Cr", 24},
    {"Cs", 55},  {"Cu", 29},  {"Db", 105}, {"Ds", 110}, {"Dy", 66},
    {"Er", 68},  {"Es", 99},  {"Eu", 63},  {"F", 9},    {"Fe", 26},
    {"Fl", 114}, {"Fm", 100}, {"Fr", 87},  {"Ga", 31},  {"Gd", 64},
    {"Ge", 32},  {"H", 1},    {"He", 2},   {"Hf", 72},  {"Hg", 80},
    {"Ho", 67},  {"Hs", 108}, {"I", 53},   {"In", 49},  {"Ir", 77},
    {"K", 19},   {"Kr", 36},  {"La", 57},  {"Li", 3},   {"Lr", 103},
    {"Lu", 71},  {"Lv", 116}, {"Mc", 115}, {"Md", 101}, {"Mg", 12},
    {"Mn", 25},  {"Mo", 42},  {"Mt", 109}, {"N", 7},    {"Na", 11},
    {"Nb", 41},  {"Nd", 60},  {"Ne", 10},  {"Nh", 113}, {"Ni", 28},
    {"No", 102}, {"Np", 93},  {"O", 8},    {"Og", 118}, {"Os", 76},
    {"P", 15},   {"Pa", 91},  {"Pb", 82},  {"Pd", 46},  {"Pm", 61},
    {"Po", 84},  {"Pr", 59},  {"Pt", 78},  {"Pu", 94},  {"Ra", 88},
    {"Rb", 37},  {"Re", 75},  {"Rf", 104}, {"Rg", 111}, {"Rh", 45},
    {"Rn", 86},  {"Ru", 44},  {"S", 16},   {"Sb", 51},  {"Sc", 21},
    {"Se", 34},  {"Sg", 106}, {"Si", 14},  {"Sm", 62},  {"Sn", 50},
    {"Sr", 38},  {"Ta", 73},  {"Tb", 65},  {"Tc", 43},  {"Te", 52},
    {"Th", 90},  {"Ti", 22},  {"Tl", 81},  {"Tm", 69},  {"Ts", 117},
    {"U", 92},   {"V", 23},   {"W", 74},   {"Xe", 54},  {"Y", 39},
    {"Yb", 70},  {"Zn", 30},  {"Zr", 40},
};

const std::vector<std::string> atomic_symbol{
    "H",  "He", "Li", "Be", "B",  "C",  "N",  "O",  "F",  "Ne", "Na", "Mg",
    "Al", "Si", "P",  "S",  "Cl", "Ar", "K",  "Ca", "Sc", "Ti", "V",  "Cr",
    "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr",
    "Rb", "Sr", "Y",  "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd",
    "In", "Sn", "Sb", "Te", "I",  "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd",
    "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu", "Hf",
    "Ta", "W",  "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po",
    "At", "Rn", "Fr", "Ra", "Ac", "Th", "Pa", "U",  "Np", "Pu", "Am", "Cm",
    "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr", "Rf", "Db", "Sg", "Bh", "Hs",
    "Mt", "Ds", "Rg", "Cn", "Nh", "Fl", "Mc", "Lv", "Ts", "Og",
};

string strip(const string &src) {
  const auto not_space = [](unsigned char c) { return !std::isspace(c); };
  auto first = std::find_if(src.begin(), src.end(), not_space);
  auto last = std::find_if(src.rbegin(), src.rend(), not_space).base();
  if (first >= last) {
    return "";
  }
  return std::string(first, last);
}

} // namespace

float get_radius(int atomic_number) {
  return atomic_radius.at(atomic_number - 1);
}

float get_symbol_radius(const std::string &atomic_symbol) {
  return get_radius(get_atomic_num(atomic_symbol));
}

unsigned char get_atomic_num(const std::string &atomic_symbol) {
  string sym(strip(atomic_symbol));
  const auto entry = atomic_num_by_atomic_symbol.find(sym);
  if (entry == atomic_num_by_atomic_symbol.end()) {
    ostringstream msg;
    msg << "Unknown atomic symbol '" << sym << "'";
    throw std::invalid_argument(msg.str());
  }
  return entry->second;
}

string get_symbol(int atomic_number) {
  return atomic_symbol.at(atomic_number - 1);
}

float get_atomic_mass(int atomic_number) {
  return atomic_mass.at(atomic_number - 1);
}
} // namespace mesaac::mol
