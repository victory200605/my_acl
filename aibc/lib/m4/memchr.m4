# memchr.m4 serial 6
dnl Copyright (C) 2002, 2003, 2004, 2009 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_MEMCHR],
[
  dnl Check for prerequisites for memory fence checks.
  gl_FUNC_MMAP_ANON
  AC_CHECK_HEADERS_ONCE([sys/mman.h])
  AC_CHECK_FUNCS_ONCE([mprotect])

  dnl These days, we assume memchr is present.  But just in case...
  AC_REQUIRE([gl_HEADER_STRING_H_DEFAULTS])
  AC_REPLACE_FUNCS([memchr])
  if test $ac_cv_func_memchr = no; then
    gl_PREREQ_MEMCHR
    REPLACE_MEMCHR=1
  fi

  if test $ac_cv_func_memchr = yes; then
    # Detect platform-specific bugs in some versions of glibc:
    # memchr should not dereference anything with length 0
    #   http://bugzilla.redhat.com/499689
    # memchr should not dereference overestimated length after a match
    #   http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=521737
    #   http://sourceware.org/bugzilla/show_bug.cgi?id=10162
    # Assume that memchr works on platforms that lack mprotect.
    AC_CACHE_CHECK([whether memchr works], [gl_cv_func_memchr_works],
      [AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <string.h>
#if HAVE_SYS_MMAN_H
# include <fcntl.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/mman.h>
# ifndef MAP_FILE
#  define MAP_FILE 0
# endif
#endif
]], [[
  char *fence = NULL;
#if HAVE_SYS_MMAN_H && HAVE_MPROTECT
# if HAVE_MAP_ANONYMOUS
  const int flags = MAP_ANONYMOUS | MAP_PRIVATE;
  const int fd = -1;
# else /* !HAVE_MAP_ANONYMOUS */
  const int flags = MAP_FILE | MAP_PRIVATE;
  int fd = open ("/dev/zero", O_RDONLY, 0666);
  if (fd >= 0)
# endif
    {
      int pagesize = getpagesize ();
      char *two_pages =
	(char *) mmap (NULL, 2 * pagesize, PROT_READ | PROT_WRITE,
		       flags, fd, 0);
      if (two_pages != (char *)(-1)
	  && mprotect (two_pages + pagesize, pagesize, PROT_NONE) == 0)
	fence = two_pages + pagesize;
    }
#endif
  if (fence)
    {
      if (memchr (fence, 0, 0))
        return 1;
      strcpy (fence - 9, "12345678");
      if (memchr (fence - 9, 0, 79) != fence - 1)
        return 2;
    }
  return 0;
]])], [gl_cv_func_memchr_works=yes], [gl_cv_func_memchr_works=no],
      [dnl Be pessimistic for now.
       gl_cv_func_memchr_works="guessing no"])])
    if test "$gl_cv_func_memchr_works" != yes; then
      gl_PREREQ_MEMCHR
      REPLACE_MEMCHR=1
      AC_LIBOBJ([memchr])
    fi
  fi
])

# Prerequisites of lib/memchr.c.
AC_DEFUN([gl_PREREQ_MEMCHR], [
  AC_CHECK_HEADERS([bp-sym.h])
])
