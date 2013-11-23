set(SOURCEFILES
    main.cpp
    camera.cpp
    intersection.cpp
    kdtree_primitive_manager.cpp
    naive_primitive_manager.cpp
    nff_scene_reader.cpp
    parser.cpp
    tga_surface.cpp
    ray.cpp
    scene.cpp
    sdl_surface.cpp
)

IF(GD_FOUND)
  list(APPEND SOURCEFILES png_surface.cpp)
ENDIF()

set_source_files_properties(${SOURCEFILES}
  PROPERTIES
  COMPILE_FLAGS "-std=c++11")

add_executable(ray ${SOURCEFILES})
target_link_libraries(ray
  ${Boost_LIBRARIES}
  ${SDL_LIBRARY}
  ${GD_LIBRARIES}
  )
