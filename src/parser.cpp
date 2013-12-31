#include "area_light.h"
#include "background_light.h"
#include "common.h"
#include "directional_light.h"
#include "geometry.h"
#include "itriangle.h"
#include "material.h"
#include "point_light.h"
#include "scene.h"
#include "sphere.h"
#include "spotlight.h"
#include "texture.h"
#include "tga_reader.h"
#include "triangle.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>


namespace /* anonymous */ {

material_index_t current_material;
texture_index_t current_texture = -1;

typedef double COORD3[3];
#define SET_COORD3(r,A,B,C)     { (r)[0] = (A); (r)[1] = (B); (r)[2] = (C); }

typedef double COORD2[2];
#define SET_COORD2(r,A,B)     { (r)[0] = (A); (r)[1] = (B); }

void
show_error(const char* s) {
	fprintf(stderr, "%s\n", s);
}

void do_comment(FILE* fp) {
  char* cp;
  char comment[256];

  if (fgets(comment, 255, fp) == NULL) {
    return;
  }

  /* strip out newline */
  cp = (char*)strchr(comment, '\n');
  if (cp != NULL)
    *cp = '\0';
}

void do_view(Scene& scene, FILE* fp) {
  float x = 0, y = 0, z = 0;
  float fov_angle = 45.0;
  float hither = 0.001;
  int resx = 800;
  int resy = 600;

  bool err = false;

  err = err || (fscanf(fp, " from %f %f %f", &x, &y, &z) != 3);
  const auto from = Point {x, y, z};

  err = err || (fscanf(fp, " at %f %f %f", &x, &y, &z) != 3);
  const auto at = Point {x, y, z};

  err = err || (fscanf(fp, " up %f %f %f", &x, &y, &z) != 3);
  const auto up = Vector {x, y, z};

  err = err || (fscanf(fp, " angle %f", &fov_angle) != 1);

  fscanf(fp, " hither %f", &hither);

  fscanf(fp, " resolution %d %d", &resx, &resy);

  if (! err) {
      scene.camera ().setup (from, at, up, fov_angle, hither, resx, resy);
      for (auto& surface : scene.surfaces()) {
        surface->setDimensions(resy, resx);
      }
  }
  else {
      show_error("NFF view syntax error");
      exit(1);
  }
}

void do_regular_light(Scene &scene, FILE *fp) {
    float x, y, z, r = 1.0, g = 1.0, b = 1.0;
    const auto n = fscanf(fp, "%f %f %f %f %f %f", &x, &y, &z, &r, &g, &b);
    if (n != 3 && n != 6) {
      show_error("Light source syntax error");
      exit(1);
    }

    const auto intensity = Colour{r, g, b};
    const auto pos = Point{x, y, z};

    Light* light = new PointLight{scene.sceneSphere(), intensity, pos};
    scene.addLight(light);
}

void do_sphere_light(Scene &scene, FILE *fp) {
    float R, x, y, z, r = 1.0, g = 1.0, b = 1.0;
    const auto n = fscanf(fp, "%f %f %f %f %f %f %f", &R, &x, &y, &z, &r, &g, &b);
    if (n != 4 && n != 7) {
      show_error("Spherical area light source syntax error");
      exit(1);
    }

    const auto intensity = Colour{r, g, b};
    const auto pos = Point{x, y, z};
    const floating rad = R;

    Light* light = new SphereLight{scene.sceneSphere(), intensity,  pos, rad};
    Primitive* prim = new Sphere{pos, rad};
    prim->setMaterial(Materials::lightMaterial ());
    prim->setLight(light);
    scene.addPrimitive(prim);
    scene.addLight(light);
}

void do_area_light (Scene& scene, FILE* fp) {
    float x, y, z, ux, uy, uz, vx, vy, vz, r = 1.0, g = 1.0, b = 1.0;
    const auto n = fscanf(fp, "%f %f %f %f %f %f %f %f %f %f %f %f",
      &x, &y, &z, &ux, &uy, &uz, &vx, &vy, &vz, &r, &g, &b);
    if (n != 9 && n != 12) {
        show_error ("Rectangular area light source syntax error");
        exit (1);
    }

    const auto intensity = Colour {r, g, b};
    const auto pos = Point {x, y, z};
    const auto u = Vector {ux, uy, uz};
    const auto v = Vector {vx, vy, vz};

    Light* light = new AreaLight {scene.sceneSphere(), intensity, pos, u, v};
    Primitive* prim = new Rectangle {pos, u, v};
    prim->setMaterial(Materials::lightMaterial ());
    prim->setLight(light);
    scene.addPrimitive(prim);
    scene.addLight(light);
}

void do_directional_light (Scene& scene, FILE* fp) {
    float x, y, z, r = 1.0, g = 1.0, b = 1.0;
    const auto n = fscanf(fp, "%f %f %f %f %f %f", &x, &y, &z, &r, &g, &b);
    if (n != 6 && n != 3) {
        show_error ("Directional light source syntax error");
        exit (1);
    }

    const auto intensity = Colour {r, g, b};
    const auto dir = Vector {x, y, z};
    Light* light = new DirectionalLight {scene.sceneSphere (), intensity, dir};
    scene.addLight (light);
}

void do_spotlight (Scene& scene, FILE* fp) {
    float x, y, z, dx, dy, dz, a, r = 1.0, g = 1.0, b = 1.0;
    const auto n = fscanf(fp, "%f %f %f %f %f %f %f %f %f %f",
      &x, &y, &z, &dx, &dy, &dz, &a, &r, &g, &b);
    if (n != 10 && n != 7) {
        show_error ("Spotlight source syntax error");
        exit (1);
    }

    const auto intensity = Colour {r, g, b};
    const auto pos = Point {x, y, z};
    const auto dir = Vector {dx, dy, dz};
    const floating alpha = a * M_PI / 180.0;
    Light* light = new Spotlight {scene.sceneSphere(), intensity, pos, dir, alpha};
    scene.addLight (light);
}

void do_light(Scene& scene, FILE* fp) {

  const auto c = getc(fp);

  if (c == 's') {
    do_sphere_light (scene, fp);
    return;
  }

  if (c == 'a') {
    do_area_light (scene, fp);
    return;
  }

  // since 's' is taken let's use 'f' for flashlight as an alternative name for spotlight
  if (c == 'f') {
    do_spotlight (scene, fp);
    return;
  }

  if (c == 'd') {
    do_directional_light (scene, fp);
    return;
  }

  ungetc(c, fp);
  do_regular_light (scene, fp);
}

void do_background(Scene& scene, FILE* fp) {
  float r = 0, g = 0, b = 0;
  if (fscanf(fp, "%f %f %f", &r, &g, &b) != 3) {
    show_error("background color syntax error");
    exit(1);
  }

  const auto col = Colour {r, g, b};
  scene.setBackground (col);
  if (r > 0 || g > 0 || b > 0) {
      Light* light = new BackgroundLight {scene.sceneSphere (), col};
      scene.addLight (light);
      scene.setBackgroundLight (light);
  }
}

void do_fill(Scene& scene, FILE* fp) {
  float r, g, b, kd, ks, ks_spec, phong_pow, ang, t, ior;

  if (fscanf(fp, "%f %f %f", &r, &g, &b) != 3) {
    show_error("fill color syntax error");
    exit(1);
  }

  if (fscanf(fp, "%f %f %f %f %f", &kd, &ks, &phong_pow, &t, &ior) != 5) {
    show_error("fill material syntax error");
    exit(1);
  }

  ks_spec = ks;
  /* convert phong_pow back into phong hilight angle. */
  /* reciprocal of formula in libpr1.c, lib_output_color() */
  if (phong_pow < 1.0)
    phong_pow = 1.0;
  ang = (float)((180.0 / M_PI) * acos(exp(log(0.5) / phong_pow)));

  current_material = scene.materials().registerMaterial(
      Material(Colour(r, g, b), kd, ks, t, ior, phong_pow));
}

// TODO: support for cones
void do_cone(Scene&, FILE* fp) {
  float x0, y0, z0, x1, y1, z1, r0, r1;

  if (fscanf(fp, " %f %f %f %f %f %f %f %f", &x0, &y0, &z0, &r0, &x1, &y1, &z1,
             &r1) !=
      8) {
    show_error("cylinder or cone syntax error");
    exit(1);
  }
  if (r0 < 0.0) {
    r0 = -r0;
    r1 = -r1;
  }
}

void do_sphere(Scene& scene, FILE* fp) {
  float x, y, z, r;
  int textured;

  textured = getc(fp);
  if (textured != 't') {
    ungetc(textured, fp);
    textured = 0;
  }

  if (fscanf(fp, "%f %f %f %f", &x, &y, &z, &r) != 4) {
    show_error("sphere syntax error");
    exit(1);
  }

  Primitive* p = new Sphere(Point(x, y, z), r);
  p->setMaterial(current_material);
  if (textured)
    p->setTexture(current_texture);
  scene.addPrimitive(p);
}

void do_poly(Scene& scene, FILE* fp) {
  int ispatch;
  int textured;
  int nverts;
  int vertcount;
  COORD3* norms;
  COORD3* verts;
  COORD2* uvs;
  float x, y, z;

  norms = verts = NULL;
  uvs = NULL;

  ispatch = getc(fp);
  if (ispatch != 'p') {
    ungetc(ispatch, fp);
    ispatch = 0;
  }
  textured = getc(fp);
  if (textured != 't') {
    ungetc(textured, fp);
    textured = 0;
  }

  if (fscanf(fp, "%d", &nverts) != 1)
    goto fmterr;

  verts = (COORD3*)malloc(nverts * sizeof(COORD3));
  if (verts == NULL)
    goto memerr;

  if (ispatch) {
    norms = (COORD3*)malloc(nverts * sizeof(COORD3));
    if (norms == NULL)
      goto memerr;
  }

  if (textured) {
    uvs = (COORD2*)malloc(nverts * sizeof(COORD2));
    if (uvs == NULL)
      goto memerr;
  }

  /* read all the vertices into temp array */
  for (vertcount = 0; vertcount < nverts; vertcount++) {
    if (fscanf(fp, " %f %f %f", &x, &y, &z) != 3)
      goto fmterr;
    SET_COORD3(verts[vertcount], x, y, z);

    if (ispatch) {
      if (fscanf(fp, " %f %f %f", &x, &y, &z) != 3)
        goto fmterr;
      SET_COORD3(norms[vertcount], x, y, z);
    }

    if (textured) {
      if (fscanf(fp, " %f %f", &x, &y) != 2)
        goto fmterr;
      SET_COORD2(uvs[vertcount], x, y);
    }
  }

  /* write output */
  /*    if (ispatch)
    lib_output_polypatch(nverts, verts, norms);
      else
    lib_output_polygon(nverts, verts);
  */
  /* XXX Naíve */
  for (int i = 1; i < nverts - 1; ++i) {
    Primitive* p;
    const auto p0 = 0;
    const auto p1 = i;
    const auto p2 = i + 1;

    const auto point0 = Point {verts[p0][0], verts[p0][1], verts[p0][2]};
    const auto point1 = Point {verts[p1][0], verts[p1][1], verts[p1][2]};
    const auto point2 = Point {verts[p2][0], verts[p2][1], verts[p2][2]};

    if (ispatch && textured) {
        const auto n0 = Vector {norms[p0][0], norms[p0][1], norms[p0][2]};
        const auto n1 = Vector {norms[p1][0], norms[p1][1], norms[p1][2]};
        const auto n2 = Vector {norms[p2][0], norms[p2][1], norms[p2][2]};
        const auto v0 = make_vertex (point0, n0, uvs[p0][0], uvs[p0][1]);
        const auto v1 = make_vertex (point1, n1, uvs[p1][0], uvs[p1][1]);
        const auto v2 = make_vertex (point2, n2, uvs[p2][0], uvs[p2][1]);
        p = make_triangle (v0, v1, v2);
    }
    else if (ispatch) {
        const auto n0 = Vector {norms[p0][0], norms[p0][1], norms[p0][2]};
        const auto n1 = Vector {norms[p1][0], norms[p1][1], norms[p1][2]};
        const auto n2 = Vector {norms[p2][0], norms[p2][1], norms[p2][2]};
        const auto v0 = make_vertex (point0, n0);
        const auto v1 = make_vertex (point1, n1);
        const auto v2 = make_vertex (point2, n2);
        p = make_triangle (v0, v1, v2);
    }
    else if (textured) {
        const auto v0 = make_vertex (point0, uvs[p0][0], uvs[p0][1]);
        const auto v1 = make_vertex (point1, uvs[p1][0], uvs[p1][1]);
        const auto v2 = make_vertex (point2, uvs[p2][0], uvs[p2][1]);
        p = make_triangle (v0, v1, v2);
    }
    else {
        const auto v0 = make_vertex (point0);
        const auto v1 = make_vertex (point1);
        const auto v2 = make_vertex (point2);
        p = make_triangle (v0, v1, v2);
    }

    p->setMaterial(current_material);
    if (textured) {
      if (current_texture < 0)
        goto textureError;
      p->setTexture(current_texture);
    }

    scene.addPrimitive(p);
  }

  free(verts);
  if (ispatch)
    free(norms);
  if (textured)
    free(uvs);

  return;
fmterr:
  show_error("polygon or patch syntax error");
  exit(1);
memerr:
  show_error("can't allocate memory for polygon or patch");
  exit(1);
textureError:
  show_error("texture must be defined before trying to use one");
  exit(1);
}

void do_texture(Scene& scene, FILE* fp) {
  char tf[511];

  if (fscanf(fp, " %s", tf) != 1) {
    show_error("texture syntax error");
    exit(1);
  }
  // assume texture file is in the same location as the nff file that references it
  std::string file_location = scene.getFname();
  file_location.erase(scene.getFname().find_last_of('/') + 1, std::string::npos);
  std::string texture_file = file_location + std::string(tf);

  current_texture = scene.textures().registerTexture(tga_reader::readTexture(texture_file));
}

void
parse_nff(Scene& scene, FILE* fp)
{
  int c;
  while ((c = getc (fp)) != EOF ) {
    switch (c) {
      case ' ':            /* white space */
      case '\t':
      case '\n':
      case '\f':
      case '\r':
        continue;
      case '#': do_comment(fp); break;
      case 'v': do_view(scene, fp); break;
      case 'l': do_light(scene, fp); break;
      case 'b': do_background(scene, fp); break;
      case 'f': do_fill(scene, fp); break;
      case 'c': do_cone(scene, fp); break;
      case 's': do_sphere(scene, fp); break;
      case 'p': do_poly(scene, fp); break;
      case 't': do_texture(scene, fp); break;
      default:            /* unknown */
                show_error("unknown NFF primitive code: ");
                exit(1);
    }
  }
} /* parse_nff */

} /* namespace anonymous */

bool nff2scene(Scene& scene, char const* fname) {
  FILE* fp;

  if ((fp = fopen(fname, "r")) == NULL) {
    return false;
  }

  parse_nff(scene, fp);

  fclose(fp);

  return true;
}
