set(TARGET mesaac_shape)

set(SRC src/axis_aligner.cpp src/fingerprinter.cpp src/hammersley.cpp
        src/vol_box.cpp)

set(HEADER_DIR include)
set(HEADERS
    ${HEADER_DIR}/mesaac_shape/axis_aligner.hpp
    ${HEADER_DIR}/mesaac_shape/fingerprinter.hpp
    ${HEADER_DIR}/mesaac_shape/hammersley.hpp
    ${HEADER_DIR}/mesaac_shape/shared_types.hpp
    ${HEADER_DIR}/mesaac_shape/vol_box.hpp)

add_library(${TARGET} STATIC ${SRC})
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_include_directories(${TARGET} PUBLIC ${HEADER_DIR})
# mesaac_common is needed only for shape_defs.hpp.  Should that header live
# elsewhere?
target_link_libraries(${TARGET} PUBLIC mesaac_common mesaac_mol svd ap
                                       Boost::dynamic_bitset)

target_sources(${TARGET} PUBLIC FILE_SET HEADERS BASE_DIRS ${HEADER_DIR} FILES
                                ${HEADERS})

install(TARGETS ${TARGET} FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
