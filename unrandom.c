#define _GNU_SOURCE

#include <time.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
static char* statm_path = NULL;
static char* cpuinfo_path = NULL;

int open(const char *pathname, int flags, ...) {
  va_list ap;
  mode_t mode;

  if(!orig_open) {
    orig_open = dlsym(RTLD_NEXT, "open");
    assert(orig_open);

    statm_path = getenv("STATM_PATH");
    assert(statm_path);

    cpuinfo_path = getenv("CPUINFO_PATH");
    assert(cpuinfo_path);
  }

  va_start(ap, flags);
#if SIZEOF_MODE_T < SIZEOF_INT
  mode= (mode_t) va_arg(ap, int);
#else
  mode= va_arg(ap, mode_t);
#endif
  va_end(ap);

  if (strcmp(pathname, "/dev/urandom") == 0) {
    fprintf(stderr, "Opening /dev/zero rather than /dev/urandom\n");
    return orig_open("/dev/zero", flags, mode);
  } else if (strcmp(pathname, "/proc/self/statm") == 0) {
    fprintf(stderr, "Opening custom statm\n");
    return orig_open(statm_path, flags, mode);
  } else if (strcmp(pathname, "/proc/cpuinfo") == 0) {
    fprintf(stderr, "Opening custom cpuinfo\n");
    return orig_open(cpuinfo_path, flags, mode);
  } else {
    fprintf(stderr, "Opening %s\n", pathname);
    return orig_open(pathname, flags, mode);
  }
}

#define TIME_TS 1451610000


/*
static time_t       (*orig__time)            (time_t *);
  real_time =               dlsym(RTLD_NEXT, "time");


  real_ftime =              dlsym(RTLD_NEXT, "ftime");
static int          (*orig__ftime)           (struct timeb *);
static int          (*orig___ftime)           (struct timeb *);

  real_gettimeofday =       dlsym(RTLD_NEXT, "gettimeofday");
static int          (*orig__gettimeofday)    (struct timeval *, void *);
static int          (*orig___gettimeofday)    (struct timeval *, void *);


static int          (*orig__clock_gettime)   (clockid_t clk_id, struct timespec *tp);
static int          (*orig___clock_gettime)   (clockid_t clk_id, struct timespec *tp);
*/


//static int (*clock_gettime)(clockid_t clk_id, struct timespec *tp);

//           struct timespec {
//               time_t   tv_sec;        /* seconds */
//               long     tv_nsec;       /* nanoseconds */
//           };

int clock_gettime(clockid_t clk_id, struct timespec *tp) {
  struct timespec td = { TIME_TS, 0 };
  if(tp) *tp = td;
  return 0;
}

time_t time(time_t *t) {
  if (t) *t = TIME_TS;
  return TIME_TS;
}

//           struct tms {
//               clock_t tms_utime;  /* user time */
//               clock_t tms_stime;  /* system time */
//               clock_t tms_cutime; /* user time of children */
//               clock_t tms_cstime; /* system time of children */
//           };
clock_t times(struct tms *buf) {
  buf->tms_utime = 1;
  buf->tms_stime = 1;
  buf->tms_cutime = 1;
  buf->tms_cstime = 1;
  return 0;
}

//           struct timeval {
//               time_t      tv_sec;     /* seconds */
//               suseconds_t tv_usec;    /* microseconds */
//           };
int gettimeofday(struct timeval *tv, struct timezone *tz) {
  static struct timeval  tdv = { TIME_TS, 0 }; 
  static struct timezone tdz = { 0, 0}; 
  if(tv) *tv = tdv; 
  if(tz) *tz = tdz; 
  return 0;
}

//           struct timeval {
//               time_t      tv_sec;     /* seconds */
//               suseconds_t tv_usec;    /* microseconds */
//           };
int __gettimeofday(struct timeval *tv, struct timezone *tz) {
  static struct timeval  tdv = { TIME_TS, 0 }; 
  static struct timezone tdz = { 0, 0}; 
  if(tv) *tv = tdv; 
  if(tz) *tz = tdz; 
  return 0;
}


/*******************************************************************************************
 * Stat functions
 *******************************************************************************************/

/*
  real_stat =               dlsym(RTLD_NEXT, "__xstat");
  real_fstat =              dlsym(RTLD_NEXT, "__fxstat");
  real_fstatat =            dlsym(RTLD_NEXT, "__fxstatat");
  real_lstat =              dlsym(RTLD_NEXT, "__lxstat");
  real_stat64 =             dlsym(RTLD_NEXT, "__xstat64");
  real_fstat64 =            dlsym(RTLD_NEXT, "__fxstat64");
  real_fstatat64 =          dlsym(RTLD_NEXT, "__fxstatat64");
  real_lstat64 =            dlsym(RTLD_NEXT, "__lxstat64");
*/

