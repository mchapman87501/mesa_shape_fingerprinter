set(TARGET mesaac_shape)

set(SRC
    axis_aligner.cpp
    fingerprinter.cpp
    hammersley.cpp
    vol_box.cpp
)

set(HEADER_DIR ${CMAKE_SOURCE_DIR}/include/${TARGET})
set(HEADERS
    ${HEADER_DIR}/axis_aligner.hpp
    ${HEADER_DIR}/fingerprinter.hpp
    ${HEADER_DIR}/hammersley.hpp
    ${HEADER_DIR}/shared_types.hpp
    ${HEADER_DIR}/vol_box.hpp
)


add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/dependencies/svd ${CMAKE_SOURCE_DIR}/dependencies/svd/libs)
target_link_libraries(${TARGET} PUBLIC svd ap Boost::dynamic_bitset)

target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET HEADERS 
        BASE_DIRS ${HEADER_DIR}
        FILES ${HEADERS}
)

install(
    TARGETS ${TARGET}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${TARGET}
)
