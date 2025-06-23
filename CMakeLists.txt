cmake_minimum_required(VERSION 4.0)
set(PROJECT shape_fingerprints)

project(${PROJECT} VERSION 1.0.0 LANGUAGES C CXX Fortran)

# Facilitate *not* building tests when using via FetchContent_MakeAvailable.
# See https://github.com/glfw/glfw/blob/master/CMakeLists.txt
# This presumes all tests are defined in the toplevel tests directory --
# a presumption that is not true of this project.
#option(SHAPE_FINGERPRINTS_BUILD_TESTS "Build test targets" ${PROJECT_IS_TOP_LEVEL})


# Get Boost from Github
set(BOOST_INCLUDE_LIBRARIES dynamic_bitset)
set(BOOST_ENABLE_CMAKE ON)
include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE) # Needed to print downloading progress
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.tar.gz # downloading a zip release speeds up the download
    USES_TERMINAL_DOWNLOAD TRUE 
    GIT_PROGRESS TRUE   
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(Boost)

# mesaac_shape_eigen depends on Eigen.
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.4.0
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(eigen)

# shape_feature_fingerprinter depends on OpenBabel.
# Alas, OpenBabel depends on CMake 3.5, which is not compatible with
# CMake 4.0.  It may be necessary to use our own fork of OpenBabel,
# one with a patched top-level CMakeLists.txt; or to require that
# OpenBabel be installed separately before compiling.
# FetchContent_Declare(
#     OpenBabel
#     GIT_REPOSITORY https://github.com/openbabel/openbabel.git
#     GIT_TAG openbabel-3-1-1
#     GIT_SHALLOW TRUE
# )
# FetchContent_MakeAvailable(OpenBabel)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)



# See note, above, about the arrangement of tests within this project.
# if(SHAPE_FINGERPRINTS_BUILD_TESTS)
#     # Source of CodeCoverage.cmake:
#     # https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake
#     include(cmake_modules/CodeCoverage.cmake)
#     find_package(Catch2 3 REQUIRED)
#     enable_testing()

#     include(CTest)
#     include(Catch)

#     add_subdirectory(tests)
# endif()

# include(cmake_modules/CodeCoverage.cmake)
find_package(Catch2 3 REQUIRED)
enable_testing()

include(CTest)
include(Catch)

add_subdirectory(src)
