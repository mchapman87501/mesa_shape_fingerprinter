set(TARGET shape_volume)

add_executable(${TARGET} shape_volume.cpp)
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_link_libraries(${TARGET} PRIVATE mesaac_mol mesaac_common
                                        mesaac_arg_parser svd ap)

install(TARGETS ${TARGET})
