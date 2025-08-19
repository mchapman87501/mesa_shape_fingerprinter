# Most of this is from "Professional CMake: A Practical Guide", chapter 33,
# "Dynamic Code Analysis". It defines settings for a "COVERAGE" build type.
if(PROJECT_IS_TOP_LEVEL AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS_COVERAGE "-g -O0 --coverage")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    string(APPEND CMAKE_CXX_FLAGS_COVERAGE
           " -fprofile-abs-path -fprofile-update=atomic")
  endif()
  set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")
  set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "--coverage")
  set(CMAKE_MODULE_LINKER_FLAGS_COVERAGE "--coverage")
endif()

add_custom_target(
  reset_coverage_counters
  COMMAND find ${CMAKE_BINARY_DIR} -name '*.gcda' -delete
  COMMENT "Remove coverage counters")

add_custom_target(
  clean_run_ctest_benchmarks
  COMMAND ctest --preset coverage-report
  DEPENDS reset_coverage_counters
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Re-run ctest, including benchmarks")
