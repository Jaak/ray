#ifndef RAY_VCM_H
#define RAY_VCM_H

#include "area_light.h"
#include "brdf.h"
#include "framebuffer.h"
#include "geometry.h"
#include "hashgrid.h"
#include "point_light.h"
#include "primitive.h"
#include "primitive_manager.h"
#include "random.h"
#include "ray.h"
#include "renderer.h"
#include "scene.h"
#include "table.h"


  /*****************************
   * Bidirectional path tracer *
   *****************************/

class VCMRenderer : public Renderer {
private: /* Types: */

    static constexpr floating INITIAL_RADIUS = 0.5;
    static constexpr floating RADIUS_ALPHA = 0.75;
    static constexpr size_t MIN_PATH_LENGTH = 0;
    static constexpr size_t MAX_PATH_LENGTH = 10;

    struct PathState {
        Point     hitpoint;
        Vector    direction;
        Colour    throughput;
        size_t    length;
        bool      isFinite;

        floating  dVCM;
        floating  dVC;
        floating  dVM;
    };

    struct Vertex {
        Point      hitpoint;
        Colour     throughput;
        size_t     length;
        BRDF       brdf;

        floating   dVCM;
        floating   dVC;
        floating   dVM;

        // path has previous hit point
        Vertex (Point hitpoint, const PathState& st, const BRDF& brdf)
            : hitpoint (hitpoint)
            , throughput (st.throughput)
            , length (st.length)
            , brdf (brdf)
            , dVCM (st.dVCM)
            , dVC (st.dVC)
            , dVM (st.dVM)
        { }

        Point position () const {
            return hitpoint;
        }
    };

private: /* Methods: */

  VCMRenderer& operator = (const VCMRenderer&) = delete;
  VCMRenderer(const VCMRenderer&) = delete;

  Intersection intersectWithPrims(const Ray& ray) const {
    return m_scene.manager().intersectWithPrims(ray);
  }

  static Ray shootRay(const Point& from, const Point& to) {
    return shootRay(from, to - from);
  }

  static Ray shootRay(const Point& from, Vector d) {
    d.normalise();
    return { from + d*ray_epsilon, d };
  }


public: /* Methods: */

    explicit VCMRenderer(const Scene& s)
      : Renderer { s }
      , m_misVmWeightFactor {0.0}
      , m_misVcWeightFactor {0.0}
      , m_lightSubpathCount {1.0}
    { }

    std::unique_ptr<Renderer> clone () const {
        return std::unique_ptr<Renderer> {new VCMRenderer {m_scene}};
    }

    void render (Framebuffer& buf, size_t iter) override {
        const auto& camera = scene ().camera ();
        m_lightSubpathCount = buf.width () * buf.height ();

        floating radius = INITIAL_RADIUS;
        radius = std::pow ((floating)(iter + 1), 0.5 * (1.0 - RADIUS_ALPHA));
        radius = std::max (radius, epsilon);

        const floating sqrRadius = radius * radius;
        const floating etaVCM = (M_PI * sqrRadius) * m_lightSubpathCount;
        m_misVmWeightFactor = mis (etaVCM);
        m_misVcWeightFactor = mis (1.0 / etaVCM);
        m_vmNormalization = 1.0 / etaVCM; // huh? not really sure about this

        m_lightVertices.clear ();
        m_lightPathBegins.clear ();
        m_lightPathBegins.push_back (0);

        // Generate all light paths:
        for (size_t x = 0; x < buf.width (); ++ x) {
            for (size_t y = 0; y < buf.height (); ++ y) {
                generateLightPath (buf);
            }
        }

        // Build hash grid of light paths:
        const auto numCells = buf.width () * buf.height ();
        const auto& hashGrid = HashGrid { m_lightVertices.begin (), m_lightVertices.end (), numCells, radius };

        // Generate all camera paths:
        uint32_t pathIdx = 0;
        for (size_t x = 0; x < buf.width (); ++ x) {
            for (size_t y = 0; y < buf.height (); ++ y) {
                const auto dx = rng ();
                const auto dy = rng ();
                const auto ray = camera.spawnRay (x + dx, y + dy);
                const auto col = generateCameraPath (hashGrid, buf, ray, pathIdx ++);
                buf.addColour (x, y, col);
            }
        }
    }

