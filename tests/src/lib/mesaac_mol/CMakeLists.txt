add_mesaac_test(TEST_NAME test_element_info SOURCES test_element_info.cpp LIBS
                mesaac_mol)
add_mesaac_test(TEST_NAME test_atom SOURCES test_atom.cpp LIBS mesaac_mol)
add_mesaac_test(TEST_NAME test_bond SOURCES test_bond.cpp LIBS mesaac_mol)
add_mesaac_test(TEST_NAME test_mol SOURCES test_mol.cpp LIBS mesaac_mol)

add_subdirectory(io)
