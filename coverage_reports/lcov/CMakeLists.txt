# lcov behavior has changed from 1.x to 2.0, and from 2.0 to 2.3. Hence the
# following.
execute_process(COMMAND lcov "--version" OUTPUT_VARIABLE LCOV_VERSION_OUT)

# Update 20250704 attempt to address lcov 2.3.1-1 warning "WARNING:
# (unsupported) Function begin/end line exclusions not supported..." and
# subsequent genhtml errors:

# Presume LCOV version 1.x
set(LCOV_IGNORE_ARG "--ignore-errors=source")
if(LCOV_VERSION_OUT MATCHES "LCOV version 2\.3")
  set(LCOV_IGNORE_ARG "--ignore-errors=format,source,unsupported,inconsistent")
elseif(LCOV_VERSION_OUT MATCHES "LCOV version 2\.")
  set(LCOV_IGNORE_ARG "--ignore-errors=source,inconsistent,inconsistent")
endif()

set(GENHTML_IGNORE_ARG "")
if(LCOV_VERSION_OUT MATCHES "LCOV version 2\.3")
  set(GENHTML_IGNORE_ARG
      "--ignore-errors=source,source,category,inconsistent,corrupt")
endif()

set(CAPTURE_PROFILE_CMD
    lcov
    --capture
    --directory
    ${CMAKE_BINARY_DIR}
    -b
    ${PROJECT_SOURCE_DIR}
    --include
    ${PROJECT_SOURCE_DIR}/src
    --include
    ${PROJECT_SOURCE_DIR}/test
    --exclude
    ${PROJECT_SOURCE_DIR}/dependencies
    --exclude
    /Library/Developer
    --erase-functions
    __cxx_global_var_init
    ${LCOV_IGNORE_ARG}
    --output-file
    coverage.info)

set(GEN_REPORT_CMD
    genhtml
    ${GENHTML_IGNORE_ARG}
    --demangle-cpp
    --rc
    derive_function_end_line=0
    --hierarchical
    --output-directory
    ${CMAKE_CURRENT_BINARY_DIR}
    coverage.info)

add_custom_target(
  lcov_capture_profile_data
  DEPENDS clean_run_ctest
  COMMAND ${CAPTURE_PROFILE_CMD}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Capture profile data using lcov")

add_custom_target(
  coverage_report_lcov
  DEPENDS lcov_capture_profile_data
  COMMAND ${GEN_REPORT_CMD}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Generate coverage report using lcov")
