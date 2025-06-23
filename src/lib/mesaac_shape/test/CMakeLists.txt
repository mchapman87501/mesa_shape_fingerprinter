file(REAL_PATH ${CMAKE_SOURCE_DIR}/tests/data TEST_DATA_DIR)

add_executable(test_axis_aligner test_axis_aligner.cpp)
target_compile_definitions(test_axis_aligner PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}")
target_include_directories(test_axis_aligner PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_axis_aligner PRIVATE mesaac_shape mesaac_mol Catch2::Catch2WithMain)

add_test(NAME test_axis_aligner COMMAND test_axis_aligner)
set_tests_properties(test_axis_aligner PROPERTIES LABELS mesaac_tests)

# add_executable(test_fingerprinter test_fingerprinter.cpp)
# add_executable(test_hammersley test_hammersley.cpp)
# add_executable(test_vol_box test_vol_box.cpp)