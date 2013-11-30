#include "area_light.h"
#include "common.h"
#include "geometry.h"
#include "itriangle.h"
#include "material.h"
#include "point_light.h"
#include "scene.h"
#include "sphere.h"
#include "triangle.h"
#include "tga_reader.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>


namespace /* anonymous */ {

material_index_t current_material;

typedef double COORD3[3];
#define SET_COORD3(r,A,B,C)     { (r)[0] = (A); (r)[1] = (B); (r)[2] = (C); }

void
show_error(const char* s) {
	fprintf(stderr, "%s\n", s);
}

bool is_whitespace (int c) {
    switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case '\f':
    case '\r':
        return true;
    }

    return false;
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
  Camera& cam = scene.camera();
  float x, y, z;
  float fov_angle;
  float aspect_ratio;
  float hither;
  int resx;
  int resy;

  if (fscanf(fp, " from %f %f %f", &x, &y, &z) != 3)
    goto fmterr;

  cam.setEye(Point(x, y, z));

  if (fscanf(fp, " at %f %f %f", &x, &y, &z) != 3)
    goto fmterr;

  cam.setAt(Point(x, y, z));

  if (fscanf(fp, " up %f %f %f", &x, &y, &z) != 3)
    goto fmterr;

  cam.setUp(Vector(x, y, z));

  if (fscanf(fp, " angle %f", &fov_angle) != 1)
    goto fmterr;

  cam.setFOV(fov_angle);

  fscanf(fp, " hither %f", &hither);

  cam.setHither(hither);

  aspect_ratio = (float) 1.0;

  fscanf(fp, " resolution %d %d", &resx, &resy);

  cam.setDimensions(resy, resx);
  for (auto& surface : scene.surfaces()) {
    surface->setDimensions(resy, resx);
  }

  return;
fmterr:
  show_error("NFF view syntax error");
  exit(1);
}

void do_light(Scene& scene, FILE* fp) {
  float x, y, z, r = 1.0, g = 1.0, b = 1.0;
  const auto n = fscanf(fp, "%f %f %f %f %f %f", &x, &y, &z, &r, &g, &b);
  if (n != 3 && n != 6) {
    show_error("Light source syntax error");
    exit(1);
  }

  auto light = new PointLight(Point(x, y, z), Colour(r, g, b));
  scene.addLight(light);
}

// TODO: more area lights
void do_area_light(Scene& scene, FILE* fp) {
  int c;
  while ((c = getc (fp)) != EOF ) {
      if (! is_whitespace (c))
          break;
  }

  // Sphere area light
  if (c == 's') {
      float R, x, y, z, r = 1.0, g = 1.0, b = 1.0;
      const auto n = fscanf(fp, "%f %f %f %f %f %f %f", &R, &x, &y, &z, &r, &g, &b);
      if (n != 4 && n != 7) {
          show_error ("Spherical area light source syntax error");
          exit (1);
      }

      auto light = new SphereLight { Point { x, y, z}, R, Colour { r, g, b } };
      scene.addLight (light);
  }
  else {
      show_error ("Area light syntax error");
      exit (1);
  }
}

void do_background(Scene& scene, FILE* fp) {
  float r, g, b;
  if (fscanf(fp, "%f %f %f", &r, &g, &b) != 3) {
    show_error("background color syntax error");
    exit(1);
  }

  scene.setBackground(Colour(r, g, b));
}

void do_fill(Scene& scene, FILE* fp) {
  float r, g, b, ka, kd, ks, ks_spec, phong_pow, ang, t, ior;

  if (fscanf(fp, "%f %f %f", &r, &g, &b) != 3) {
    show_error("fill color syntax error");
    exit(1);
  }

  if (fscanf(fp, "%f %f %f %f %f", &kd, &ks, &phong_pow, &t, &ior) != 5) {
    show_error("fill material syntax error");
    exit(1);
  }

  /* some parms not input in NFF, so hard-coded. */
  ka = (float) 0.1;
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

  if (fscanf(fp, "%f %f %f %f", &x, &y, &z, &r) != 4) {
    show_error("sphere syntax error");
    exit(1);
  }

  Primitive* p = new Sphere(Point(x, y, z), r);
  p->setMaterial(current_material);
  scene.addPrimitive(p);
}

void do_poly(Scene& scene, FILE* fp) {
  int ispatch;
  int nverts;
  int vertcount;
  COORD3* norms;
  COORD3* verts;
  float x, y, z;

  norms = verts = NULL;

  ispatch = getc(fp);
  if (ispatch != 'p') {
    ungetc(ispatch, fp);
    ispatch = 0;
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

    if (ispatch) {
      p = new ITriangle(
          Point(verts[0][0], verts[0][1], verts[0][2]),
          Point(verts[i][0], verts[i][1], verts[i][2]),
          Point(verts[i + 1][0], verts[i + 1][1], verts[i + 1][2]),
          Vector(norms[0][0], norms[0][1], norms[0][2]),
          Vector(norms[i][0], norms[i][1], norms[i][2]),
          Vector(norms[i + 1][0], norms[i + 1][1], norms[i + 1][2]));
    } else {
      p = new Triangle(
          Point(verts[0][0], verts[0][1], verts[0][2]),
          Point(verts[i][0], verts[i][1], verts[i][2]),
          Point(verts[i + 1][0], verts[i + 1][1], verts[i + 1][2]));
    }

    p->setMaterial(current_material);

    scene.addPrimitive(p);
  }

  free(verts);
  if (ispatch)
    free(norms);

  return;
fmterr:
  show_error("polygon or patch syntax error");
  exit(1);
memerr:
  show_error("can't allocate memory for polygon or patch");
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

  tga_reader::readTexture(texture_file, scene);
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
      case 'L': do_area_light (scene, fp); break;
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
