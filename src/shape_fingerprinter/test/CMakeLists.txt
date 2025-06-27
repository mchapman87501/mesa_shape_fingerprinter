# STUB
message(STATUS "DEBUG: Preparing the test dir")
configure_file(cmake_paths.py.in cmake_paths.py)

# Copy all of the Python files to the binary directory...
set(PYTHON_SRC test_basics.py testsupport.py)
foreach(PYFILE ${PYTHON_SRC})
configure_file(${PYFILE} ${PYFILE} COPYONLY)
endforeach()

add_test(
    NAME shape_fingerprinter_test_basics
    COMMAND python3 test_basics.py
    )
set_tests_properties(shape_fingerprinter_test_basics PROPERTIES LABELS "mesaac")