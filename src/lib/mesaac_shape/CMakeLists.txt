set(TARGET mesaac_shape)

add_library(${TARGET} STATIC)
target_sources(
  ${TARGET}
  PRIVATE src/axis_aligner.cpp src/fingerprinter.cpp src/hammersley.cpp
          src/vol_box.cpp
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         include
         FILES
         include/mesaac_shape/axis_aligner.hpp
         include/mesaac_shape/fingerprinter.hpp
         include/mesaac_shape/hammersley.hpp
         include/mesaac_shape/shared_types.hpp
         include/mesaac_shape/vol_box.hpp)

target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_link_libraries(${TARGET} PUBLIC mesaac_measures mesaac_mol svd ap
                                       Boost::dynamic_bitset)

if(PROVIDE_EIGEN)
  target_sources(
    ${TARGET}
    PRIVATE src/axis_aligner_eigen.cpp
    PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
           include/mesaac_shape/axis_aligner_eigen.hpp)

  target_link_libraries(${TARGET} PUBLIC Eigen3::Eigen)
endif()

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
