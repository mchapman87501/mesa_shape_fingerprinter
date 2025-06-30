set(TARGET shape_fingerprinter)

set(SRC shape_fingerprinter.cpp sdf_shape_fingerprinter.cpp mol_fingerprinter.cpp)

add_executable(${TARGET} ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_link_libraries(${TARGET} PRIVATE mesaac_shape mesaac_mol mesaac_common)

install(TARGETS ${TARGET} DESTINATION bin)

add_subdirectory(test)