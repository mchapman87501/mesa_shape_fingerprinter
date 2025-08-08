set(TARGET mesaac_mol)

set(SRC
    src/atom.cpp
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
    src/io/internal/v3000_ctab_reader.cpp)

set(HEADER_DIR include)
set(MOL_HEADER_DIR ${HEADER_DIR}/mesaac_mol)

set(HEADERS
    ${MOL_HEADER_DIR}/mesaac_mol.hpp
    ${MOL_HEADER_DIR}/io/path_sdreader.hpp
    ${MOL_HEADER_DIR}/io/sdreader.hpp
    ${MOL_HEADER_DIR}/io/sdwriter.hpp
    ${MOL_HEADER_DIR}/atom_props.hpp
    ${MOL_HEADER_DIR}/atom.hpp
    ${MOL_HEADER_DIR}/bond_props.hpp
    ${MOL_HEADER_DIR}/bond.hpp
    ${MOL_HEADER_DIR}/element_info.hpp
    ${MOL_HEADER_DIR}/io.hpp
    ${MOL_HEADER_DIR}/mol.hpp
    ${MOL_HEADER_DIR}/position.hpp
    ${MOL_HEADER_DIR}/result.hpp
    ${MOL_HEADER_DIR}/sd_tag_map.hpp)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${HEADER_DIR} src/io/internal)

target_sources(${TARGET} PUBLIC FILE_SET HEADERS BASE_DIRS ${HEADER_DIR} FILES
                                ${HEADERS})

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
