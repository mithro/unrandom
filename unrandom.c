#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

static void (*orig_srand)(unsigned int seed);

void srand(unsigned int seed) {
  char *v;

  if(!orig_srand) {
    orig_srand = dlsym(RTLD_NEXT, "srand");
    assert(orig_srand);
  }

  v = getenv("UNRANDOM_SEED");
  orig_srand(v?atoi(v):0);
}

static int (*orig_open)(const char *pathname, int flags, ...);

int open(const char *pathname, int flags, ...) {
  va_list ap;
  mode_t mode;

  if(!orig_open) {
    orig_open = dlsym(RTLD_NEXT, "open");
    assert(orig_open);
  }

  va_start(ap, flags);
#if SIZEOF_MODE_T < SIZEOF_INT
  mode= (mode_t) va_arg(ap, int);
#else
  mode= va_arg(ap, mode_t);
#endif
  va_end(ap);

  if (strcmp(pathname, "/dev/urandom", mode) == 0) {
    return orig_open("/dev/zero", flags, mode);
  } else {
    return orig_open(pathname, flags, mode);
  }
}
