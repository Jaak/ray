cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

project ("ray" "CXX" "C")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Boost
find_package(Boost COMPONENTS thread date_time system program_options REQUIRED)
include_directories(${BOOST_INCLUDE_DIRS})
link_directories(${BOOST_LIBRARY_DIRS})
add_definitions(${BOOST_DEFINITIONS})

# GD
find_package(GD)
IF (GD_FOUND)
  add_definitions( -DHAVE_GD_SUPPORT_PNG )
  include_directories(${GD_INCLUDE_DIRS})
  link_directories(${GD_LIBRARY_DIRS})
  add_definitions(${GD_DEFINITIONS})
ENDIF()

include(config.local OPTIONAL)
include_directories(${ray_SOURCE_DIR}/include)
add_subdirectory (src)
