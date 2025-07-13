set(TARGET mesaac_measures)

set(SRC
    src/bub.cpp
    src/cosine.cpp
    src/euclidean.cpp
    src/hamann.cpp
    src/measures_base.cpp
    src/measures_factory.cpp
    src/shape_measures_factory.cpp
    src/tversky.cpp)

set(HEADER_DIR include)
set(HEADERS
    ${HEADER_DIR}/mesaac_measures/bub.hpp
    ${HEADER_DIR}/mesaac_measures/cosine.hpp
    ${HEADER_DIR}/mesaac_measures/euclidean.hpp
    ${HEADER_DIR}/mesaac_measures/hamann.hpp
    ${HEADER_DIR}/mesaac_measures/measures_base.hpp
    ${HEADER_DIR}/mesaac_measures/measures_factory.hpp
    ${HEADER_DIR}/mesaac_measures/shape_measures_factory.hpp
    ${HEADER_DIR}/mesaac_measures/tversky.hpp)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${HEADER_DIR})
target_link_libraries(${TARGET} PUBLIC mesaac_shape)
target_sources(${TARGET} PUBLIC FILE_SET HEADERS BASE_DIRS ${HEADER_DIR} FILES
                                ${HEADERS})

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
