set(SRC
    axis_aligner.cpp
    fingerprinter.cpp
    hammersley.cpp
    vol_box.cpp
)

set(TARGET mesaac_shape_eigen)

# Hardwired for macOS and homebrew installation of OpenBabel:
list(APPEND CMAKE_MODULE_PATH /opt/homebrew/Cellar/open-babel/3.1.1_2/lib/cmake/openbabel3)
find_package(OpenBabel3)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${TARGET} PUBLIC Boost::dynamic_bitset Eigen3::Eigen ${OpenBabel_LIBRARIES})

add_subdirectory(test)