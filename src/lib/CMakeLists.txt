# Make Boost dynamic bitset available, from Github
set(BOOST_INCLUDE_LIBRARIES dynamic_bitset)
set(BOOST_ENABLE_CMAKE ON)
message(STATUS "Downloading and extracting boost library sources. This will take some time...")
include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE) # Needed to print downloading progress
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.tar.gz # downloading a zip release speeds up the download
    USES_TERMINAL_DOWNLOAD TRUE 
    GIT_PROGRESS TRUE   
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(Boost)

add_subdirectory(mesaac_mol)
add_subdirectory(mesaac_shape_eigen)
add_subdirectory(mesaac_shape)
