set(TARGET mesaac_common)

find_package(ZLIB)

set(SRC src/gzip.cpp src/b32.cpp src/b64.cpp src/shape_defs.cpp)
# TODO move the header files into this directory, to ease their installation...
set(HEADER_DIR include)
set(HEADERS
    ${HEADER_DIR}/mesaac_common/b32.hpp ${HEADER_DIR}/mesaac_common/b64.hpp
    ${HEADER_DIR}/mesaac_common/gzip.hpp
    ${HEADER_DIR}/mesaac_common/shape_defs.hpp)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${HEADER_DIR} ${BOO})

target_sources(${TARGET} PUBLIC FILE_SET HEADERS BASE_DIRS ${HEADER_DIR} FILES
                                ${HEADERS})
# Boost dynamic_bitset is not a library.  But this is needed in order for
# dependent targets to learn the path to Boost header files.
target_link_libraries(${TARGET} PRIVATE ZLIB::ZLIB Boost::dynamic_bitset)

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
