cmake_minimum_required(VERSION 4.0)
set(PROJECT shape_fingerprints)

# Note: Fortran is needed by the Eigen 3 package.  Add it here before
# uncommenting the Eigen 3 FetchContent calls below.
project(${PROJECT} VERSION 1.0.0 LANGUAGES C CXX)

# Facilitate *not* building tests when using via FetchContent_MakeAvailable.
# See https://github.com/glfw/glfw/blob/master/CMakeLists.txt
option(SHAPE_FINGERPRINTS_BUILD_TESTS "Build test targets" ${PROJECT_IS_TOP_LEVEL})


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

# mesaac_shape_eigen depends on Eigen 2, which was from
# https://eigen.tuxfamily.org/dox/group__QuickRefPage.html
# For building mesaac_shape_eigen, Eigen 2 appears to have been
# installed system-wide.

# Once mesaac_shape_eigen has been updated, Eigen 3 can be incorporated
# into the build by uncommenting the following:
# # include(FetchContent)
# # FetchContent_Declare(
# #   eigen
# #   GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
# #   GIT_TAG 3.4.0  # Use your desired Eigen version tag
# #   GIT_SHALLOW TRUE
# # )

# FetchContent_MakeAvailable(eigen)

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

add_subdirectory(src)

if(SHAPE_FINGERPRINTS_BUILD_TESTS)
    # Source of CodeCoverage.cmake:
    # https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake
    include(cmake_modules/CodeCoverage.cmake)
    find_package(Catch2 3 REQUIRED)
    enable_testing()

    include(CTest)
    include(Catch)

    add_subdirectory(tests)
endif()
