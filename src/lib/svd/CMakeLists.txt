set(TARGET svd)

set(SRC
    bdsvd.cpp
    bidiagonal.cpp
    blas.cpp
    lq.cpp
    qr.cpp
    reflections.cpp
    rotations.cpp
    svd.cpp
)
add_library(${TARGET} STATIC ${SRC})
target_include_directories(${TARGET} PUBLIC libs)

add_subdirectory(libs)

