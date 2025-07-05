# Most of this is from "Professional CMake: A Practical Guide", chapter 33,
# "Dynamic Code Analysis". It defines settings for a "COVERAGE" build type.
if(PROJECT_IS_TOP_LEVEL AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS_COVERAGE "-g -O0 --coverage")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    string(APPEND CMAKE_CXX_FLAGS_COVERAGE " -fprofile-abs-path")
  endif()
  set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")
  set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "--coverage")
  set(CMAKE_MODULE_LINKER_FLAGS_COVERAGE "--coverage")
endif()

# ---------
# Define a default configuration for gcovr.
if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  execute_process(
    COMMAND xcrun --find gcov
    OUTPUT_VARIABLE GCOV_EXECUTABLE
    OUTPUT_STRIP_TRAILING_WHITESPACE)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  find_program(LLVM_COV_EXECUTABLE llvm-cov REQUIRED)
  file(CREATE_LINK ${LLVM_COV_EXECUTABLE} ${CMAKE_BINARY_DIR}/gcov SYMBOLIC)
  set(GCOV_EXECUTABLE "${LLVM_COV_EXECUTABLE} “gcov")
else() # Assuming gcc for this example
  find_program(GCOV_EXECUTABLE gcov REQUIRED)
endif()

add_custom_target(
  reset_coverage_counters
  COMMAND find ${CMAKE_BINARY_DIR} -name '*.gcda' -delete
  COMMENT "Remove coverage counters")

add_custom_target(
  clean_run_ctest
  COMMAND ctest --preset coverage
  DEPENDS reset_coverage_counters
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Re-run ctest")
