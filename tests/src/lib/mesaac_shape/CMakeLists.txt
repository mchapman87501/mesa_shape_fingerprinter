function(add_mesaac_shape_test NAME)
  add_mesaac_test(
    TEST_NAME
    ${NAME}
    SOURCES
    "${NAME}.cpp"
    LIBS
    mesaac_shape
    mesaac_mol)
endfunction()

add_mesaac_shape_test(test_axis_aligner)
if(PROVIDE_EIGEN)
  add_mesaac_shape_test(test_axis_aligner_eigen)
endif()
add_mesaac_shape_test(test_fingerprinter)
add_mesaac_shape_test(test_hammersley)
add_mesaac_shape_test(test_vol_box)