    // Generate a single light path
    void generateLightPath (Framebuffer& buf) {
        PathState lightState = generateLightSample ();
        for (;; ++ lightState.length) {
            const auto ray = shootRay (lightState.hitpoint, lightState.direction);
            const auto intr = intersectWithPrims (ray);
            if (! intr.hasIntersections ())
                break;

            const auto prim = intr.getPrimitive ();
            const Material& m = m_scene.materials ()[prim->material()];
            const auto hitpoint = intr.point ();
            const auto lightBrdf = BRDF { ray, prim->normal (hitpoint), m };

            if (! lightBrdf.isValid ())
                break;

            { // update MIS weights
                if (lightState.length > 1 || lightState.isFinite)
                    lightState.dVCM *= mis (intr.dist () * intr.dist ());

                lightState.dVCM /= mis (fabs (lightBrdf.cosThetaFix ()));
                lightState.dVC  /= mis (fabs (lightBrdf.cosThetaFix ()));
                lightState.dVM  /= mis (fabs (lightBrdf.cosThetaFix ()));
            }

            // Don't store path vertices for purely specular surfaces.
            if (! lightBrdf.isDelta ()) {
                m_lightVertices.emplace_back (hitpoint, lightState, lightBrdf);
            }

            // Don't connect specular vertices to camera
            if (! lightBrdf.isDelta ()) {
                if (lightState.length + 1 >= MIN_PATH_LENGTH) {
                    connectToCamera (buf, lightState, hitpoint, lightBrdf);
                }
            }

            // Stop if the path would become too long
            if (lightState.length + 2 > MAX_PATH_LENGTH)
                break;

            if (! sampleLightScattering (lightBrdf, hitpoint, lightState))
                break;
        }

        m_lightPathBegins.push_back (m_lightVertices.size ());
    }

    // render a single camera path
    Colour generateCameraPath (const HashGrid& hashGrid, Framebuffer& buf, Ray cameraRay, uint32_t pathIdx) {
        auto colour = Colour {0, 0, 0};

        PathState cameraState = generateCameraSample (cameraRay);
        // std::vector<Point> path;
        for (;; ++ cameraState.length) {
            const auto ray = shootRay (cameraState.hitpoint, cameraState.direction);
            const auto intr = intersectWithPrims (ray);
            if (! intr.hasIntersections ()) {
                /// TODO: consider background light
                break;
            }

            const auto prim = intr.getPrimitive ();
            const Material& m = m_scene.materials ()[prim->material()];
            const auto hitpoint = intr.point ();
            // path.push_back (hitpoint);
            const auto cameraBrdf = BRDF { ray, prim->normal (hitpoint), m };

            if (! cameraBrdf.isValid ())
                break;

            { // update MIS weights
                cameraState.dVCM *= mis (intr.dist () * intr.dist ());
                cameraState.dVCM /= mis (fabs (cameraBrdf.cosThetaFix ()));
                cameraState.dVC  /= mis (fabs (cameraBrdf.cosThetaFix ()));
                cameraState.dVM  /= mis (fabs (cameraBrdf.cosThetaFix ()));
            }

            if (prim->emissive ()) {
                if (cameraState.length >= MIN_PATH_LENGTH) {
                    const auto light = prim->getLight ();
                    const auto rad = getLightRadiance (light, cameraState, hitpoint, ray.dir ());
                    // if (rad.length () > 10) {
                    //   drawPath (buf, path);
                    // }

                    colour += cameraState.throughput * rad;
                }

                break;
            }

            if (cameraState.length >= MAX_PATH_LENGTH)
                break;

            // Connect to a light source
            if (! cameraBrdf.isDelta () && cameraState.length + 1 >= MIN_PATH_LENGTH) {
                colour += cameraState.throughput * directIllumination (cameraState, hitpoint, cameraBrdf);
            }

            // Connect to light vertices
            if (! cameraBrdf.isDelta ()) {
                const auto begin = m_lightPathBegins[pathIdx];
                const auto end = m_lightPathBegins[pathIdx + 1];
                for (uint32_t i = begin; i < end; ++ i) {
                    const auto& lightVertex = m_lightVertices[i];
                    const auto pathLength = lightVertex.length + 1 + cameraState.length;
                    if (pathLength < MIN_PATH_LENGTH)
                        continue;

                    if (pathLength > MAX_PATH_LENGTH)
                        break;

                    colour += cameraState.throughput * lightVertex.throughput *
                        connectVertices (lightVertex, cameraBrdf, hitpoint, cameraState);
                }
            }

            // Vertex merging:
            if (! cameraBrdf.isDelta ()) {
                auto col = Colour {0, 0, 0};
                const auto visitor =
                    [this, &col, &hitpoint, &cameraBrdf, &cameraState](const Vertex& lightVertex) -> void {
                        const auto pathLength = cameraState.length + lightVertex.length;
                        if (pathLength < MIN_PATH_LENGTH || pathLength > MAX_PATH_LENGTH)
                            return;

                        const auto lightDirection = lightVertex.brdf.worldDirFix ();
                        const auto camEv = cameraBrdf.evaluate (lightDirection);
                        if (camEv.colour.isZero ())
                            return;

                        const auto cameraBrdfDirPdfW = camEv.dirPdfW * cameraBrdf.continuationPr ();
                        const auto cameraBrdfRevPdfW = camEv.revPdfW * lightVertex.brdf.continuationPr ();
                        const auto wLight = lightVertex.dVCM * m_misVcWeightFactor + lightVertex.dVM * mis (cameraBrdfDirPdfW);
                        const auto wCamera = cameraState.dVCM * m_misVcWeightFactor + cameraState.dVM * mis (cameraBrdfRevPdfW);
                        const auto misWeight = 1.0 / (wLight + 1.0 + wCamera);
                        col += misWeight * camEv.colour * lightVertex.throughput;
                    };

                hashGrid.visit (m_lightVertices.begin (), hitpoint, visitor);
                colour += cameraState.throughput * m_vmNormalization * col;
            }

            // Scatter the light
            if (! sampleEyeScattering (cameraBrdf, hitpoint, cameraState))
                break;
        }

        return colour;
    }

