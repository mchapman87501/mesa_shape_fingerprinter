configure_file(config.py.in config.py.gen.in @ONLY)
file(
  GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/config.py"
  INPUT "${CMAKE_CURRENT_BINARY_DIR}/config.py.gen.in")

set(TEST_SCRIPTS test_basics)
foreach(SCRIPT_NAME ${TEST_SCRIPTS})
  set(TEST_NAME "hammersley_spheroid_${SCRIPT_NAME}")
  add_test(NAME ${TEST_NAME}
           COMMAND Python3::Interpreter
                   ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT_NAME}.py)
  set_tests_properties(
    ${TEST_NAME}
    PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
               LABELS "mesaac")
endforeach()
