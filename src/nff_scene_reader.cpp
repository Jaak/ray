#include "nff_scene_reader.h"
#include "parser.h"
#include "scene.h"

SceneReader::Status NFFSceneReader::init(Scene& scene) const {
    int r = nff2scene(scene, fname().c_str());
    if (!r)
        return SceneReader::E_OTHER;
    return SceneReader::OK;
}
