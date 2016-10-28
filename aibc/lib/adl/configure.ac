#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([adl],[1.0.0],[hezk@asiainfo.com])


AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX

AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE

AC_CONFIG_SRCDIR([include/AdlConfig.h.in])
AC_CONFIG_HEADERS([include/AdlConfig.h])
AC_CONFIG_FILES([Makefile src/Makefile src/adl/Makefile src/mysql/Makefile src/oracle/Makefile src/sqlite/Makefile test/Makefile include/Makefile])

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

AC_ARG_ENABLE(mysql,
              [  --enable-mysql/--disable-mysql    Enable/Disable mysql lib],
              [case "${enableval}" in
               yes) mysql=true ;;
               no)  mysql=false ;;
               *) AC_MSG_ERROR(bad value ${enableval} for --enable-mysql/--disable-mysql) ;;
           esac],[mysql=true])

AC_ARG_ENABLE(oracle,
              [  --enable-oracle/--disable-oracle    Enable/Disable oracle lib],
              [case "${enableval}" in
               yes) oracle=true ;;
               no)  oracle=false ;;
               *) AC_MSG_ERROR(bad value ${enableval} for --enable-oracle/--disable-oracle) ;;
           esac],[oracle=true])

AC_ARG_ENABLE(sqlite,
              [  --enable-sqlite/--disable-sqlite    Enable/Disable sqlite lib],
              [case "${enableval}" in
               yes) sqlite=true ;;
               no)  sqlite=false ;;
               *) AC_MSG_ERROR(bad value ${enableval} for --enable-sqlite/--disable-sqlite) ;;
           esac],[sqlite=true])

AM_CONDITIONAL(ADL_MYSQL_ENABLE, [test x$mysql = xtrue])
AM_CONDITIONAL(ADL_ORACLE_ENABLE, [test x$oracle = xtrue]) 
AM_CONDITIONAL(ADL_SQLITE_ENABLE, [test x$sqlite = xtrue]) 

AC_PROG_LN_S

AC_OUTPUT
