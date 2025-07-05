set(TARGET mesaac_arg_parser)

set(SRC src/arg_parser.cpp src/value_converter.cpp)

set(HEADER_DIR include)
set(LIB_HEADER_DIR ${HEADER_DIR}/mesaac_arg_parser)

set(HEADERS
    ${LIB_HEADER_DIR}/parse_result.hpp
    ${LIB_HEADER_DIR}/flag.hpp
    ${LIB_HEADER_DIR}/i_option.hpp
    ${LIB_HEADER_DIR}/i_argument.hpp
    ${LIB_HEADER_DIR}/option_base.hpp
    ${LIB_HEADER_DIR}/option.hpp
    ${LIB_HEADER_DIR}/choice.hpp
    ${LIB_HEADER_DIR}/multi_valued_option.hpp
    ${LIB_HEADER_DIR}/argument.hpp
    ${LIB_HEADER_DIR}/value_converter.hpp
    ${LIB_HEADER_DIR}/arg_parser.hpp)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${HEADER_DIR})

target_sources(${TARGET} PUBLIC FILE_SET HEADERS BASE_DIRS ${HEADER_DIR} FILES
                                ${HEADERS})

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
