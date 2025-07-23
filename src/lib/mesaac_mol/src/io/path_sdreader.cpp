#include "mesaac_mol/io/path_sdreader.hpp"
#include "mesaac_mol/io/sdreader.hpp"
#include <fstream>
#include <sstream>
#include <string>

namespace mesaac::mol {

struct ISDReader {
  virtual SDReader &reader() = 0;
};

struct PathSDReaderImpl : public ISDReader {
  PathSDReaderImpl(const std::string &pathname)
      : m_pathname(pathname), m_inf(pathname), m_reader(m_inf, m_pathname) {}

  SDReader &reader() override { return m_reader; }

  std::string m_pathname;
  std::ifstream m_inf;
  SDReader m_reader;
};

struct StringSDReaderImpl : public ISDReader {
  StringSDReaderImpl(const std::string &sd_content, const std::string &pathname)
      : m_pathname(pathname), m_inf(), m_reader(m_inf, m_pathname) {
    m_inf.str(sd_content);
  }
  SDReader &reader() override { return m_reader; }

  std::string m_pathname;
  std::istringstream m_inf;
  SDReader m_reader;
};

PathSDReader::PathSDReader(const std::string &sd_file_content,
                           const std::string &pathname)
    : m_impl(std::make_shared<StringSDReaderImpl>(sd_file_content, pathname)) {}

PathSDReader::PathSDReader(const std::string &pathname)
    : m_impl(std::make_shared<PathSDReaderImpl>(pathname)) {}

PathSDReader::PathSDReader(const PathSDReader &src) { *this = src; }

PathSDReader::PathSDReader(PathSDReader &&src) { *this = std::move(src); }
PathSDReader &PathSDReader::operator=(const PathSDReader &src) {
  this->m_impl = src.m_impl;
  return *this;
}
PathSDReader &PathSDReader::operator=(PathSDReader &&src) {
  this->m_impl = std::move(src.m_impl);
  return *this;
}

bool PathSDReader::read(Mol &mol) { return m_impl->reader().read(mol); }
bool PathSDReader::skip() { return m_impl->reader().skip(); }

} // namespace mesaac::mol