set(TARGET mesaac_common)

find_package(ZLIB)

set(SRC
    gzip.cpp
    b32.cpp
    b64.cpp
)
# TODO move the header files into this directory, to ease
# their installation...
set(HEADER_DIR ${CMAKE_SOURCE_DIR}/include/${TARGET})
set(HEADERS
    ${HEADER_DIR}/b32.hpp
    ${HEADER_DIR}/b64.hpp
    ${HEADER_DIR}/gzip.hpp
)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET}
    PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET HEADERS 
        BASE_DIRS ${HEADER_DIR}
        FILES ${HEADERS}
)
target_link_libraries(${TARGET} PRIVATE ZLIB::ZLIB)

install(
    TARGETS ${TARGET} 
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${TARGET}
)
