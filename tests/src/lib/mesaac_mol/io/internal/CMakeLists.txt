function(add_io_internal_test)
  set(flags)
  set(single_values TEST_NAME)
  set(multi_values SOURCES LIBS)
  cmake_parse_arguments(arg "${flags}" "${single_values}" "${multi_values}"
                        ${ARGN})

  add_executable(${arg_TEST_NAME} ${arg_SOURCES})
  target_compile_definitions(${arg_TEST_NAME}
                             PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}")
  target_include_directories(
    ${arg_TEST_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src/lib/mesaac_mol/src/io/internal)
  target_link_libraries(${arg_TEST_NAME} PRIVATE ${arg_LIBS}
                                                 Catch2::Catch2WithMain)

  catch_discover_tests(${arg_TEST_NAME} ADD_TAGS_AS_LABELS)

endfunction()

add_io_internal_test(TEST_NAME test_line_reader SOURCES test_line_reader.cpp
                     LIBS mesaac_mol)
add_io_internal_test(TEST_NAME test_mol_header_block SOURCES
                     test_mol_header_block.cpp LIBS mesaac_mol)
add_io_internal_test(TEST_NAME test_sdtags_reader SOURCES
                     test_sdtags_reader.cpp LIBS mesaac_mol)

add_io_internal_test(TEST_NAME test_v2000_prop_block_reader SOURCES
                     test_v2000_prop_block_reader.cpp LIBS mesaac_mol)
add_io_internal_test(TEST_NAME test_v2000_ctab_reader SOURCES
                     test_v2000_ctab_reader.cpp LIBS mesaac_mol)
add_io_internal_test(TEST_NAME test_v3000_atom_prop_reader SOURCES
                     test_v3000_atom_prop_reader.cpp LIBS mesaac_mol)
add_io_internal_test(TEST_NAME test_v3000_ctab_reader SOURCES
                     test_v3000_ctab_reader.cpp LIBS mesaac_mol)
