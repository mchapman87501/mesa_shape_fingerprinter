//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "sdf_mol_aligner.hpp"

#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <vector>

#include "mesaac_mol/io/sdreader.hpp"
#include "mesaac_mol/io/sdwriter.hpp"
#include "mesaac_mol/mol.hpp"

#include "mol_aligner.hpp"

using namespace std;

namespace mesaac::align_monte {
namespace {
void open_input(ifstream &inf, string &pathname, const string &description) {
  // ifstream.open will happily open a directory, on linux...
  filesystem::path path(pathname);
  auto status = filesystem::status(path);
  if (!filesystem::is_regular_file(status)) {
    cerr << "Cannot open " << description << " " << path
         << " for reading: it's not a regular file." << endl;
    exit(1);
  }
  inf.open(path);
  if (!inf) {
    cerr << "Cannot open " << description << " '" << path << "' for reading."
         << endl;
    exit(1);
  }
}

struct SortRecord {
  size_t record_num;
  float value;

  static bool greater(const SortRecord &r1, const SortRecord &r2) {
    if (r1.value > r2.value) {
      return true;
    }
    if (r1.value < r2.value) {
      return false;
    }
    return r1.record_num > r2.record_num;
  }
};

using SortRecordList = vector<SortRecord>;

float get_tag_value(const mol::Mol &mol, string tag_name) {
  const mol::SDTagMap &tags(mol.tags());
  mol::SDTagMap::const_iterator i = tags.find(tag_name);
  float result = nan("");
  if (i != tags.end()) {
    istringstream ins(i->second);
    if (!(ins >> result)) {
      result = nan("");
    }
  }
  return result;
}

void process_mols_conc(mol::SDReader &reader, mol::SDWriter &writer,
                       MolAligner &ma, bool write_sorted,
                       const string &measure_tag,
                       SortRecordList &sort_records) {

  using MolPtr = shared_ptr<mol::Mol>;
  using AlignFuture = future<MolPtr>;
  deque<AlignFuture> tasks;

  // Launch an aligner for every mol read.
  while (!reader.eof()) {
    const auto read_result = reader.read();
    if (!read_result.is_ok()) {
      break;
    }

    const auto mol = make_shared<mol::Mol>(read_result.value());
    auto align_task = async(launch::async, [&ma, mol]() {
      ma.process_one_molecule(*mol);
      return mol;
    });
    tasks.push_back(std::move(align_task));
  }

  // Immediately start writing mols as they are processed,
  // in the same order as they are read.

  // i is the sort record index - indicating the order in which
  // each mol was read.  The ref molecule has index 0.
  const size_t i_max = tasks.size();
  sort_records.reserve(i_max + 1);

  for (size_t i = 1; i <= i_max; ++i) {
    auto mol = tasks.front().get();
    tasks.pop_front();

    writer.write(*mol);
    if (write_sorted) {
      SortRecord record{i, get_tag_value(*mol, measure_tag)};
      sort_records.push_back(record);
    }
  }
}

void write_records(const filesystem::path &out_pathname,
                   SortRecordList &records, const string &measure_name) {
  ofstream outf(out_pathname);
  outf << "Index\t" << measure_name << endl;
  for (const auto &record : records) {
    outf << record.record_num << "\t" << record.value << "\n";
  }
  outf.flush();
  outf.close();
}

} // namespace

void SDFMolAligner::run() {
  read_sphere_points();
  process_molecules();
}

void SDFMolAligner::read_sphere_points() {
  ifstream inf;
  open_input(inf, m_hamms_sphere_pathname, "Hamms Sphere Points file");

  float x, y, z;
  while (inf >> x >> y >> z) {
    m_hamms_sphere_coords.emplace_back(shape::Point{x, y, z});
  }
  inf.close();
}

void SDFMolAligner::process_molecules() {
  ifstream inf;
  open_input(inf, m_sd_pathname, "sd_filename");

  // Read from sdPathname, write to stdout.
  mol::SDReader reader(inf);
  mol::SDWriter writer(cout);
  MolAligner ma(m_hamms_sphere_coords, m_epsilon_sqr, m_ref_fingerprint,
                m_atom_centers_only, m_measures);

  bool write_sorted = (!m_sorted_pathname.empty());
  SortRecordList sort_records;
  string last_measure = (m_measures.at(m_measures.size() - 1)->name());
  string measure_tag = ">  <MaxAlign" + last_measure + ">";

  mol::Mol refmol;
  const auto read_result = reader.read();
  if (read_result.is_ok()) {
    refmol = read_result.value();
    ma.process_ref_molecule(refmol, m_ref_fingerprint);
    writer.write(refmol);
    if (write_sorted) {
      SortRecord r{0, get_tag_value(refmol, measure_tag)};
      sort_records.push_back(r);
    }

    process_mols_conc(reader, writer, ma, write_sorted, measure_tag,
                      sort_records);

    if (write_sorted) {
      sort(sort_records.begin(), sort_records.end(), SortRecord::greater);
      write_records(m_sorted_pathname, sort_records, last_measure);
    }
  }
}
} // namespace mesaac::align_monte
