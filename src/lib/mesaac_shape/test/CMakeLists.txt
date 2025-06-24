file(REAL_PATH ${CMAKE_SOURCE_DIR}/tests/data TEST_DATA_DIR)

function(add_mesaac_shape_test TEST_NAME)
    set(SOURCES ${ARGN})
    message(STATUS "DEBUG add_mesaac_shape_test NAME ${TEST_NAME} SOURCES ${SOURCES}")
    add_executable(${TEST_NAME} ${SOURCES})
    target_compile_definitions(${TEST_NAME} PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}")
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${TEST_NAME} PRIVATE mesaac_shape mesaac_mol Catch2::Catch2WithMain)

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES LABELS mesaac_tests)
endfunction()

add_mesaac_shape_test(test_axis_aligner test_axis_aligner.cpp)
add_mesaac_shape_test(test_fingerprinter test_fingerprinter.cpp)

# add_executable(test_hammersley test_hammersley.cpp)
# add_executable(test_vol_box test_vol_box.cpp)