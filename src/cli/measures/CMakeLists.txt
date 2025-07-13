function(add_measures_exe TARGET)
  set(SRC ${ARGN})
  add_executable(${TARGET} ${SRC})
  target_compile_features(${TARGET} PUBLIC cxx_std_20)
  target_link_libraries(${TARGET} PRIVATE cli_measures_lib mesaac_arg_parser)
  install(TARGETS ${TARGET})
endfunction()

add_library(cli_measures_lib STATIC fingerprint_reader.cpp
                                    measure_type_converter.cpp)
target_compile_features(cli_measures_lib PUBLIC cxx_std_20)
target_include_directories(cli_measures_lib PUBLIC .)
target_link_libraries(cli_measures_lib PUBLIC mesaac_measures mesaac_common)

add_measures_exe(measures_nxn measures_nxn.cpp)

# add_measures_exe( measures_count_nxn measures_count_nxn.cpp
# tanimoto_counts.cpp tversky_counts.cpp euclidean_counts.cpp cosine_counts.cpp
# )

# add_measures_exe(usr_measures usr_measures_main.cpp usr_s_measure.cpp )

# add_measures_exe(measures_pvm measures_pvm.cpp)

add_measures_exe(measures_sim measures_sim.cpp)

# add_measures_exe(measures_all measures_all.cpp)

add_measures_exe(measures_shape_fp measures_shape_fp.cpp)

# add_measures_exe(measures_sfp_band measures_sfp_band.cpp )

add_subdirectory(find_diverse)
