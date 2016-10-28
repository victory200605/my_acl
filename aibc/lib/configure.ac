#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([libaibc],[1.0.0],[fengzz@asiainfo.com])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
gl_EARLY
AC_PROG_CXX


AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE

AC_CONFIG_SRCDIR([include/aibc_config.h.in])
AC_CONFIG_HEADERS([include/aibc_config.h])

LT_INIT
# Checks for libraries.

# Checks for header files.
AC_HEADER_ASSERT

AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([fcntl.h sys/fcntl.h])
AC_CHECK_HEADERS([time.h sys/time.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/sockio.h])
AC_CHECK_HEADERS([sys/devpoll.h])
AC_CHECK_HEADERS([sys/epoll.h])
AC_CHECK_HEADERS([sys/pollset.h])
AC_CHECK_HEADERS([sys/mman.h])
AC_CHECK_HEADERS([spawn.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_LANG_WERROR
AC_C_INLINE
AC_C_VOLATILE
AC_C_BIGENDIAN


AC_TYPE_OFF_T

AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INTMAX_T
AC_TYPE_INTPTR_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINTMAX_T
AC_TYPE_UINTPTR_T

AC_CHECK_SIZEOF([int8_t])
AC_CHECK_SIZEOF([int16_t])
AC_CHECK_SIZEOF([int32_t])
AC_CHECK_SIZEOF([int64_t])
AC_CHECK_SIZEOF([intmax_t])
AC_CHECK_SIZEOF([intptr_t])
AC_CHECK_SIZEOF([uint8_t])
AC_CHECK_SIZEOF([uint16_t])
AC_CHECK_SIZEOF([uint32_t])
AC_CHECK_SIZEOF([uint64_t])
AC_CHECK_SIZEOF([uintmax_t])
AC_CHECK_SIZEOF([uintptr_t])

AC_CHECK_MEMBER([struct sockaddr.sa_len],
 AC_DEFINE(HAVE_SOCKADDR_SA_LEN, 1, define if socket address structures have length fields),,[
#include <sys/types.h>
#include <sys/socket.h>])

# Checks for library functions.
AC_SEARCH_LIBS(pthread_create, [pthread])
AC_SEARCH_LIBS(sem_init, [rt])
AC_SEARCH_LIBS(if_indextoname, [socket nsl])
AC_SEARCH_LIBS(dlopen, [dl])
AC_SEARCH_LIBS(clock_gettime, [rt])
AC_SEARCH_LIBS(getaddrinfo, [nsl])
AC_SEARCH_LIBS(gethostbyname, [nsl])
AC_CHECK_FUNCS([shm_open])
AC_CHECK_FUNCS([shm_unlink])
AC_CHECK_FUNCS([posix_fadvise])
AC_CHECK_FUNCS([posix_fallocate])
AC_CHECK_FUNCS([posix_madvise])
AC_CHECK_FUNCS([posix_spawn])
AC_CHECK_FUNCS([pthread_cond_timedwait])
AC_CHECK_FUNCS([pthread_mutex_timedlock])
AC_CHECK_FUNCS([pthread_rwlock_timedrdlock])
AC_CHECK_FUNCS([pthread_rwlock_timedwrlock])
AC_CHECK_FUNCS([sem_timedwait])
AC_CHECK_FUNCS([pthread_spin_lock])
AC_CHECK_FUNCS([pthread_mutexattr_setspin_np])
AC_CHECK_FUNCS([poll])
AC_CHECK_FUNCS([getaddrinfo])
AC_CHECK_FUNCS([gethostbyname])
AC_CHECK_FUNCS([gethostbyname_r])
AC_CHECK_FUNCS([strtoumax])
AC_CHECK_FUNCS([strtoimax])

gl_INIT

case $host in
    *aix*)      EXTRA_FLAGS='-pthread -D_REENTRANT -D_XOPEN_SOURCE=600 -DAPL_OS_AIX=1' 
                AC_SUBST([REPLACE_STRPTIME], [1])
                AC_SUBST([AR_FLAGS], ['-X32_64 cru'])
                AC_SUBST([NM], ['nm -X32_64 -B'])
                AC_SUBST([REPLACE_VSNPRINTF], [0]) ;;
    *hpux*)     EXTRA_FLAGS='-pthread -D_XOPEN_SOURCE_EXTENDED=1 -DAPL_OS_HPUX=1'
                AC_SUBST([REPLACE_STRPTIME], [1])
                AC_SUBST([REPLACE_VSNPRINTF], [0]) ;;
    *linux*)    EXTRA_FLAGS='-pthread -D_XOPEN_SOURCE=600 -DAPL_OS_LINUX=1'
                AC_SUBST([REPLACE_STRPTIME], [0])
                AC_SUBST([REPLACE_LOCALTIME_R], [0])
                AC_SUBST([REPLACE_MEMCHR], [0])
                AC_SUBST([REPLACE_VSNPRINTF], [0]) ;;
    *solaris*)  EXTRA_FLAGS='-pthreads -D_XOPEN_SOURCE=500 -DAPL_OS_SOLARIS=1'
                AC_SUBST([REPLACE_STRPTIME], [0])
                AC_SUBST([REPLACE_LOCALTIME_R], [0])
                AC_SUBST([REPLACE_VSNPRINTF], [0]) ;;
    *)          EXTRA_FLAGS='-pthread -D_XOPEN_SOURCE=500'
                AC_SUBST([REPLACE_STRPTIME], [0])
                AC_SUBST([REPLACE_LOCALTIME_R], [0])
                AC_SUBST([REPLACE_VSNPRINTF], [0]) ;; 
esac

AC_SUBST([EXTRA_FLAGS])

AM_CONDITIONAL([WANT_STRPTIME], [test "$REPLACE_STRPTIME" = 1])
AM_CONDITIONAL([WANT_VSNPRINTF], [test "$REPLACE_VSNPRINTF" = 1])

AC_CONFIG_FILES([Makefile include/Makefile apl/Makefile apl/include/Makefile apl/src/Makefile apl/gl/Makefile apl/test/Makefile acl/Makefile acl/include/Makefile acl/src/Makefile acl/test/Makefile anf/Makefile anf/include/Makefile anf/src/Makefile anf/test/Makefile])
#AC_CONFIG_FILES([Makefile apl/Makefile apl/include/Makefile apl/src/Makefile apl/gl/Makefile apl/test/Makefile])

AC_OUTPUT

