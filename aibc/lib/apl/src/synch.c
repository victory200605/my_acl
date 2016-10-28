#include "apl/synch.h"
#include "apl/time.h"
#include "apl/errno.h"

#if defined(HAVE_PTHREAD_COND_TIMEDWAIT)
#include "posix_cond.c"
#include "posix_mutex.c"

#   if defined(HAVE_PTHREAD_RWLOCK_TIMEDRDLOCK) && defined(HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK)
#       include "posix_rwlock.c"
#   else
#       include "sim_rwlock.c"
#   endif

#   if defined(HAVE_SEM_TIMEDWAIT)
#       include "posix_sema.c"
#   else
#       include "sim_sema.c"
#   endif

#else
#   error unsupported system
#endif

#if defined(HAVE_PTHREAD_SPIN_LOCK)
#   include "posix_spin.c"
#else
#   include "sim_spin.c"
#endif

