# Prepare for running gcovr:
set(GCOVR_EXECUTABLE gcovr)

# As noted in Professional CMake, when using the Clang toolchain, gcovr will
# sometimes fail.

add_custom_target(
  coverage_report_gcovr
  COMMAND
    ${GCOVR_EXECUTABLE} --root=${CMAKE_SOURCE_DIR}
    --filter=${CMAKE_SOURCE_DIR}/src --filter=${CMAKE_SOURCE_DIR}/tests
    --html-nested=${CMAKE_CURRENT_BINARY_DIR}/index.html
    --gcov-executable=${GCOV_EXECUTABLE}
  DEPENDS clean_run_ctest
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Generate gcovr coverage report")
