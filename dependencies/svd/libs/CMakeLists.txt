set(PARENT_TARGET ${TARGET})
set(TARGET ap)

set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADERS
    ${HEADER_DIR}/ap.h
    ${HEADER_DIR}/apvt.h
)

add_library(${TARGET} STATIC ap.cpp)
target_include_directories(${TARGET} PUBLIC ${HEADER_DIR})
target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET HEADERS 
        BASE_DIRS ${HEADER_DIR}
        FILES ${HEADERS}
)
install(
    TARGETS ${TARGET}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PARENT_TARGET}/${TARGET}
)
