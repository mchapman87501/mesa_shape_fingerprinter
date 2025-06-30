add_executable(test_alglib_svd main.cpp testsvdunit.cpp)
target_include_directories(test_alglib_svd PRIVATE  ${CMAKE_SOURCE_DIR}/src/lib/svd ${CMAKE_SOURCE_DIR}/src/lib/svd/libs)
target_link_libraries(test_alglib_svd PRIVATE svd ap)
add_test(NAME test_alglib_svd COMMAND test_alglib_svd)
# Since ctest collects coverage data for svd routines, might as well
# include the tests that shipped with svd (i.e., label them "mesaac").
set_tests_properties(test_alglib_svd PROPERTIES LABELS "mesaac;alglib")