set(TARGET mesaac_shape_eigen)

set(SRC
    axis_aligner.cpp
    fingerprinter.cpp
    hammersley.cpp
    vol_box.cpp
)
set(HEADER_DIR ${CMAKE_SOURCE_DIR}/include/${TARGET})
set(HEADERS
    ${HEADER_DIR}/axis_aligner.hpp
    ${HEADER_DIR}/fingerprinter.hpp
    ${HEADER_DIR}/hammersley.hpp
    ${HEADER_DIR}/shared_types.hpp
    ${HEADER_DIR}/vol_box.hpp
)


# Hardwired for macOS and homebrew installation of OpenBabel:
list(APPEND CMAKE_MODULE_PATH /opt/homebrew/Cellar/open-babel/3.1.1_2/lib/cmake/openbabel3)
find_package(OpenBabel3)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${TARGET} PUBLIC Boost::dynamic_bitset Eigen3::Eigen ${OpenBabel_LIBRARIES})

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

add_subdirectory(test)