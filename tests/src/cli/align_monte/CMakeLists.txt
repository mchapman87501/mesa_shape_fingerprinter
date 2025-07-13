configure_file(config.py.in config.py.gen.in @ONLY)
file(
  GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/config.py"
  INPUT "${CMAKE_CURRENT_BINARY_DIR}/config.py.gen.in")

# Copy all of the Python files to the binary directory...
set(PYTHON_SRC base_test_case.py test_basics.py test_usage.py)
foreach(PYFILE ${PYTHON_SRC})
  configure_file(${PYFILE} ${PYFILE} COPYONLY)
endforeach()

set(TEST_SCRIPTS test_basics test_usage)
foreach(SCRIPT_NAME ${TEST_SCRIPTS})
  set(TEST_NAME "align_monte_${SCRIPT_NAME}")
  add_test(NAME ${TEST_NAME} COMMAND Python3::Interpreter ${SCRIPT_NAME}.py)
  set_tests_properties(${TEST_NAME} PROPERTIES LABELS "mesaac;align_monte")
endforeach()
