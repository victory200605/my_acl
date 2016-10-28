#include "apl/poll.h"
#include "apl/heap.h"
#include "apl/errno.h"
#include "apl/io.h"
#include "apl/file.h"
#include <assert.h>

/* ---------------------------------------------------------------------- */

apl_int_t apl_pollset_init(
    struct apl_pollset_t* aps_pollset,
    apl_size_t au_maxfd)
{
    aps_pollset->mi_dpfd = apl_open("/dev/poll", APL_O_RDWR, 0);

    if (aps_pollset->mi_dpfd < 0)
    {
        return APL_INT_C(-1);
    }

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_pollset_destroy(
    struct apl_pollset_t* aps_pollset)
{
    apl_int_t   li_ret;

    li_ret = apl_close(aps_pollset->mi_dpfd); 

    return (apl_int_t)li_ret;
}
   
/* ---------------------------------------------------------------------- */
    
apl_int_t apl_pollset_add_handle(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes,
    apl_int_t               ai_events)
{
    struct apl_pollfd_t     ls_pfd;
    apl_int_t               li_ret;

    ls_pfd.fd = ai_fildes;
    ls_pfd.events = ai_events;
    ls_pfd.revents = 0;

    li_ret = write(
        aps_pollset->mi_dpfd,
        &ls_pfd,
        sizeof(ls_pfd));

    if (li_ret >=0 && sizeof(ls_pfd) != li_ret)
    {
        apl_set_errno(APL_EIO);

        return APL_INT_C(-1);
    }

    return 0;
}
    
/* ---------------------------------------------------------------------- */
    
apl_int_t apl_pollset_del_handle(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes)
{
    struct apl_pollfd_t     ls_pfd;
    apl_int_t               li_ret;

    ls_pfd.fd = ai_fildes;
    ls_pfd.events = POLLREMOVE;
    ls_pfd.revents = 0;

    li_ret = write(
        aps_pollset->mi_dpfd,
        &ls_pfd,
        sizeof(ls_pfd));

    if (li_ret >= 0 && sizeof(ls_pfd) != li_ret)
    {
        apl_set_errno(APL_EIO);

        return APL_INT_C(-1);
    }


    return 0;
}    

/* ---------------------------------------------------------------------- */

apl_int_t apl_pollset_set_events(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes,
    apl_int_t               ai_events)
{
    apl_int_t               li_ret;

    li_ret = apl_pollset_del_handle(aps_pollset, ai_fildes);

    if (li_ret < 0)
    {
        return li_ret;
    }

    li_ret = apl_pollset_add_handle(aps_pollset, ai_fildes, ai_events);

    if (li_ret < 0)
    {
        return li_ret;
    }

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_poll(
    struct apl_pollset_t*   aps_pollset,
    struct apl_pollfd_t*    aps_outfds,
    apl_size_t              au_outfdslen,
    apl_time_t              ai64_timeout)
{
    struct dvpoll           ls_dvpoll;
    apl_int_t               li_ret;

    ls_dvpoll.dp_fds = aps_outfds;
    ls_dvpoll.dp_nfds = au_outfdslen;

    ls_dvpoll.dp_timeout = ai64_timeout < 0 
        ? APL_INT_C(-1) 
        : (ai64_timeout / APL_TIME_MSEC);

    li_ret = (apl_int_t) ioctl(
        aps_pollset->mi_dpfd, 
        DP_POLL,
        &ls_dvpoll);

    return li_ret;
}

/* ---------------------------------------------------------------------- */

