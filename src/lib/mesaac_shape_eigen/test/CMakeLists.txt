add_mesaac_test(TEST_NAME test_axis_aligner_eigen SOURCES test_axis_aligner.cpp LIBS mesaac_shape_eigen mesaac_mol ${OpenBabel_LIBRARIES})
# add_mesaac_test(TEST_NAME test_fingerprinter_eigen SOURCES test_fingerprinter.cpp LIBS mesaac_shape_eigen mesaac_mol ${OpenBabel_LIBRARIES})
add_mesaac_test(TEST_NAME test_hammersley_eigen SOURCES test_hammersley.cpp LIBS mesaac_shape_eigen mesaac_mol ${OpenBabel_LIBRARIES})
# add_mesaac_test(TEST_NAME test_vol_box_eigen SOURCES test_vol_box.cpp LIBS mesaac_shape_eigen mesaac_mol ${OpenBabel_LIBRARIES})