# Add wrappers around the mesa libraries of interest.
set(SRC
    TypeAliases.swift
    Position.swift
    Atom.swift
    Bond.swift
    Mol.swift
    V2000SDReader.swift
    mesaac_swift.swift)
set(TARGET mesaac_swift)
add_library(${TARGET} STATIC ${SRC})
set_target_properties(${TARGET} PROPERTIES Swift_MODULE_NAME "MesaacSwift")
target_compile_options(
  ${TARGET}
  PUBLIC "$<$<COMPILE_LANGUAGE:Swift>:-cxx-interoperability-mode=default>")
target_compile_options(${TARGET} PRIVATE "-emit-module")

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  target_compile_options(
    ${TARGET} PUBLIC "$<$<COMPILE_LANGUAGE:Swift>:-profile-generate>"
                     "$<$<COMPILE_LANGUAGE:Swift>:-profile-coverage-mapping>")
  target_link_options(
    ${TARGET} PUBLIC "$<$<COMPILE_LANGUAGE:Swift>:-profile-generate>"
    "$<$<COMPILE_LANGUAGE:Swift>:-profile-coverage-mapping>")
endif()

# Swift can't figure out where to find the boost headers used by mesaac_common.
target_include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/src/lib)
target_link_libraries(${TARGET} PUBLIC mesaac_common mesaac_measures mesaac_mol
                                       Boost::dynamic_bitset)
