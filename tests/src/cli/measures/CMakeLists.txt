# Compiled tests:

add_mesaac_test(
  TEST_NAME
  test_measure_type_converter
  SOURCES
  test_measure_type_converter.cpp
  LIBS
  cli_measures_lib
  mesaac_common
  mesaac_measures)

# Python test drivers:
configure_file(config.py.in config.py.gen.in @ONLY)
file(
  GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/config.py"
  INPUT "${CMAKE_CURRENT_BINARY_DIR}/config.py.gen.in")

set(TEST_SCRIPTS test_measures_nxn test_measures_shape_fp test_measures_sim)
foreach(SCRIPT_NAME ${TEST_SCRIPTS})
  set(TEST_NAME "test_cli_measures_${SCRIPT_NAME}")
  add_test(NAME ${TEST_NAME}
           COMMAND Python3::Interpreter
                   ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT_NAME}.py)
  set_tests_properties(
    ${TEST_NAME}
    PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
               LABELS "mesaac;mesaac_measures_cli")
endforeach()
