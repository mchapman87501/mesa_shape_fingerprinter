set(SRC
    axis_aligner.cpp
    fingerprinter.cpp
    hammersley.cpp
    vol_box.cpp
)

set(TARGET mesaac_shape)

add_library(
    ${TARGET} STATIC
    ${SRC}
)
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src/lib/svd ${CMAKE_SOURCE_DIR}/src/lib/svd/libs)
target_link_libraries(${TARGET} PRIVATE svd Boost::dynamic_bitset)