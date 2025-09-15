set(TARGET svd)

add_library(${TARGET} STATIC)
target_sources(
  ${TARGET}
  PRIVATE bdsvd.cpp
          bidiagonal.cpp
          blas.cpp
          lq.cpp
          qr.cpp
          reflections.cpp
          rotations.cpp
          svd.cpp
  PRIVATE FILE_SET
          privateHeaders
          TYPE
          HEADERS
          BASE_DIRS
          ${CMAKE_CURRENT_SOURCE_DIR}/libs
          FILES
          libs/stdafx.h
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}
         FILES
         bdsvd.h
         bidiagonal.h
         blas.h
         lq.h
         qr.h
         reflections.h
         rotations.h
         svd.h)

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${TARGET})

add_subdirectory(libs)

if(SHAPE_FINGERPRINTS_BUILD_TESTS)
  add_subdirectory(tests)
endif()
