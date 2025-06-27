set(TARGET mesaac_common)

find_package(ZLIB)

set(SRC
    gzip.cpp
    b32.cpp
    b64.cpp
)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET}
    PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${TARGET} PRIVATE ZLIB::ZLIB)
add_subdirectory(test)