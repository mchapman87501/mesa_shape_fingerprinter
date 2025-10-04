set(TARGET align_monte)

set(SRC align_monte.cpp mol_aligner.cpp sdf_mol_aligner.cpp)

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

install(TARGETS ${TARGET})
