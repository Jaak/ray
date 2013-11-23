set(SOURCEFILES
    camera.cpp
    intersection.cpp
    kdtree_primitive_manager.cpp
    main.cpp
    naive_primitive_manager.cpp
    nff_scene_reader.cpp
    parser.cpp
    random.cpp
    ray.cpp
    scene.cpp
    sdl_surface.cpp
    tga_surface.cpp
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
