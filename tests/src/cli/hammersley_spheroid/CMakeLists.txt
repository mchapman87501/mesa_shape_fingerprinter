configure_file(config.py.in config.py.gen.in @ONLY)
file(GENERATE
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/config.py"
INPUT "${CMAKE_CURRENT_BINARY_DIR}/config.py.gen.in"
)

# Copy all of the Python files to the binary directory...
set(PYTHON_SRC test_basics.py)
foreach(PYFILE ${PYTHON_SRC})
configure_file(${PYFILE} ${PYFILE} COPYONLY)
endforeach()

add_test(
    NAME hammersley_spheroid_test_basics
    COMMAND Python3::Interpreter test_basics.py
    )
set_tests_properties(hammersley_spheroid_test_basics PROPERTIES LABELS "mesaac")