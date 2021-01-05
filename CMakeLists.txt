cmake_minimum_required(VERSION 3.15)
project(AdvCmpEx CXX)

#====================================================
# Enable policy
#====================================================
# enable CMAKE_MSVC_RUNTIME_LIBRARY
cmake_policy(SET CMP0091 NEW)

#====================================================
# Set default build to release
#====================================================
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type, one of: Release, Debug" FORCE)
endif ()
message("Build type for AdvCmpEx: ${CMAKE_BUILD_TYPE}")

# check platform for build
set(IS_x64_PLATFORM FALSE CACHE BOOL "" FORCE)
if ("$ENV{PLATFORM}" MATCHES "x64")
  set(IS_x64_PLATFORM TRUE CACHE BOOL "" FORCE)
endif ()

# Get plugin version from src/version.hpp and put it in PLUGIN_VERSION
function(plugin_extract_version)
  file(READ "${CMAKE_CURRENT_LIST_DIR}/src/version.hpp" file_contents)
  string(REGEX MATCH "PLUGIN_VER_MAJOR ([0-9]+)" _ "${file_contents}")
  if (NOT CMAKE_MATCH_COUNT EQUAL 1)
    message(FATAL_ERROR "Could not extract major version number from version.hpp")
  endif ()
  set(ver_major ${CMAKE_MATCH_1})

  string(REGEX MATCH "PLUGIN_VER_MINOR ([0-9]+)" _ "${file_contents}")
  if (NOT CMAKE_MATCH_COUNT EQUAL 1)
    message(FATAL_ERROR "Could not extract minor version number from version.hpp")
  endif ()

  set(ver_minor ${CMAKE_MATCH_1})
  string(REGEX MATCH "PLUGIN_VER_PATCH ([0-9]+)" _ "${file_contents}")
  if (NOT CMAKE_MATCH_COUNT EQUAL 1)
    message(FATAL_ERROR "Could not extract patch version number from version.hpp")
  endif ()
  set(ver_patch ${CMAKE_MATCH_1})

  set(PLUGIN_VERSION "${ver_major}.${ver_minor}.${ver_patch}" PARENT_SCOPE)
endfunction()

plugin_extract_version()
add_subdirectory(src)
