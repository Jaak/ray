cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

project (ray)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Opengl
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
link_directories(${OPENGL_LIBRARY_DIRS})
add_definitions(${OPENGL_DEFINITIONS})

# SDL
find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIRS})
link_directories(${SDL_LIBRARY_DIRS})
add_definitions(${SDL_DEFINITIONS})

# Boost
find_package(Boost COMPONENTS thread date_time REQUIRED)
include_directories(${BOOST_INCLUDE_DIRS})
link_directories(${BOOST_LIBRARY_DIRS})
add_definitions(${BOOST_DEFINITIONS})

# GD
find_package(GD REQUIRED)
include_directories(${GD_INCLUDE_DIRS})
link_directories(${GD_LIBRARY_DIRS})
add_definitions(${GD_DEFINITIONS})

if(EXISTS config.local)
  include(config.local)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11 -O ${CMAKE_CXX_FLAGS}")
include_directories(${ray_SOURCE_DIR}/include)
add_subdirectory (src)
