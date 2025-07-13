add_executable(alglib_svd_tests main.cpp testsvdunit.cpp)
target_include_directories(alglib_svd_tests PRIVATE .. ../libs)
target_link_libraries(alglib_svd_tests PRIVATE svd ap)
add_test(NAME alglib_svd_tests COMMAND alglib_svd_tests)
# Since ctest collects coverage data for svd routines, might as well include the
# tests that shipped with svd (i.e., label them "mesaac").
set_tests_properties(alglib_svd_tests PROPERTIES LABELS "mesaac;alglib")
