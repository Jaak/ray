set(SOURCEFILES
    camera.cpp
    framebuffer.cpp
    geometry.cpp
    kdtree_primitive_manager.cpp
    main.cpp
    naive_primitive_manager.cpp
    nff_scene_reader.cpp
    parser.cpp
    random.cpp
    ray.cpp
    renderer.cpp
    scene.cpp
    tga_surface.cpp
)

set(RAY_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/")

set(INCLUDEFILES
  "${RAY_INCLUDE_DIR}/aabb.h"
  "${RAY_INCLUDE_DIR}/area_light.h"
  "${RAY_INCLUDE_DIR}/background_light.h"
  "${RAY_INCLUDE_DIR}/brdf.h"
  "${RAY_INCLUDE_DIR}/camera.h"
  "${RAY_INCLUDE_DIR}/common.h"
  "${RAY_INCLUDE_DIR}/directional_light.h"
  "${RAY_INCLUDE_DIR}/frame.h"
  "${RAY_INCLUDE_DIR}/geometry.h"
  "${RAY_INCLUDE_DIR}/hashgrid.h"
  "${RAY_INCLUDE_DIR}/intersection.h"
  "${RAY_INCLUDE_DIR}/kdtree_primitive_manager.h"
  "${RAY_INCLUDE_DIR}/light.h"
  "${RAY_INCLUDE_DIR}/material.h"
  "${RAY_INCLUDE_DIR}/materials.h"
  "${RAY_INCLUDE_DIR}/naive_primitive_manager.h"
  "${RAY_INCLUDE_DIR}/nff_scene_reader.h"
  "${RAY_INCLUDE_DIR}/parser.h"
  "${RAY_INCLUDE_DIR}/pathtracer.h"
  "${RAY_INCLUDE_DIR}/png_surface.h"
  "${RAY_INCLUDE_DIR}/point_light.h"
  "${RAY_INCLUDE_DIR}/primitive.h"
  "${RAY_INCLUDE_DIR}/primitive_manager.h"
  "${RAY_INCLUDE_DIR}/random.h"
  "${RAY_INCLUDE_DIR}/ray.h"
  "${RAY_INCLUDE_DIR}/raytracer.h"
  "${RAY_INCLUDE_DIR}/rectangle.h"
  "${RAY_INCLUDE_DIR}/renderer.h"
  "${RAY_INCLUDE_DIR}/scene.h"
  "${RAY_INCLUDE_DIR}/scene_reader.h"
  "${RAY_INCLUDE_DIR}/sphere.h"
  "${RAY_INCLUDE_DIR}/surface.h"
  "${RAY_INCLUDE_DIR}/table.h"
  "${RAY_INCLUDE_DIR}/texture.h"
  "${RAY_INCLUDE_DIR}/tga_surface.h"
  "${RAY_INCLUDE_DIR}/triangle.h"
  "${RAY_INCLUDE_DIR}/vcm.h"
)

IF(GD_FOUND)
  list(APPEND SOURCEFILES png_surface.cpp)
ENDIF()

set_source_files_properties(${SOURCEFILES}
  PROPERTIES
  COMPILE_FLAGS "-std=c++11"
)

add_executable(ray
  ${SOURCEFILES}
  ${INCLUDEFILES}
)

include_directories(ray
  "${RAY_INCLUDE_DIR}"
  ${Boost_INCLUDE_DIRS}
  ${GD_INCLUDE_DIRS}
)

target_link_libraries(ray
  ${Boost_LIBRARIES}
  ${GD_LIBRARIES}
)
