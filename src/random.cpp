#include "random.h"

#ifndef HAVE_THREAD_LOCAL_STORAGE
  #include <boost/trhead/tss.hpp>
#endif

Engine& rng_engine () {
#ifdef HAVE_THREAD_LOCAL_STORAGE
  static thread_local auto
    the_rng_engine = Engine {std::random_device {}()};
  return the_rng_engine;
#else
  static boost::thread_specific_ptr<Engine> engine_ptr;
  if (! engine_ptr.get ()) {
    engine_ptr.reset (new Engine {std::random_device{}()});
  }

  return *engine_ptr;
#endif
}
