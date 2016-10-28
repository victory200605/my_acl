#include "apl/poll.h"
#include "apl/heap.h"
#include "apl/errno.h"
#include "apl/io.h"

/* ---------------------------------------------------------------------- */

apl_int_t apl_pollset_init(
    struct apl_pollset_t* aps_pollset,
    apl_size_t au_maxfd)
{
    aps_pollset->mi_pollset = pollset_create(au_maxfd);

    if (aps_pollset->mi_pollset < 0)
    {
        return APL_INT_C(-1);
    }

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_pollset_destroy(
    struct apl_pollset_t* aps_pollset)
{
    return (apl_int_t) pollset_destroy(aps_pollset->mi_pollset);
}
   
/* ---------------------------------------------------------------------- */
    
apl_int_t apl_pollset_add_handle(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes,
    apl_int_t               ai_events)
{
    struct poll_ctl     ls_pctl;

    ls_pctl.cmd     = PS_ADD;
    ls_pctl.events  = ai_events;
    ls_pctl.fd      = (int) ai_fildes;

    return (apl_int_t)pollset_ctl(aps_pollset->mi_pollset, &ls_pctl, 1);
}
    
/* ---------------------------------------------------------------------- */
    
apl_int_t apl_pollset_del_handle(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes)
{
    struct poll_ctl     ls_pctl;

    ls_pctl.cmd     = PS_DELETE;
    ls_pctl.events  = 0;
    ls_pctl.fd      = (int) ai_fildes;

    return (apl_int_t)pollset_ctl(aps_pollset->mi_pollset, &ls_pctl, 1);
}    

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_pollset_set_events(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes,
    apl_int_t               ai_events)
{
    apl_int_t           li_ret;

    li_ret = apl_pollset_del_handle(aps_pollset, ai_fildes);

    if (li_ret < 0)
    {
        return li_ret;
    } 

    li_ret = apl_pollset_add_handle(aps_pollset, ai_fildes, ai_events);

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_poll(
    struct apl_pollset_t*   aps_pollset,
    struct apl_pollfd_t*    aps_outfds,
    apl_size_t              ai_outfdslen,
    apl_time_t              ai64_timeout)
{
    int                     li_timeout;

    li_timeout = ai64_timeout < 0 
        ? APL_INT_C(-1) 
        : (ai64_timeout / APL_TIME_MSEC);

    return (apl_int_t) pollset_poll(
        aps_pollset->mi_pollset, 
        aps_outfds,
        ai_outfdslen,
        li_timeout);
}

/* ---------------------------------------------------------------------- */

