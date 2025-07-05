add_mesaac_test(TEST_NAME test_value_converter SOURCES test_value_converter.cpp
                LIBS mesaac_arg_parser)
add_mesaac_test(TEST_NAME test_flag SOURCES test_flag.cpp LIBS
                mesaac_arg_parser)
add_mesaac_test(TEST_NAME test_option SOURCES test_option.cpp LIBS
                mesaac_arg_parser)
add_mesaac_test(TEST_NAME test_choice SOURCES test_choice.cpp LIBS
                mesaac_arg_parser)
add_mesaac_test(TEST_NAME test_argument SOURCES test_argument.cpp LIBS
                mesaac_arg_parser)
add_mesaac_test(TEST_NAME test_arg_parser SOURCES test_arg_parser.cpp LIBS
                mesaac_arg_parser)