    void drawPath (Framebuffer& buf, const std::vector<Point>& path) const {
      const auto& camera = m_scene.camera ();
      if (path.empty ())
        return;

      const Colour cols[7] = {
          {0, 0, 1}
        , {0, 1, 0}
        , {0, 1, 1}
        , {1, 0, 0}
        , {1, 0, 1}
        , {1, 1, 0}
        , {1, 1, 1}
      };

      floating prevX, prevY;
      camera.raster (path.front (), prevX, prevY);
      for (size_t i = 1; i < path.size (); ++ i) {
        floating x, y;
        camera.raster (path[i], x, y);
        buf.drawLine (prevX, prevY, x, y, 2 * cols[std::min (i-1, 6ul)]);
        prevX = x;
        prevY = y;
      }
    }

private:

    static inline floating mis (floating x) { return x; }

    Light* pickLight () const {
        floating acc = 0.0;
        for (const auto& l : m_scene.lights()) {
          acc += l->samplingPr();
          if (rng () <= acc) {
            return l.get();
          }
        }

        return nullptr;
    }

    PathState generateLightSample () const {
        Light* light = pickLight ();

        const auto e = light->emit ();
        const auto emissionPdfW = e.emissionPdfW * light->samplingPr ();
        const auto directPdfW = e.directPdfA * light->samplingPr ();

        PathState st;
        st.hitpoint = e.position;
        st.direction = e.direction;
        st.throughput = e.energy / emissionPdfW;
        st.length = 1;
        st.isFinite = light->isFinite ();
        st.dVCM = mis (directPdfW / emissionPdfW);
        st.dVC = light->isDelta () ? 0.0 : mis (e.cosTheta / emissionPdfW);
        st.dVM = st.dVC * m_misVcWeightFactor;
        return st;
    }

    PathState generateCameraSample (Ray ray) const {
        const auto& camera = m_scene.camera ();
        const auto cosAtCamera = camera.forward().dot(ray.dir());
        const auto imagePointToCameraDist = camera.imagePlaneDistance () / cosAtCamera;
        const auto imageToSolidAngleFactor = (imagePointToCameraDist*imagePointToCameraDist) / cosAtCamera;
        const auto cameraPdfW = imageToSolidAngleFactor;

        PathState st;
        st.hitpoint = ray.origin ();
        st.direction = ray.dir ();
        st.throughput = Colour {1, 1, 1};
        st.length = 1;
        st.isFinite = true;
        st.dVCM = mis (m_lightSubpathCount / cameraPdfW);
        st.dVC = 0.0;
        st.dVM = 0.0;
        return st;
    }

