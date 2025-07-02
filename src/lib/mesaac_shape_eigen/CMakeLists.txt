set(TARGET mesaac_shape_eigen)

set(SRC
    axis_aligner.cpp
    fingerprinter.cpp
    hammersley.cpp
    vol_box.cpp
)
set(HEADER_DIR include)
set(HEADERS
    ${HEADER_DIR}/mesaac_shape_eigen/axis_aligner.hpp
    ${HEADER_DIR}/mesaac_shape_eigen/fingerprinter.hpp
    ${HEADER_DIR}/mesaac_shape_eigen/hammersley.hpp
    ${HEADER_DIR}/mesaac_shape_eigen/shared_types.hpp
    ${HEADER_DIR}/mesaac_shape_eigen/vol_box.hpp
)


# Hardwired for macOS and homebrew installation of OpenBabel:
list(APPEND CMAKE_MODULE_PATH /opt/homebrew/Cellar/open-babel/3.1.1_2/lib/cmake/openbabel3)
find_package(OpenBabel3)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${HEADER_DIR})

# mesaac_common is needed only for shape_defs.hpp.  Should that header live elsewhere?
target_link_libraries(${TARGET} PUBLIC mesaac_common mesaac_mol Boost::dynamic_bitset Eigen3::Eigen ${OpenBabel_LIBRARIES})

target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET HEADERS 
        BASE_DIRS ${HEADER_DIR}
        FILES ${HEADERS}
)

install(
    TARGETS ${TARGET}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
