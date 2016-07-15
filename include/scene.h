#pragma once

#include "camera.h"
#include "common.h"
#include "geometry.h"
#include "light.h"
#include "material.h"
#include "materials.h"
#include "surface.h"
#include "texture.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

class BackgroundLight;
class Block;
class Light;
class Pixel;
class Primitive;
class PrimitiveManager;
class Renderer;
class SceneReader;

// TODO: replace background material with background light.

/// Representation of scene.
class Scene {
    friend class Block;

public: /* Methods: */
    Scene();

    ~Scene();

    /**
     * Does the real raytracing.
     * Image is generated.
     */
    void run();

    /**
     * Initialises scene for raytracing.
     * Complete destruction of our spacetime will ensure
     * if this method is not called before raytracing begins.
     *
     * @todo Throw logic exceptions from here.
     */
    void init();

    /**
     * Sets scenes primitive manager.
     * Given PrimitiveManager will be deallocated after
     * Scene object is destroyed.
     */
    void setPrimitiveManager(PrimitiveManager* pm);

    void setRenderer(Renderer* r);

    void setSamples(size_t n) { m_samples = n; }

    void setSceneReader(SceneReader* sr);

    void addPrimitive(const Primitive* prim);
    void addLight(Light* light);
    const std::vector<std::unique_ptr<Light>>& lights() const;
    void attachSurface(Surface* surface) { m_surfaces.emplace_back(surface); }
    const std::vector<std::unique_ptr<Surface>>& surfaces() const {
        return m_surfaces;
    }
    PrimitiveManager& manager() const { return *m_manager; }
    const Materials& materials() const { return m_materials; }
    Materials& materials() { return m_materials; }
    Camera& camera() { return m_camera; }
    const Camera& camera() const { return m_camera; }
    Renderer& renderer() { return *m_renderer; }
    const Renderer& renderer() const { return *m_renderer; }
    void setBackground(const Colour& c);
    const Material& background() const;
    std::string getFname();
    const Textures& textures() const { return m_textures; }
    Textures& textures() { return m_textures; }

    void setBackgroundLight(Light* light);
    Light* backgroundLight() const;

    const SceneSphere& sceneSphere() const { return m_sceneSphere; }
    void setSceneSphere(Point center, floating radius) {
        m_sceneSphere.setCenter(center);
        m_sceneSphere.setRadius(radius);
    }

private:
    void updatePixel(size_t x, size_t y, Colour c) const;

protected: /* Fields: */
    SceneSphere                           m_sceneSphere;
    Camera                                m_camera;
    Materials                             m_materials;
    std::unique_ptr<PrimitiveManager>     m_manager;
    std::unique_ptr<SceneReader>          m_scene_reader;
    std::vector<std::unique_ptr<Light>>   m_lights;
    Light*                                m_backgroundLight;
    material_index_t                      m_background;
    std::vector<std::unique_ptr<Surface>> m_surfaces;
    Textures                              m_textures;
    std::unique_ptr<Renderer>             m_renderer;
    size_t                                m_samples;
};
