cmake_minimum_required(VERSION 3.15)
project(AdvCmpEx)

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
message("Build type for ${PROJECT_NAME}: ${CMAKE_BUILD_TYPE}")
#====================================================
# Set configuration types
#====================================================
if (NOT MSVC_IDE)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
else ()
  #target_compile_options cannot set parameters for all configurations in MSVC
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" CACHE STRING "" FORCE)
endif ()
message("Configurations for IDE: ${CMAKE_CONFIGURATION_TYPES}")

# check platform for build
set(IS_x64_PLATFORM FALSE CACHE BOOL "" FORCE)
if ("$ENV{PLATFORM}" MATCHES "x64")
  set(IS_x64_PLATFORM TRUE CACHE BOOL "" FORCE)
endif ()

include(cmake/utils.cmake)
plugin_extract_version(${CMAKE_CURRENT_LIST_DIR}/src/version.hpp)
if (IS_x64_PLATFORM)
  set(PACKAGE_FILE_NAME "${PROJECT_NAME}.${PLUGIN_VERSION}.x64")
else ()
  set(PACKAGE_FILE_NAME "${PROJECT_NAME}.${PLUGIN_VERSION}.x86")
endif ()

#====================================================
# build plugin
#====================================================
add_subdirectory(src)

#====================================================
# build resource files
#====================================================
# used @@ESCAPE and ESCAPE@@ for escape @ in files
configure_file(src/AdvCmpExRus.hlf.in src/AdvCmpExRus.hlf)

#====================================================
# install
#====================================================
install(TARGETS AdvCmpEx RUNTIME DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/src/${PROJECT_NAME}.map
    ${CMAKE_CURRENT_BINARY_DIR}/src/AdvCmpExRus.hlf
    src/AdvCmpExEng.lng
    src/AdvCmpExRus.lng
    ${PROJECT_SOURCE_DIR}/WhatsNewRus.txt
    ${PROJECT_SOURCE_DIR}/ReadMe.txt
    DESTINATION .)

#====================================================
# cpack
#====================================================
set(CPACK_GENERATOR "ZIP" CACHE STRING "Generators to support. semi-colon delimited list")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_FILE_NAME}")

include(CPack)

