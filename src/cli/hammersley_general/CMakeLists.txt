set(TARGET hammersley_general)

add_executable(${TARGET} hammersley_general.cpp)
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_link_libraries(${TARGET} PRIVATE mesaac_arg_parser)

install(TARGETS ${TARGET})
