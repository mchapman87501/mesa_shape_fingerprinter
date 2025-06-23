add_executable(test_axis_aligner test_axis_aligner.cpp)
target_include_directories(test_axis_aligner PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_axis_aligner PRIVATE mesaac_shape mesaac_mol Catch2::Catch2WithMain)

# add_executable(test_fingerprinter test_fingerprinter.cpp)
# add_executable(test_hammersley test_hammersley.cpp)
# add_executable(test_vol_box test_vol_box.cpp)