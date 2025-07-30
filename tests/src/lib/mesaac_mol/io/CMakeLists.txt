add_mesaac_test(TEST_NAME test_sdreader SOURCES test_sdreader.cpp LIBS
                mesaac_mol)
add_mesaac_test(TEST_NAME test_sdreader_v3000 SOURCES test_sdreader_v3000.cpp
                LIBS mesaac_mol)
add_mesaac_test(TEST_NAME test_sdwriter SOURCES test_sdwriter.cpp LIBS
                mesaac_mol)

add_subdirectory(internal)
