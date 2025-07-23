set(TARGET mol_demo)

add_executable(${TARGET} mol_demo.swift)
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  target_compile_options(
    ${TARGET} PUBLIC "$<$<COMPILE_LANGUAGE:Swift>:-profile-generate>"
                     "$<$<COMPILE_LANGUAGE:Swift>:-profile-coverage-mapping>")
  target_link_options(
    ${TARGET} PUBLIC "$<$<COMPILE_LANGUAGE:Swift>:-profile-generate>"
    "$<$<COMPILE_LANGUAGE:Swift>:-profile-coverage-mapping>")
endif()

target_link_libraries(${TARGET} PRIVATE mesaac_swift)