//           struct stat {
//               dev_t     st_dev;     /* ID of device containing file */
//               ino_t     st_ino;     /* inode number */
//               mode_t    st_mode;    /* protection */
//               nlink_t   st_nlink;   /* number of hard links */
//               uid_t     st_uid;     /* user ID of owner */
//               gid_t     st_gid;     /* group ID of owner */
//               dev_t     st_rdev;    /* device ID (if special file) */
//               off_t     st_size;    /* total size, in bytes */
//               blksize_t st_blksize; /* blocksize for filesystem I/O */
//               blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
//               time_t    st_atime;   /* time of last access */
//               time_t    st_mtime;   /* time of last modification */
//               time_t    st_ctime;   /* time of last status change */
//           };

void munge_stat(struct stat *buf) {
  if (buf) {
    buf->st_atime = TIME_TS - 1;
    buf->st_mtime = TIME_TS - 1;
    buf->st_ctime = TIME_TS - 1;
  }
}

void munge_stat64(struct stat64 *buf) {
  if (buf) {
    buf->st_atime = TIME_TS - 1;
    buf->st_mtime = TIME_TS - 1;
    buf->st_ctime = TIME_TS - 1;
  }
}

static int          (*orig__xstat)            (int, const char *, struct stat *);
int __xstat (int ver, const char *path, struct stat *buf) {
  if(!orig__xstat) {
    orig__xstat = dlsym(RTLD_NEXT, "__xstat");
    assert(orig__xstat);
  }
  int r = orig__xstat(ver, path, buf);
  munge_stat(buf);
  return r;
}

static int          (*orig__fxstat)           (int, int, struct stat *);
int __fxstat (int ver, int fildes, struct stat *buf) {
  if(!orig__fxstat) {
    orig__fxstat = dlsym(RTLD_NEXT, "__fxstat");
    assert(orig__fxstat);
  }
  int r = orig__fxstat(ver, fildes, buf);
  munge_stat(buf);
  return r;
}

static int          (*orig__fxstatat)         (int, int, const char *, struct stat *, int);
int __fxstatat(int ver, int fildes, const char *filename, struct stat *buf, int flag) {
  if(!orig__fxstatat) {
    orig__fxstatat = dlsym(RTLD_NEXT, "__fxstatat");
    assert(orig__fxstatat);
  }
  int r = orig__fxstatat(ver, fildes, filename, buf, flag);
  munge_stat(buf);
  return r;
}

static int          (*orig__lxstat)           (int, const char *, struct stat *);
int __lxstat (int ver, const char *path, struct stat *buf) {
  if(!orig__lxstat) {
    orig__lxstat = dlsym(RTLD_NEXT, "__lxstat");
    assert(orig__lxstat);
  }
  int r = orig__lxstat(ver, path, buf);
  munge_stat(buf);
  return r;
}

static int          (*orig__xstat64)          (int, const char *, struct stat64 *);
int __xstat64 (int ver, const char *path, struct stat64 *buf) {
  if(!orig__xstat64) {
    orig__xstat64 = dlsym(RTLD_NEXT, "__xstat64");
    assert(orig__xstat64);
  }
  int r = orig__xstat64(ver, path, buf);
  munge_stat64(buf);
  return r;
}

static int          (*orig__fxstat64)         (int, int , struct stat64 *);
int __fxstat64 (int ver, int fildes, struct stat64 *buf) {
  if(!orig__fxstat64) {
    orig__fxstat64 = dlsym(RTLD_NEXT, "__fxstat64");
    assert(orig__fxstat64);
  }
  int r = orig__fxstat64(ver, fildes, buf);
  munge_stat64(buf);
  return r;
}

static int          (*orig__fxstatat64)       (int, int , const char *, struct stat64 *, int);
int __fxstatat64 (int ver, int fildes, const char *filename, struct stat64 *buf, int flag) {
  if(!orig__fxstatat64) {
    orig__fxstatat64 = dlsym(RTLD_NEXT, "__fxstatat64");
    assert(orig__fxstatat64);
  }
  int r = orig__fxstatat64(ver, fildes, filename, buf, flag);
  munge_stat64(buf);
  return r;
}

static int          (*orig__lxstat64)         (int, const char *, struct stat64 *);
int __lxstat64 (int ver, const char *path, struct stat64 *buf) {
  if(!orig__lxstat64) {
    orig__lxstat64 = dlsym(RTLD_NEXT, "__lxstat64");
    assert(orig__lxstat64);
  }
  int r = orig__lxstat64(ver, path, buf);
  munge_stat64(buf);
  return r;
}




