add_subdirectory(lib)
add_subdirectory(cli)

include(CheckLanguage)
check_language(Swift)

if(CMAKE_Swift_COMPILER STREQUAL "NOTFOUND")
  message(STATUS "Skipping build of ${CMAKE_CURRENT_SOURCE_DIR}/swift.")
else()
  execute_process(
    COMMAND ${CMAKE_Swift_COMPILER} --version
    OUTPUT_VARIABLE SWIFT_VERSION_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX MATCH "Swift version ([0-9]+\\.[0-9]+)" SWIFT_VERSION_TEXT
               "${SWIFT_VERSION_OUTPUT}")
  set(SWIFT_VERSION_NUMBER ${CMAKE_MATCH_1})
  message(STATUS "Found swift with version output '${SWIFT_VERSION_NUMBER}'")
  if("${SWIFT_VERSION_NUMBER}" VERSION_LESS 6.2)
    message(
      STATUS
        "Swift/C++ interop for this templated C++ code is known to work with Swift 6.2 only.  This build is using Swift ${SWIFT_VERSION_NUMBER}"
    )
  else()
    message(STATUS "Building swift code using Swift ${SWIFT_VERSION_NUMBER}")
    add_subdirectory(swift)
  endif()
endif()
