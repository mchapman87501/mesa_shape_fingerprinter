set(TARGET svd)

set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADERS
    ${HEADER_DIR}/bdsvd.h
    ${HEADER_DIR}/bidiagonal.h
    ${HEADER_DIR}/blas.h
    ${HEADER_DIR}/lq.h
    ${HEADER_DIR}/qr.h
    ${HEADER_DIR}/reflections.h
    ${HEADER_DIR}/rotations.h
    ${HEADER_DIR}/svd.h
)

set(SRC
    bdsvd.cpp
    bidiagonal.cpp
    blas.cpp
    lq.cpp
    qr.cpp
    reflections.cpp
    rotations.cpp
    svd.cpp
)
add_library(${TARGET} STATIC ${SRC})
target_include_directories(${TARGET} PUBLIC ${HEADER_DIR} libs)

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

add_subdirectory(libs)

if(SHAPE_FINGERPRINTS_BUILD_TESTS)
add_subdirectory(tests)
endif()