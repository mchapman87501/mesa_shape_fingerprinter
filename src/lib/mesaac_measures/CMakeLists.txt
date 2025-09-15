set(TARGET mesaac_measures)

add_library(${TARGET} STATIC)
target_sources(
  ${TARGET}
  PRIVATE src/bub.cpp
          src/cosine.cpp
          src/euclidean.cpp
          src/hamann.cpp
          src/measures_base.cpp
          src/measures_factory.cpp
          src/shape_measures_factory.cpp
          src/tversky.cpp
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         include
         FILES
         include/mesaac_measures/mesaac_measures.hpp
         include/mesaac_measures/bub.hpp
         include/mesaac_measures/cosine.hpp
         include/mesaac_measures/euclidean.hpp
         include/mesaac_measures/hamann.hpp
         include/mesaac_measures/measures_base.hpp
         include/mesaac_measures/measures_factory.hpp
         include/mesaac_measures/shape_measures_factory.hpp
         include/mesaac_measures/tversky.hpp)

target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_link_libraries(${TARGET} PUBLIC mesaac_shape)

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
