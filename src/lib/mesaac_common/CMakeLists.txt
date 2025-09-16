set(TARGET mesaac_common)

find_package(ZLIB)

add_library(${TARGET} STATIC)
target_sources(
  ${TARGET}
  PRIVATE src/gzip.cpp src/b32.cpp src/b64.cpp
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         include
         FILES
         include/mesaac_common/b32.hpp
         include/mesaac_common/b64.hpp
         include/mesaac_common/gzip.hpp)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

# Boost dynamic_bitset is not a library.  But this is needed in order for
# dependent targets to learn the path to Boost header files.
target_link_libraries(${TARGET} PRIVATE ZLIB::ZLIB Boost::dynamic_bitset)

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
