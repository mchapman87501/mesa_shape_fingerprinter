find_package(Doxygen COMPONENTS doxygen)

if (DOXYGEN_FOUND)
    configure_file(Doxyfile.in Doxyfile @ONLY)
    add_custom_target(
        docs ALL
        COMMAND
            Doxygen::doxygen Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION share/doc/${PROJECT_NAME}
    )
endif()

# TODO: If we're hosting on Github, consider using https://codedocs.xyz/
# for hosting/generating documentation.