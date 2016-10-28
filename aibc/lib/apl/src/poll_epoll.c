#include "apl/poll.h"
#include "apl/errno.h"
#include "apl/heap.h"
#include "apl/io.h"

/* ---------------------------------------------------------------------- */

apl_int_t apl_pollset_init(
    struct apl_pollset_t*   aps_pollset,
    apl_size_t ai_maxfd)
{
    aps_pollset->mi_epfd = (apl_handle_t) epoll_create(ai_maxfd);

    if (aps_pollset->mi_epfd < 0)
    {
        return APL_INT_C(-1);
    }

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_pollset_destroy(
    struct apl_pollset_t*   aps_pollset)
{
    apl_int_t   li_ret;

    li_ret = apl_close(aps_pollset->mi_epfd);
    
    return li_ret;
}
   
/* ---------------------------------------------------------------------- */
    
apl_int_t apl_pollset_add_handle(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes,
    apl_int_t               ai_events)
{
    struct apl_pollfd_t     ls_ev;
    apl_int_t               li_ret;
    
    ls_ev.data.fd = ai_fildes;
    ls_ev.events  = ai_events;

    li_ret = epoll_ctl(
        aps_pollset->mi_epfd,
        EPOLL_CTL_ADD,
        ls_ev.data.fd,
        &ls_ev);
    
    return li_ret;
}
    
/* ---------------------------------------------------------------------- */
    
apl_int_t apl_pollset_del_handle(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes)
{
    struct apl_pollfd_t     ls_ev;
    apl_int_t               li_ret;

    ls_ev.data.fd = ai_fildes;

    li_ret = epoll_ctl(
        aps_pollset->mi_epfd,
        EPOLL_CTL_DEL,
        ls_ev.data.fd,
        &ls_ev);

    return li_ret;
}    

/* ---------------------------------------------------------------------- */

apl_int_t apl_pollset_set_events(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes,
    apl_int_t               ai_events)
{
    struct apl_pollfd_t     ls_ev;
    apl_int_t               li_ret;

    ls_ev.events  = ai_events;
    ls_ev.data.fd = ai_fildes;

    li_ret = epoll_ctl(
        aps_pollset->mi_epfd,
        EPOLL_CTL_MOD,
        ls_ev.data.fd,
        &ls_ev);
 
    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_poll(
    struct apl_pollset_t*   aps_pollset,
    struct apl_pollfd_t*    aps_outfds,
    apl_size_t              ai_outfdslen,
    apl_time_t              ai64_timeout)
{
    int                     li_timeout;
    apl_int_t               li_ret;

    li_timeout = ai64_timeout < 0 
        ? APL_INT_C(-1) 
        : (ai64_timeout / APL_TIME_MSEC);

    li_ret = epoll_wait(
        aps_pollset->mi_epfd,
        aps_outfds,
        ai_outfdslen,
        li_timeout);
    
    return li_ret;  
}

/* ---------------------------------------------------------------------- */

