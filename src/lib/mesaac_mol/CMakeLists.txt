set(TARGET mesaac_mol)

set(SRC
    src/atom.cpp
    src/io/sdwriter.cpp
    src/io/sdreader.cpp
    src/io/path_sdreader.cpp
    src/mol.cpp
    src/element_info.cpp
    src/sd_tag_map.cpp)

set(HEADER_DIR include)
set(HEADERS
    ${HEADER_DIR}/mesaac_mol/mesaac_mol.hpp
    ${HEADER_DIR}/mesaac_mol/io/path_sdreader.hpp
    ${HEADER_DIR}/mesaac_mol/io/sdreader.hpp
    ${HEADER_DIR}/mesaac_mol/io/sdwriter.hpp
    ${HEADER_DIR}/mesaac_mol/atom.hpp
    ${HEADER_DIR}/mesaac_mol/bond.hpp
    ${HEADER_DIR}/mesaac_mol/element_info.hpp
    ${HEADER_DIR}/mesaac_mol/io.hpp
    ${HEADER_DIR}/mesaac_mol/mol.hpp
    ${HEADER_DIR}/mesaac_mol/position.hpp
    ${HEADER_DIR}/mesaac_mol/sd_tag_map.hpp)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${HEADER_DIR})

target_sources(${TARGET} PUBLIC FILE_SET HEADERS BASE_DIRS ${HEADER_DIR} FILES
                                ${HEADERS})

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
