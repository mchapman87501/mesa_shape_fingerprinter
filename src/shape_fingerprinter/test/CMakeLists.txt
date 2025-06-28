set(SPF_EXE "$<TARGET_FILE:${TARGET}>")
configure_file(cmake_paths.py.in cmake_paths.py.gen.in @ONLY)
file(GENERATE
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/cmake_paths.py"
INPUT "${CMAKE_CURRENT_BINARY_DIR}/cmake_paths.py.gen.in"
)

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