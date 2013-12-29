#ifndef RAY_COMMON_H
#define RAY_COMMON_H

#include <cmath>
#include <limits>

typedef double floating;

/**
 * TODO: this should be machine epsilon.
 */
constexpr floating epsilon = 1e-7; // std::numeric_limits<floating>::epsilon ();

/**
 * How much do we need to pump the new ray away from the intersection (in the
 * direction of the normal) to avoid invalid self-intersections.
 */
constexpr floating ray_epsilon = 1e-7;

constexpr floating RAY_PI = (floating) M_PI;

constexpr floating RAY_INV_PI = (floating)(1.0 / RAY_PI);

inline bool almost_equal(floating x, floating y, int ulp = 1) {
  return fabs(x - y) <= epsilon * fmax(fabs(x), fabs(y)) * ulp;
}

inline bool almost_zero(floating x, int ulp = 1) {
  return fabs(x) <= epsilon * fabs(x) * ulp;
}

inline floating clamp (floating x, floating mn = 0.0, floating mx = 1.0) {
  using namespace std;
  return fmin (mx, fmax (x, mn));
}

/**
 * @note Code based on https://en.wikipedia.org/wiki/Fresnel_equations
 * @param cosI Co-sine of incoming vector.
 * @param n1 Incoming medium.
 * @param n2 Outgoing medium.
 * @return Probability of reflection between two refractive mediums
 */
inline floating fresnel (floating cosI, floating n1, floating n2) {
    if (cosI < 0.0) {
        return fresnel (- cosI, n2, n1);
    }

    const auto n = n1 / n2;
    const auto sinT2 = n*n*(1.0 - cosI*cosI);
    if (sinT2 > 1.0) return 1.0;
    const auto term = std::sqrt(1.0 - sinT2);
    const auto Rs = (n1*cosI - n2*term) / (n1*cosI + n2*term);
    const auto Rp = (n1*term - n2*cosI) / (n1*term + n2*cosI);
    return (Rs*Rs + Rp*Rp) / 2.0;
}

/**
 * Maximum number of times ray is bounced around the scene.
 * This is not the only parameter that determins the maximum
 * number of bounces, the recursion is also cut when rays contribution
 * to pixels colour becomes trivially small.
 */
constexpr std::size_t RAY_MAX_REC_DEPTH = 5;

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
  #if GCC_VERSION >= 40800
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