    Colour getLightRadiance (Light* light, const PathState& cameraState, Point hitpoint, Vector dir) const {
        const auto lightPickPr = light->samplingPr ();
        const auto r = light->radiance (hitpoint, dir);
        if (r.radiance.isZero ())
            return {0, 0, 0};

        if (cameraState.length == 1)
            return r.radiance;

        const auto directPdfA = r.directPdfA * lightPickPr;
        const auto emissionPdfW = r.emissionPdfW * lightPickPr;
        const auto wCamera = mis(directPdfA) * cameraState.dVCM + mis(emissionPdfW)*cameraState.dVC;
        const auto misWeight = 1.0 / (1.0 + wCamera);
        return misWeight * r.radiance;
    }

    // Connect eye and light vertex
    Colour connectVertices (const Vertex& lightVertex, const BRDF& cameraBrdf, Point hitpoint, const PathState& cameraState) const {
        auto direction = lightVertex.hitpoint - hitpoint;
        const auto sqrDist = direction.sqrlength();
        const auto distance = std::sqrt (sqrDist);
        direction /= distance;

        const auto camEv = cameraBrdf.evaluate (direction);
        if (camEv.colour.isZero ())
            return {0, 0, 0};
        const auto cameraCont = cameraBrdf.continuationPr ();
        const auto cameraBrdfDirPdfW = camEv.dirPdfW * cameraCont;
        const auto cameraBrdfRevPdfW = camEv.revPdfW * cameraCont;

        const auto lightEv = lightVertex.brdf.evaluate (- direction);
        if (lightEv.colour.isZero ())
            return {0, 0, 0};
        const auto lightCont = lightVertex.brdf.continuationPr ();
        const auto lightBrdfDirPdfW = lightEv.dirPdfW * lightCont;
        const auto lightBrdfRevPdfW = lightEv.revPdfW * lightCont;

        const auto geometryTerm = lightEv.cosTheta * camEv.cosTheta / sqrDist;
        if (geometryTerm <= 0.0)
            return {0, 0, 0};

        const auto cameraBrdfDirPdfA = pdfWtoA(cameraBrdfDirPdfW, distance, lightEv.cosTheta);
        const auto lightBrdfDirPdfA = pdfWtoA(lightBrdfDirPdfW, distance, camEv.cosTheta);

        const auto wLight = mis(cameraBrdfDirPdfA) * (m_misVcWeightFactor + lightVertex.dVCM + lightVertex.dVC * mis (lightBrdfRevPdfW));
        const auto wCamera = mis(lightBrdfDirPdfA) * (m_misVmWeightFactor + cameraState.dVCM + cameraState.dVC * mis (cameraBrdfRevPdfW));
        const auto misWeight = 1.0 / (wLight + 1.0 + wCamera);

        const auto contrib = misWeight * geometryTerm * camEv.colour * lightEv.colour;
        if (contrib.isZero () || occluded (hitpoint, direction, distance))
            return {0, 0, 0};

        return contrib;
    }

    Colour directIllumination (const PathState& cameraState, Point hitpoint, const BRDF& cameraBrdf) const {
        const auto light = pickLight ();
        const auto lightPickPr = light->samplingPr ();
        const auto i = light->illuminate (hitpoint);
        if (i.radiance.isZero ())
            return {0, 0, 0};

        const auto camEv = cameraBrdf.evaluate (i.direction);
        if (camEv.colour.isZero ())
            return {0, 0, 0};

        const auto contPr = cameraBrdf.continuationPr ();
        const auto brdfDirPdfW = light->isDelta () ? 0.0 : (camEv.dirPdfW * contPr);
        const auto brdfRevPdfW = camEv.revPdfW * contPr;
        const auto wLight = mis (brdfDirPdfW / (lightPickPr * i.directPdfW));
        const auto wCamera = mis (i.emissionPdfW * camEv.cosTheta / (i.directPdfW * i.cosTheta)) *
            (m_misVmWeightFactor + cameraState.dVCM + cameraState.dVC*mis(brdfRevPdfW));
        const auto misWeight = 1.0 / (wLight + 1.0 + wCamera);

        const auto contrib = (misWeight * camEv.cosTheta / (lightPickPr * i.directPdfW)) * (i.radiance * camEv.colour);
        if (contrib.isZero () || occluded (hitpoint, i.direction, i.distance))
            return {0, 0, 0};

        return contrib;
    }

