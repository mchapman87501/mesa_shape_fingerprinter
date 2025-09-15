set(TARGET mesaac_arg_parser)
set(HEADER_DIR include/mesaac_arg_parser)

add_library(${TARGET} STATIC)

target_sources(
  ${TARGET}
  PRIVATE src/arg_parser.cpp src/value_converter.cpp
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         include
         FILES
         ${HEADER_DIR}/parse_result.hpp
         ${HEADER_DIR}/flag.hpp
         ${HEADER_DIR}/i_option.hpp
         ${HEADER_DIR}/i_argument.hpp
         ${HEADER_DIR}/option_base.hpp
         ${HEADER_DIR}/option.hpp
         ${HEADER_DIR}/choice.hpp
         ${HEADER_DIR}/multi_valued_option.hpp
         ${HEADER_DIR}/argument.hpp
         ${HEADER_DIR}/value_converter.hpp
         ${HEADER_DIR}/arg_parser.hpp)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
