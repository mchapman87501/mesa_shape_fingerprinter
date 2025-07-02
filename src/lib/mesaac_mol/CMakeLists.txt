set(TARGET mesaac_mol)

set(SRC 
    atom.cpp
    io/sdwriter.cpp
    io/sdreader.cpp
    mol.cpp
    element_info.cpp
    sd_tag_map.cpp
)

set(HEADER_DIR ${CMAKE_SOURCE_DIR}/include/${TARGET})
set(HEADERS
    ${HEADER_DIR}/io/sdreader.hpp
    ${HEADER_DIR}/io/sdwriter.hpp
    ${HEADER_DIR}/atom.hpp
    ${HEADER_DIR}/bond.hpp
    ${HEADER_DIR}/element_info.hpp
    ${HEADER_DIR}/io.hpp
    ${HEADER_DIR}/mol.hpp
    ${HEADER_DIR}/position.hpp
    ${HEADER_DIR}/sd_tag_map.hpp
)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET HEADERS 
        BASE_DIRS ${HEADER_DIR}
        FILES ${HEADERS}
)

install(
    TARGETS ${TARGET}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${TARGET}
)
