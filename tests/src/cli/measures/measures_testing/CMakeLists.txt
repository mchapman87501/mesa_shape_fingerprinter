configure_file(config.py.in config.py.gen.in @ONLY)
file(
  GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/config.py"
  INPUT "${CMAKE_CURRENT_BINARY_DIR}/config.py.gen.in")

# Copy all of the Python files to the binary directory...
set(PYTHON_SRC
    __init__.py
    demo_test_measures.py
    expected_results_generator.py
    fp_file_generator.py
    fp_measurer.py
    fp_utils.py
    i_measure.py
    i_shape_measure.py
    measure_factory.py
    measure.py
    measures_runner.py
    results_reader.py
    results_verifier_factory.py
    results_verifier.py
    shape_measure.py
    testfactory.py)

foreach(PYFILE ${PYTHON_SRC})
  configure_file(${PYFILE} ${PYFILE} COPYONLY)
endforeach()
