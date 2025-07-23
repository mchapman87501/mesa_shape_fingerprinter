add_subdirectory(lib)
add_subdirectory(cli)

include(CheckLanguage)
check_language(Swift)

if(CMAKE_Swift_COMPILER STREQUAL "NOTFOUND")
  message(STATUS "Skipping build of ${CMAKE_CURRENT_SOURCE_DIR}/swift.")
else()
  add_subdirectory(swift)
endif()
