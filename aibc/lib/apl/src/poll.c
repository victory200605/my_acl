#include "apl/poll.h"

#include <poll.h>
/*
#define PFDS_INIT_SIZE      APL_UINT_C(64)
#define PFDS_INC_FACTOR     APL_UINT_C(2)

#define INIT_POLLSET_POLLFDS(pps, pfd) \
    do { \
        (pps)->mps_pfds = (stru*)malloc(sizeof(stru) * PFDS_INIT_SIZE); \
        assert((pps)->mps_pfds); \
        (pps)->mu_maxfds = PFDS_INIT_SIZE; \
    } while(0)


#define DESTROY_POLLSET_POLLFDS(pps) \
    do { \
        free((pps)->mps_pfds); \
    } while(0)

#define SETDEFAULT_POLLSET_POLLFD(pps, 
*/

#if defined(HAVE_SYS_EPOLL_H) /* LINUX */
#   include "poll_epoll.c"
#elif defined(HAVE_SYS_DEVPOLL_H) /* SOLARIS/HP-UX */
#   include "poll_devpoll.c"
#elif defined(HAVE_SYS_POLLSET_H) /* AIX */
#   include "poll_pollset.c"
#else
#   include "poll_poll.c"
#endif


apl_int_t apl_poll_one(
    apl_handle_t            ai_fildes,
    apl_int_t               ai_events,
    apl_int_t*              api_revents,
    apl_time_t              ai64_timeout)
{
    struct pollfd   ls_pfd;
    int             li_timeout;
    apl_int_t       li_ret;

    ls_pfd.fd       = ai_fildes;
    ls_pfd.events   = 0;

    if (ai_events & APL_POLLIN)
    {
        ls_pfd.events |= POLLIN;
    }

    if (ai_events & APL_POLLOUT)
    {
        ls_pfd.events |= POLLOUT;
    }

    if (ai_events & APL_POLLPRI)
    {
        ls_pfd.events |= POLLPRI;
    }

    if (ai_events & APL_POLLHUP)
    {
        ls_pfd.events |= POLLHUP;
    }

    li_timeout = ai64_timeout < 0 ? -1 : (ai64_timeout / APL_TIME_MSEC);

    li_ret = (apl_int_t) poll(&ls_pfd, (nfds_t)1, li_timeout);

    if (APL_INT_C(1) == li_ret && api_revents)
    {
        *api_revents = ls_pfd.revents;
    }

    return li_ret;
}

