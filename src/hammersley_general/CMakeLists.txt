set(TARGET hammersley_general)

add_executable(${TARGET} hammersley_general.cpp)
target_compile_features(${TARGET} PUBLIC cxx_std_20)

# TODO Add usage tests -- invalid/missing command-line args, etc.

# TODO Add a regression test that verifies expected output
# for, e.g., dimension 3 and ... 50 points.
