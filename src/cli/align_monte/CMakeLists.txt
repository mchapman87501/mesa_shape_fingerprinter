set(TARGET align_monte)

set(SRC align_monte.cpp mol_aligner.cpp sdf_mol_aligner.cpp)

# Use OpenMP if it is available
find_package(OpenMP)

add_executable(${TARGET} ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_link_libraries(
  ${TARGET}
  PRIVATE mesaac_mol
          mesaac_common
          mesaac_measures
          mesaac_shape
          mesaac_arg_parser
          svd
          ap)
if(OpenMP_FOUND)
  target_compile_definitions(${TARGET} PRIVATE HAVE_OPENMP=1)
  target_link_libraries(${TARGET} PRIVATE OpenMP::OpenMP_CXX)
endif()

install(TARGETS ${TARGET})
