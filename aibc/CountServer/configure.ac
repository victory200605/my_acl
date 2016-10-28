#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([CountServer],[1.0.1],[daizh@asiainfo.com])


AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX

AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE

AC_CONFIG_SRCDIR([include/CSvrConfig.h.in])
AC_CONFIG_HEADERS([include/CSvrConfig.h])
AC_CONFIG_FILES([Makefile api/Makefile src/Makefile test/Makefile include/Makefile config/Makefile])

LT_INIT

AC_SEARCH_LIBS(pthread_create, [pthread])
AC_SEARCH_LIBS(sem_init, [rt])
AC_SEARCH_LIBS(if_indextoname, [socket nsl])
AC_SEARCH_LIBS(dlopen, [dl])
AC_SEARCH_LIBS(clock_gettime, [rt])
AC_SEARCH_LIBS(getaddrinfo, [nsl])
AC_SEARCH_LIBS(gethostbyname, [nsl])

case $host in
    *aix*)      EXTRA_FLAGS='-pthread -D_REENTRANT -D_XOPEN_SOURCE=600 -DAPL_OS_AIX=1'
                AC_SUBST([AR_FLAGS], ['-X32_64 cru'])
                AC_SUBST([NM], ['nm -X32_64 -B']);;
    *hpux*)     EXTRA_FLAGS='-pthread -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED=1 -DAPL_OS_HPUX=1';;
    *linux*)    EXTRA_FLAGS='-pthread -D_XOPEN_SOURCE=600 -DAPL_OS_LINUX=1';;
    *solaris*)  EXTRA_FLAGS='-pthreads -D_XOPEN_SOURCE=500 -DAPL_OS_SOLARIS=1';;
    *)          EXTRA_FLAGS='-pthread -D_XOPEN_SOURCE=500';;
esac

AC_SUBST([EXTRA_FLAGS])

AC_OUTPUT
