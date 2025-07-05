set(TARGET hammersley_spheroid)

add_executable(${TARGET} hammersley_spheroid.cpp)
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_link_libraries(${TARGET} PRIVATE mesaac_arg_parser)

install(TARGETS ${TARGET} DESTINATION bin)

# TODO Add usage tests -- invalid/missing command-line args, etc.

# TODO Add a regression test that verifies expected output
# for, e.g., dimension 3 and ... 50 points.
