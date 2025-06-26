file(REAL_PATH ${CMAKE_SOURCE_DIR}/tests/data TEST_DATA_DIR)

# Define a simple Catch2 test executable.
# The executable is compiled with the C++ preprocessor symbol `TEST_DATA_DIR`
# set to the path of the CMake project's toplevel `tests/data` directory.
# Any Catch2 `TEST_CASE` tags are added to the test as CMake test labels.
# 
# Arguments:
# TEST_NAME - name of the executable to build
# SOURCES - source files from which to build the executable
# LIBS - libraries against which to link the executable
# Example:
#   add_mesaac_test(foo foo.cpp foo_funcs.cpp)
function(add_mesaac_test)
    set(flags)
    set(single_values TEST_NAME)
    set(multi_values SOURCES LIBS)
    cmake_parse_arguments(
        arg
        "${flags}" "${single_values}" "${multi_values}"
        ${ARGN}
    )

    add_executable(${arg_TEST_NAME} ${arg_SOURCES})
    target_compile_definitions(${arg_TEST_NAME} PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}")
    target_include_directories(${arg_TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${arg_TEST_NAME} PRIVATE ${arg_LIBS} Catch2::Catch2WithMain)

    catch_discover_tests(${arg_TEST_NAME} ADD_TAGS_AS_LABELS)
endfunction()
