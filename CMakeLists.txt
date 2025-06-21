cmake_minimum_required(VERSION 4.0)
set(PROJECT shape_fingerprints)

project(${PROJECT} VERSION 1.0.0)

# Facilitate *not* building tests when using via FetchContent_MakeAvailable.
# See https://github.com/glfw/glfw/blob/master/CMakeLists.txt
option(SHAPE_FINGERPRINTS_BUILD_TESTS "Build test targets" ${PROJECT_IS_TOP_LEVEL})


# add_executable(shape_fingerprints src/shape_fingerprints.cpp)
# target_compile_features(shape_fingerprints PUBLIC cxx_std_23)

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
