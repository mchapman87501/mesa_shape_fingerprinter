//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "sdf_mol_aligner.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#if HAVE_OPENMP
#include <omp.h>
#endif

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
  int record_num;
  float value;

  static bool compare(const SortRecord &r1, const SortRecord &r2) {
    return r1.value > r2.value;
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

} // namespace

void SDFMolAligner::run() {
  read_sphere_points();
  process_molecules();
}

void SDFMolAligner::read_sphere_points() {
  ifstream inf;
  open_input(inf, m_hamms_sphere_pathname, "Hamms Sphere Points file");

  float coord;
  while (inf >> coord) {
    shape::Point3D point{coord, 0, 0};
    inf >> point[1] >> point[2];
    m_hamms_sphere_coords.push_back(point);
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
  int i = 0;
  const auto read_result = reader.read();
  if (read_result.is_ok()) {
    refmol = read_result.value();
    ma.process_ref_molecule(refmol, m_ref_fingerprint);
    writer.write(refmol);
    if (write_sorted) {
      SortRecord r = {i, get_tag_value(refmol, measure_tag)};
      sort_records.push_back(r);
    }

    i++;

#if HAVE_OPENMP
    const int num_procs = omp_get_num_procs();
    const int queue_size = 4 * num_procs;

    mol::Mol mol_batch[queue_size];
    SortRecord sort_record_batch[queue_size];
    shared_ptr<ostringstream> outstr[queue_size];
    mol::SDWriter::Ptr buff_writer[queue_size];

    {
      int j;
      for (j = 0; j < queue_size; j++) {
        outstr[j] = make_shared<ostringstream>();
        outstr[j]->imbue(locale("C"));
        buff_writer[j] = make_shared<mol::SDWriter>(*outstr[j]);
      }
    }
#endif

    while (true) {
#if HAVE_OPENMP
      int j;
      for (j = 0; j < queue_size; j++) {
        const auto read_result = reader.read();
        if (!read_result.is_ok()) {
          break;
        }
        mol_batch[j] = read_result.value();
      }
      int num_mols = j;
#pragma omp parallel for
      for (j = 0; j < num_mols; j++) {
        // Is it safe to share access to the ref mol?
        ma.process_one_molecule(mol_batch[j]);
        outstr[j]->str("");
        buff_writer[j]->write(mol_batch[j]);
        if (write_sorted) {
          sort_record_batch[j].record_num = i + j;
          sort_record_batch[j].value = get_tag_value(mol_batch[j], measure_tag);
        }
      }

      // Serialize output and accumulation of sort records.
      for (j = 0; j < num_mols; j++) {
        cout << outstr[j]->str();
      }
      if (write_sorted) {
        for (j = 0; j < num_mols; j++) {
          sort_records.push_back(sort_record_batch[j]);
        }
      }
      i += num_mols;
      if (num_mols < queue_size) {
        break;
      }
#else
      const auto read_result = reader.read();
      if (!read_result.is_ok()) {
        std::cerr << read_result.error() << std::endl;
        break;
      }
      auto mol = read_result.value();
      ma.process_one_molecule(mol);
      writer.write(mol);
      if (write_sorted) {
        SortRecord r = {i, get_tag_value(mol, measure_tag)};
        sort_records.push_back(r);
      }
      i++;
#endif
    }
  }

  if (write_sorted) {
    sort(sort_records.begin(), sort_records.end(), SortRecord::compare);
    ofstream outf(m_sorted_pathname);

    outf << "Index\t" << last_measure << endl;
    SortRecordList::iterator i;
    for (i = sort_records.begin(); i != sort_records.end(); ++i) {
      SortRecord &r(*i);
      outf << r.record_num << "\t" << r.value << endl;
    }
    outf.close();
  }
}
} // namespace mesaac::align_monte