    // Check if point randomly hits the camera.
    void connectToCamera (Framebuffer& buf, const PathState& lightState, Point hitpoint, const BRDF& lightBrdf) const {
        const auto& camera = m_scene.camera ();
        floating x, y;
        if (! camera.raster (hitpoint, x, y))
            return;

        auto directionToCamera = camera.eye () - hitpoint;
        const auto sqrDist = directionToCamera.sqrlength ();
        const auto distance = std::sqrt (sqrDist);
        directionToCamera /= distance;

        const auto lightEv = lightBrdf.evaluate (directionToCamera);
        if (lightEv.colour.isZero ())
            return;

        const auto brdfRevPdfW = lightEv.revPdfW * lightBrdf.continuationPr ();
        const auto cosAtCamera = camera.forward ().dot (- directionToCamera);
        const auto cosToCamera = lightEv.cosTheta;
        const auto imagePointToCameraDist = camera.imagePlaneDistance () / cosAtCamera;
        const auto imageToSolidAngleFactor = (imagePointToCameraDist * imagePointToCameraDist) / cosAtCamera;
        const auto imageToSurfaceFactor = imageToSolidAngleFactor * fabs(cosToCamera) / sqrDist;

        const auto cameraPdfA = imageToSurfaceFactor;

        const auto wLight = mis (cameraPdfA / m_lightSubpathCount) * (m_misVmWeightFactor + lightState.dVCM + lightState.dVC * mis (brdfRevPdfW));
        const auto misWeight = 1.0 / (wLight + 1.0);
        const auto surfaceToImageFactor = 1.0 / imageToSurfaceFactor;
        const auto contrib = misWeight * lightState.throughput * lightEv.colour / (m_lightSubpathCount * surfaceToImageFactor);

        if (contrib.isZero () || occluded (hitpoint, directionToCamera, distance))
            return;

        buf.addColour (x, y, contrib);
    }

    bool sampleLightScattering (const BRDF& lightBrdf, Point hitpoint, PathState& lightState) const {
        return sampleScattering<true>(lightBrdf, hitpoint, lightState);
    }

    bool sampleEyeScattering (const BRDF& cameraBrdf, Point hitpoint, PathState& cameraState) const {
        return sampleScattering<false>(cameraBrdf, hitpoint, cameraState);
    }

    template <bool LightTracing>
    bool sampleScattering (const BRDF& brdf, Point hitpoint, PathState& state) const {
        const auto sample = brdf.sample<LightTracing>();
        if (sample.event == BRDF::NONE || sample.colour.isZero ())
            return false;

        const auto contPr = brdf.continuationPr ();
        if (rng () > contPr)
            return false;

        const auto isSpecularEvent =
            sample.event == BRDF::REFLECT ||
            sample.event == BRDF::REFRACT; /// TODO not pretty
        const auto brdfDirPdfW = sample.dirPdfW * contPr;
        const auto brdfRevPdfW = (isSpecularEvent ? sample.dirPdfW : brdf.pdf(state.direction).revPdfW) * contPr;

        if (isSpecularEvent) {
            state.dVCM = 0.0;
            state.dVC *= mis(sample.cosTheta);
            state.dVM *= mis(sample.cosTheta);
        }
        else {
            const auto dVCM = mis (1.0 / brdfDirPdfW);
            const auto dVC = mis (sample.cosTheta / brdfDirPdfW) *
                (state.dVC*mis(brdfRevPdfW) + state.dVCM + m_misVmWeightFactor);
            const auto dVM = mis (sample.cosTheta / brdfRevPdfW) *
                (state.dVM*mis(brdfRevPdfW) + state.dVCM * m_misVcWeightFactor + 1.0);
            state.dVCM = dVCM;
            state.dVC  = dVC;
            state.dVM  = dVM;
        }

        state.hitpoint = hitpoint;
        state.direction = sample.direction;
        state.throughput *= sample.colour * (sample.cosTheta / brdfDirPdfW);
        return true;
    }

    inline bool occluded (Point pos, Vector dir, floating dist) const {
        const auto ray = shootRay (pos, dir);
        const auto intr = intersectWithPrims (ray);
        // tolerance to avoid self intersections...
        // TODO think if this is actually correct...
        const auto tolerance = 2*ray_epsilon;
        if (intr.hasIntersections () && 0 <= intr.dist() && intr.dist () < dist - tolerance)
            return true;

        return false;
    }

    static floating pdfWtoA (floating pdfW, floating dist, floating cosThere) {
        return pdfW * fabs(cosThere) / (dist * dist);
    }

private: /* Fields: */
    std::vector<Vertex>   m_lightVertices;
    std::vector<uint32_t> m_lightPathBegins;
    floating              m_misVmWeightFactor;
    floating              m_misVcWeightFactor;
    floating              m_lightSubpathCount;
    floating              m_vmNormalization;
};

#endif
