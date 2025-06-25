file(REAL_PATH ${CMAKE_SOURCE_DIR}/tests/data TEST_DATA_DIR)

# Define a simple Catch2 test executable.
# The executable is compiled with the C++ preprocessor symbol `TEST_DATA_DIR`
# set to the path of the CMake project's toplevel `tests/data` directory.
# Any Catch2 `TEST_CASE` tags are added to the test as CMake test labels.
# 
# Arguments:
# TEST_NAME - the name of the executable to build
# SOURCES - the source files from which to build the executable
#
# Example:
#   add_mesaac_test(foo foo.cpp foo_funcs.cpp)
function(add_mesaac_test TEST_NAME)
    set(SOURCES ${ARGN})
    add_executable(${TEST_NAME} ${SOURCES})
    target_compile_definitions(${TEST_NAME} PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}")
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${TEST_NAME} PRIVATE mesaac_shape mesaac_mol Catch2::Catch2WithMain)

    catch_discover_tests(${TEST_NAME} ADD_TAGS_AS_LABELS)
endfunction()
