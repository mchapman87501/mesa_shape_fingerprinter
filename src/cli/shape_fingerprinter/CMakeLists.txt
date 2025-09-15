set(TARGET shape_fingerprinter)

add_executable(${TARGET} shape_fingerprinter.cpp sdf_shape_fingerprinter.cpp
                         mol_fingerprinter.cpp)
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_link_libraries(${TARGET} PRIVATE mesaac_shape mesaac_mol mesaac_common
                                        mesaac_arg_parser)

install(TARGETS ${TARGET})
