configure_file(config.py.in config.py.gen.in @ONLY)
file(
  GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/config.py"
  INPUT "${CMAKE_CURRENT_BINARY_DIR}/config.py.gen.in")

# Copy all of the Python files to the binary directory... set(PYTHON_TESTS
# test_measures_all.py test_measures_nxn.py test_measures_sfp_band.py
# test_measures_shape_fp.py test_measures_sim.py)
set(PYTHON_TESTS test_measures_nxn.py test_measures_shape_fp.py
                 test_measures_sim.py)

set(PYTHON_SRC
    ${PYTHON_TESTS} compare_matrices.py gen_ref_in_count_order_files.py
    mesa_unittest.py validate_matrix.py)

foreach(PYFILE ${PYTHON_SRC})
  configure_file(${PYFILE} ${PYFILE} COPYONLY)
endforeach()

# Convert each of the Python unittest modules to a CMake test.
foreach(TEST_SCRIPT ${PYTHON_TESTS})
  string(REPLACE "\.py" "" TEST_STEM ${TEST_SCRIPT})
  string(REPLACE "^test_" "test_cli_measures_" TEST_NAME ${TEST_STEM})
  add_test(NAME ${TEST_NAME} COMMAND Python3::Interpreter ${TEST_SCRIPT})
  set_tests_properties(${TEST_NAME} PROPERTIES LABELS
                                               "mesaac;mesaac_measures_cli")
endforeach()

add_subdirectory(measures_testing)
