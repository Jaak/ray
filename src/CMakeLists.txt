set(SOURCEFILES
    main.cpp
    camera.cpp
    intersection.cpp
    kdtree_primitive_manager.cpp
    naive_primitive_manager.cpp
    nff_scene_reader.cpp
    parser.cpp
    png_surface.cpp
    ray.cpp
    scene.cpp
    sdl_surface.cpp
    )

add_executable(ray ${SOURCEFILES})
target_link_libraries(ray
  ${Boost_LIBRARIES}
  ${SDL_LIBRARY}
  ${GD_LIBRARIES}
  )
