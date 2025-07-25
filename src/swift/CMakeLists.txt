cmake_minimum_required(VERSION 3.31)
set(PROJECT mesaac_swift_tools)

project(
  ${PROJECT}
  VERSION 1.0.0
  LANGUAGES C CXX Swift)

if("${CMAKE_Swift_COMPILER_VERSION}" VERSION_LESS 5.9)
  message(
    WARNING
      "Bidirectional C++ Interop requires Swift 5.9 or greater. Have ${CMAKE_Swift_COMPILER_VERSION}"
  )
  set(HAS_USABLE_SWIFT OFF)
else()
  set(HAS_USABLE_SWIFT ON)
endif()

if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
   AND NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  message(WARNING "Project requires building with Clang.
  Have ${CMAKE_CXX_COMPILER_ID}")
  set(HAS_COMPATIBLE_CXX OFF)
else()
  set(HAS_COMPATIBLE_CXX ON)
endif()

if(HAS_USABLE_SWIFT AND HAS_COMPATIBLE_CXX)
  include(initialize_swift)
  include(add_swift)

  add_subdirectory(MesaacSwift)

  # MolViewer depends on SwiftUI, which is mac operating system specific.
  if(APPLE)
    add_subdirectory(MolViewer)
  else()
    message(STATUS "MolViewer is for Apple platforms.  Skipping")
  endif()
endif()
