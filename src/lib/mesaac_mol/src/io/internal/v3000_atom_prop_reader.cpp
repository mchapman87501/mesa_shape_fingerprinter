
#include "v3000_atom_prop_reader.hpp"

#include <map>
#include <optional>
#include <variant>

#include <iostream>

namespace mesaac::mol::internal {

namespace {
using StrNameVal = std::pair<std::string, std::string>;
std::optional<StrNameVal> read_next_nv_pair(std::istream &ins) {
  // Skip whitespace.
  if (ins >> std::ws) {
    // Read until the next "=", to get the property name.
    std::string prop_name;
    if (std::getline(ins, prop_name, '=')) {
      std::string sval;
      if (ins >> sval) {
        // If the value starts with a quote or a paren, read till the closing
        // quote / paren.
        if (sval.starts_with("(") || sval.starts_with("'")) {
          std::string tail;
          const char delim = sval.starts_with("(") ? ')' : '\'';
          if (std::getline(ins, tail, delim)) {
            sval += tail + delim;
          }
        }
        return std::make_pair(prop_name, sval);
      }
    }
  }
  return std::nullopt;
}

bool set_atom_prop(const std::string &name, const std::string &sval,
                   AtomProps &props_result) {
  bool result = true;
  if (name == "CHG") {
    props_result.chg = std::stoi(sval);
  } else if (name == "RAD") {
    props_result.rad = std::stoi(sval);
  } else if (name == "CFG") {
    props_result.cfg = std::stoi(sval);
  } else if (name == "MASS") {
    props_result.mass = std::stof(sval);
  } else if (name == "VAL") {
    props_result.val = std::stoi(sval);
  } else if (name == "HCOUNT") {
    props_result.hcount = std::stoi(sval);
  } else if (name == "STBOX") {
    props_result.stbox = std::stoi(sval);
  } else if (name == "INVRET") {
    props_result.invret = std::stoi(sval);
  } else if (name == "EXACHG") {
    props_result.exachg = std::stoi(sval);
  } else if (name == "SUBST") {
    props_result.subst = std::stoi(sval);
  } else if (name == "UNSAT") {
    props_result.unsat = std::stoi(sval);
  } else if (name == "RBCNT") {
    props_result.rbcnt = std::stoi(sval);
  } else if (name == "ATTCHPT") {
    props_result.attchpt = std::stoi(sval);
  } else if (name == "RGROUPS") {
    // Just pull in the value verbatim.
    props_result.rgroups = sval;
  } else if (name == "ATTCHORD") {
    // Same issues as for RGROUPS...
    props_result.attchord = sval;
  } else if (name == "CLASS") {
    // Presume no quotes
    props_result.cta_class = sval;
  } else if (name == "SEQID") {
    props_result.seqid = std::stoi(sval);
  } else if (name == "SEQNAME") {
    // Not sure of syntax here.  Are single quotes optional?
    // Just use the whole string.
    props_result.seqname = sval;
  } else {
    // This is a warning, not an error.
    std::cerr << "WARNING: atom property '" << name
              << "' is not recognized and will be ignored." << std::endl;
  }
  // Hm.  Turns out we always return true...
  return result;
}
} // namespace

V3000AtomPropReader::Result V3000AtomPropReader::read(std::istream &ins) {
  AtomProps props;

  for (;;) {
    const auto name_value = read_next_nv_pair(ins);
    if (!name_value.has_value()) {
      break;
    }

    const auto nv = name_value.value();
    // It would be nice if props could be just a dict.  But the goal is
    // to be able to read an SD file in either V2000 or V3000 format, and
    // to output it in either format, conserving atom properties.
    if (!set_atom_prop(nv.first, nv.second, props)) {
      return Result::Err("Could not save atom property '" + nv.first + "'");
    }
  }
  return Result::Ok(props);
}
} // namespace mesaac::mol::internal
