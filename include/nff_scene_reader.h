#pragma once

#include "scene_reader.h"

class NFFSceneReader : public SceneReader {
public: /* Methods: */
    NFFSceneReader(char const *fname) : SceneReader(fname) {}

    ~NFFSceneReader() {}

    SceneReader::Status init(Scene &scene) const;
};
