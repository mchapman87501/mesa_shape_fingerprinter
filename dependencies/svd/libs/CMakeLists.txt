set(PARENT_TARGET ${TARGET})
set(TARGET ap)

add_library(${TARGET} STATIC)
target_sources(
  ${TARGET}
  PRIVATE ap.cpp
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}
         FILES
         ap.h
         apvt.h)
install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PARENT_TARGET}/${TARGET})
