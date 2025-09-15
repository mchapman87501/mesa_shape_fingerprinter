set(TARGET mesaac_mol)

add_library(${TARGET} STATIC)
target_sources(
  ${TARGET}
  PRIVATE src/atom.cpp
          src/io/sdwriter.cpp
          src/io/sdreader.cpp
          src/io/path_sdreader.cpp
          src/mol.cpp
          src/element_info.cpp
          src/sd_tag_map.cpp
          src/io/internal/line_reader.cpp
          src/io/internal/sdtags_reader.cpp
          src/io/internal/v2000_ctab_reader.cpp
          src/io/internal/v2000_field_read_fns.cpp
          src/io/internal/v2000_prop_block_reader.cpp
          src/io/internal/v3000_atom_prop_reader.cpp
          src/io/internal/v3000_bond_prop_reader.cpp
          src/io/internal/v3000_ctab_reader.cpp
  PRIVATE FILE_SET privateHeaders TYPE HEADERS BASE_DIRS src/io/internal
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         include
         FILES
         include/mesaac_mol/mesaac_mol.hpp
         include/mesaac_mol/atom_props.hpp
         include/mesaac_mol/atom.hpp
         include/mesaac_mol/bond_props.hpp
         include/mesaac_mol/bond.hpp
         include/mesaac_mol/element_info.hpp
         include/mesaac_mol/io.hpp
         include/mesaac_mol/io/path_sdreader.hpp
         include/mesaac_mol/io/sdreader.hpp
         include/mesaac_mol/io/sdwriter.hpp
         include/mesaac_mol/mol.hpp
         include/mesaac_mol/position.hpp
         include/mesaac_mol/result.hpp
         include/mesaac_mol/sd_tag_map.hpp)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
