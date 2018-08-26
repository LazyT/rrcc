/* Platform-specific stuff. */

/* The general approach is to: (1) program for a generic POSIX-like
   system with added support for text/binary files, and (2) put
   platform-dependent hacks here.  There are some cases where this may
   not work, such as terminal I/O. In this case, some platform
   dependent code may appear in individual source files.

   Note: the dummy replacements for missing functions provided here
   are not fully functional; they just work well enough for our
   purposes. In many cases they are no-ops. */

#ifndef PLATFORM_H
#define PLATFORM_H

/* ---------------------------------------------------------------------- */
/* Windows/MinGW, dummy replacements for some POSIX functions */
#ifdef __MINGW32__

//#error "Ccrypt cannot currently be compiled under MinGW. For details, see platform.h"
/* Some problems with MinGW are:
   - there is no way to read passwords from console in MinGW that
     works for Windows console, Msys console, Cygwin console, and
     xterm under X
   - "unlink" will not work on open files, and "rename" does not work
     if the destination exists
   - getpid() does not return a meaningful value
   - stat() returns an empty inode field
   - progress display using VT100 terminal codes does not work on Windows
     console
*/

#include <sys/time.h>  /* for struct timeval */

/* traverse.c */

/* treat symbolic links, permissions, and ownership as no-ops */
#define lstat(x,y) stat(x,y)
#define S_ISLNK(x) 0

#define S_IWGRP 0
#define S_IWOTH 0

static inline int fchown(int fd, int owner, int group) {
  return 0;
}
static inline int fchmod(int fd, mode_t mode) {
  return 0;
}

/* mkstemp replacement */
int mkstemp(char *template);

/* ccrypt.c */

#define ftruncate(fd, len) chsize(fd, len)

/* ccryptlib.c */

/* avoid conflict with a declaraion in winsock2.h */
#define gethostname(name, len) mingw_compat_gethostname(name, len)
int mingw_compat_gethostname(char *name, size_t len);

int gettimeofday(struct timeval *tv, void *tz);

#endif /* __MINGW32__ */ 

/* ---------------------------------------------------------------------- */
/* setmode() and O_BINARY */

#if defined(__CYGWIN__)

#include <io.h>  /* for setmode() */

#elif defined(__MINGW32__) || defined(__EMX__)

/* nothing needed */

#else /* on a POSIX system, map these to no-ops */

/* BSD defines setmode() with a different meaning, in unistd.h */
static inline void ccrypt_setmode(int fd, int mode) {
  return;
}
#undef setmode
#define setmode(x,y) ccrypt_setmode(x,y)

#ifndef O_BINARY
#define O_BINARY 0
#endif /* O_BINARY */

#endif /* __CYGWIN__ || __MINGW32__ || etc */

/* ---------------------------------------------------------------------- */
#endif /* PLATFORM_H */
