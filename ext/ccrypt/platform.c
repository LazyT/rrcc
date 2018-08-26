/* Platform specific stuff. */

#ifdef __MINGW32__

#include <windows.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "platform.h"

/* Windows replacement for gethostname() */
int mingw_compat_gethostname(char *name, size_t len) {
  char buf[MAX_COMPUTERNAME_LENGTH + 1];
  int r;
  DWORD len1 = sizeof(buf);

  r = GetComputerNameA(buf, &len1);
  if (r==0) {
    name[0] = '\0';
  } else {
    strncpy(name, buf, len-1);
    name[len-1] = '\0';
  }
  return 0;
}

/* Windows replacement for gettimeofday(). Fills a struct timeval with
   the current time, ignoring the timezone argument. Note: although
   the result is in microseconds, it has only millisecond resolution. */
int gettimeofday(struct timeval *tv, void *tz) {
  SYSTEMTIME st;
  FILETIME ft;
  long long int t;

  GetSystemTime(&st);              /* get current time (millisec resolution) */
  SystemTimeToFileTime(&st, &ft);  /* converts to file time */
  t = ((long long)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
                                   /* 100 nanosec since Jan 1, 1601 UTC */ 
  t -= 116444736000000000;          /* 100 nanosec since Jan 1, 1970 UTC */
  t /= 10;                         /* microsec since Jan 1, 1970 UTC */ 
  tv->tv_usec = t % 1000000;
  tv->tv_sec = t / 1000000;
  return 0; 
}

/* generic replacement for mkstemp() */
int mkstemp(char *template) {
  int len = strlen(template);
  int errno_save = errno;
  int fd;
  char *p;
  int i;
  const int max_tries = 1024;
  
  if (len < 6) {
    errno = EINVAL;
    return -1;
  }
  for (i=0; i<max_tries; i++) {
    p = mktemp(template);
    if (!p || p[0] == 0) {
      return -1;
    }
    fd = open(template, O_CREAT | O_EXCL | O_RDWR | O_BINARY, S_IRUSR | S_IWUSR);
    if (fd >= 0) {
      errno = errno_save;
      return fd;
    }
    if (errno != EEXIST) {
      return -1;
    }
    /* there was a race condition: a file of this name was created
       between the mktemp and open calls. We just try again */
    memset(template+len-6, 'X', 6);
  }
  errno = EEXIST;
  return -1;
}

#endif /* __MINGW32__ */

/* ISO C forbids an empty translation unit, so we put a dummy
   declaration here to turn off the warning. */
typedef int dummy_declaration;
