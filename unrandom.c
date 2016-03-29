#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdlib.h>
#include <assert.h>

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

static int (*orig_open)(const char *pathname, int flags, mode_t mode);

int open(const char *pathname, int flags, mode_t mode) {
  if(!orig_open) {
    orig_open = dlsym(RTLD_NEXT, "open");
    assert(orig_open);
  }

  if (strcmp(pathname, "/dev/urandom", mode) == 0) {
    return orig_open("/dev/zero", flags, mode);
  } else {
    return orig_open(pathname, flags, mode);
  }
}
