set(TARGET mesaac_mol)

set(SRC 
    bond.cpp
    atom.cpp
    io/sdwriter.cpp
    io/sdreader.cpp
    mol.cpp
    element_info.cpp
)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/include)