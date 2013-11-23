#ifndef RAY_COMMON_H
#define RAY_COMMON_H

#include <cmath>
#include <limits>

typedef double floating;

/**
 * TODO: this should be machine epsilon.
 */
constexpr floating epsilon = 1e-5; // std::numeric_limits<floating>::epsilon ();

/**
 * How much do we need to pump the new ray away from the intersection (in the
 * direction of the normal) to avoid invalid self-intersections.
 */
constexpr floating ray_epsilon = 1e-5;

inline bool almost_equal(floating x, floating y, int ulp = 1) {
  return fabs(x - y) < epsilon * fmax(fabs(x), fabs(y)) * ulp;
}

inline bool almost_zero(floating x, int ulp = 1) {
  return fabs(x) < epsilon * fabs(x) * ulp;
}

inline floating clamp (floating x, floating mn = 0.0, floating mx = 1.0) {
  using namespace std;
  return fmin (mx, fmax (x, mn));
}

/**
 * How many samples per pixel to take.
 */
constexpr std::size_t SAMPLES = 25;

/**
 * This constant is used to determin if subpixel block has to be subdivided.
 * In more detail, we compare the difference of 4 corners and middle of
 * the subpixel block pixels, if any 2 differ more than the given constant
 * the block is divided into 4 and each subblock is reqursively traced.
 */
constexpr floating AA_TOLERANCE = 0.1;

/**
 * Maximum depth to which pixel block is subdivided.
 * This constant determins maximum number of traced rays, note
 * that the maximum is almost never reached.
 * <PRE>
 * Value    Maximum rays
 * 1        9
 * 2        25
 * 3        81
 * ..       ..
 * n        (2^n + 1)^2
 * </PRE>
 */
constexpr bool MULTISAMPLE = false;
constexpr std::size_t AA_MAX_DEPTH = 2;

/**
 * Maximum number of times ray is bounced around the scene.
 * This is not the only parameter that determins the maximum
 * number of bounces, the recursion is also cut when rays contribution
 * to pixels colour becomes trivially small.
 */
constexpr std::size_t RAY_MAX_REC_DEPTH = 16;

/**
 * If rays contribution to pixels colour becomes less than
 * this constant then recursion is cut.
 */
constexpr floating RAY_MIN_COLOUR_TOLERANCE = 0.000001;

/**
 * Check compiler versions if we have thread_local keyword.
 */

#ifdef __GNUC__
  #define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATHLEVEL__)
  #if GCC_VERSION >= 30800
    #define HAVE_THREAD_LOCAL_STORAGE 1
  #endif
#endif

#ifdef __clang__
  #define CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
  #if CLANG_VERSION >= 30200
    #define HAVE_THREAD_LOCAL_STORAGE 1
  #endif
#endif

#endif
