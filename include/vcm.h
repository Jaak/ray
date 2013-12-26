#ifndef RAY_VCM_H
#define RAY_VCM_H

#include "area_light.h"
#include "geometry.h"
#include "point_light.h"
#include "primitive.h"
#include "primitive_manager.h"
#include "random.h"
#include "ray.h"
#include "scene.h"
#include "table.h"
#include "renderer.h"
#include "brdf.h"


  /*****************************
   * Bidirectional path tracer *
   *****************************/

class VCMRenderer : public Renderer {
private: /* Types: */

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
    };

    struct Vertex {
        Point      hitpoint;
        Colour     throughput;
        size_t     length;
        BRDF       brdf;

        floating   dVCM;
        floating   dVC;

        // path has previous hit point
        Vertex (Point hitpoint, const PathState& st, const BRDF& brdf)
            : hitpoint (hitpoint)
            , throughput (st.throughput)
            , length (st.length)
            , brdf (brdf)
            , dVCM(st.dVCM)
            , dVC(st.dVC)
        { }
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
    { }

    Colour render (Ray cameraRay) {
        const floating radius = 1e-03;
        const floating radiusSqr = radius * radius;
        const floating etaVCM = (M_PI * radiusSqr) / 1.0;
        m_misVmWeightFactor = 0.0;
        m_misVcWeightFactor = mis (1.0 / etaVCM);

        std::vector<Vertex> lightVertices;
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
            }

            // Don't store path vertices for purely specular surfaces.
            if (! lightBrdf.isDelta ()) {
                lightVertices.emplace_back (hitpoint, lightState, lightBrdf);
            }

            // Don't connect specular vertices to camera
            if (! lightBrdf.isDelta ()) {
                if (lightState.length + 1 >= MIN_PATH_LENGTH) {
                    connectToCamera (lightState, hitpoint, lightBrdf);
                }
            }

            // Stop if the path would become too long
            if (lightState.length + 2 > MAX_PATH_LENGTH)
                break;

            if (! sampleLightScattering (lightBrdf, hitpoint, lightState))
                break;
        }

        auto colour = Colour {0, 0, 0};

        PathState cameraState = generateCameraSample (cameraRay);
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
            const auto cameraBrdf = BRDF { ray, prim->normal (hitpoint), m };
            
            if (! cameraBrdf.isValid ())
                break;

            { // update MIS weights
                cameraState.dVCM *= mis (intr.dist () * intr.dist ());
                cameraState.dVCM /= mis (fabs (cameraBrdf.cosThetaFix ()));
                cameraState.dVC  /= mis (fabs (cameraBrdf.cosThetaFix ()));
            }

            if (prim->emissive ()) {
                if (cameraState.length >= MIN_PATH_LENGTH) {
                    const auto light = prim->getLight ();
                    colour += cameraState.throughput * getLightRadiance (light, cameraState, hitpoint, ray.dir ());
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
                for (const auto& lightVertex : lightVertices) {
                    const auto pathLength = lightVertex.length + 1 + cameraState.length;
                    if (pathLength < MIN_PATH_LENGTH)
                        continue;

                    if (pathLength > MAX_PATH_LENGTH)
                        break;

                    colour += cameraState.throughput * lightVertex.throughput *
                        connectVertices (lightVertex, cameraBrdf, hitpoint, cameraState);
                }
            }

            // Scatter the light
            if (! sampleEyeScattering (cameraBrdf, hitpoint, cameraState))
                break;
        }

        return colour;
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
        st.dVCM = mis (1.0 / cameraPdfW);
        st.dVC = 0.0;
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
    // TODO: why are we not multiplying with throughputs?
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

        const auto wLight = mis(cameraBrdfDirPdfA) * (lightVertex.dVCM + lightVertex.dVC * mis (lightBrdfRevPdfW));
        const auto wCamera = mis(lightBrdfDirPdfA) * (cameraState.dVCM + cameraState.dVC * mis (cameraBrdfRevPdfW));
        const auto misWeight = 1.0 / (wLight + 1.0 + wCamera);

        const auto contrib = misWeight * geometryTerm * camEv.colour * lightEv.colour;
        if (contrib.isZero () || occluded (hitpoint, direction, distance))
            return {0, 0, 0};

        return contrib;
    }
    
    // 
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
    void connectToCamera (const PathState& /*lightState*/, Point /*hitpoint*/, const BRDF& /*lightBrdf*/) const {
        // XXX TODO
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
        if (sample.colour.isZero ())
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
            state.dVC = mis(sample.cosTheta);
        }
        else {
            const auto dVCM = mis (1.0 / brdfDirPdfW);
            const auto dVC = mis (sample.cosTheta / brdfDirPdfW) *
                (state.dVC*mis(brdfRevPdfW) + state.dVCM + m_misVmWeightFactor);
            state.dVCM = dVCM;
            state.dVC = dVC;
        }

        state.hitpoint = hitpoint;
        state.direction = sample.direction;
        state.throughput = state.throughput * sample.colour * (sample.cosTheta / brdfDirPdfW);
        return true;
    }

    inline bool occluded (Point pos, Vector dir, floating dist) const {
        const auto ray = shootRay (pos, dir);
        const auto intr = intersectWithPrims (ray);
        // tolerance to avoid self intersections... 
        // TODO think if this is actually correct...
        const auto tolerance = 2*ray_epsilon;
        if (intr.hasIntersections () && tolerance < intr.dist() && intr.dist () < dist - tolerance)
            return true;

        return false;
    }

    static floating pdfWtoA (floating pdfW, floating dist, floating cosThere) {
        return pdfW * fabs(cosThere) / (dist * dist);
    }

private: /* Fields: */
    floating m_misVmWeightFactor;
    floating m_misVcWeightFactor;
};

#endif
