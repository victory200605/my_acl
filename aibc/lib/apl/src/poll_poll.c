#include "apl/poll.h"
#include "apl/heap.h"
#include "apl/errno.h"
#include "apl/io.h"
#include "apl/mem.h"
#include <assert.h>
#include <stdlib.h>

/* ---------------------------------------------------------------------- */
int _pfd_comp(void const* apv_lhs, void const* apv_rhs)
{
    return ((struct apl_pollfd_t*)apv_lhs)->fd - ((struct apl_pollfd_t*)apv_rhs)->fd;
}

static inline
struct apl_pollfd_t* search_pfd(struct apl_pollset_t* aps_pollset, apl_int_t ai_fildes)
{
    struct apl_pollfd_t     ls_pfd;

    ls_pfd.fd = ai_fildes;

    return (struct apl_pollfd_t*)bsearch(
        &ls_pfd, 
        aps_pollset->mps_pfds,
        aps_pollset->mu_cur, 
        sizeof(struct apl_pollfd_t),
        _pfd_comp);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_pollset_init(
    struct apl_pollset_t* aps_pollset,
    apl_size_t au_maxfd)
{
    aps_pollset->mps_pfds = (struct apl_pollfd_t*)
        apl_malloc(au_maxfd * sizeof(struct apl_pollfd_t));

    if (APL_NULL == aps_pollset->mps_pfds)
    {
        return -1;
    }

    aps_pollset->mu_max = au_maxfd;
    aps_pollset->mu_cur = 0;

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_pollset_destroy(
    struct apl_pollset_t* aps_pollset)
{
    if (APL_NULL == aps_pollset->mps_pfds)
    {
        return APL_INT_C(-1);
    }

    apl_free((void*)aps_pollset->mps_pfds);

    return 0;
}
   
/* ---------------------------------------------------------------------- */
    
apl_int_t apl_pollset_add_handle(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes,
    apl_int_t               ai_events)
{
    struct apl_pollfd_t*    lps_pfd;

    if (aps_pollset->mu_cur >= aps_pollset->mu_max)
    {
        apl_set_errno(APL_ENOSPC);
        return APL_INT_C(-1);
    }

    if (search_pfd(aps_pollset, ai_fildes) != APL_NULL)
    {
        apl_set_errno(APL_EEXIST);
        return APL_INT_C(-1);
    }

    lps_pfd = &(aps_pollset->mps_pfds[aps_pollset->mu_cur]);
    ++(aps_pollset->mu_cur);

    lps_pfd->fd = ai_fildes;
    lps_pfd->events = ai_events;
    lps_pfd->revents = 0;

    qsort(aps_pollset->mps_pfds, aps_pollset->mu_cur, sizeof(struct apl_pollfd_t), _pfd_comp);

    return 0;
}
    
/* ---------------------------------------------------------------------- */
    
apl_int_t apl_pollset_del_handle(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes)
{
    struct apl_pollfd_t*    lps_pfd;

    lps_pfd = search_pfd(aps_pollset, ai_fildes);

    if (APL_NULL == lps_pfd)
    {
        apl_set_errno(APL_ENOENT);
        return APL_INT_C(-1);
    }

    apl_memmove(lps_pfd, lps_pfd + 1, 
        (aps_pollset->mps_pfds + aps_pollset->mu_cur - lps_pfd - 1) * sizeof(struct apl_pollfd_t));
    
    --(aps_pollset->mu_cur);

    return 0;
}    

/* ---------------------------------------------------------------------- */

apl_int_t apl_pollset_set_events(
    struct apl_pollset_t*   aps_pollset,
    apl_handle_t            ai_fildes,
    apl_int_t               ai_events)
{
    struct apl_pollfd_t*    lps_pfd;

    lps_pfd = search_pfd(aps_pollset, ai_fildes);

    if (APL_NULL == lps_pfd)
    {
        apl_set_errno(APL_ENOENT);
        return APL_INT_C(-1);
    }

    lps_pfd->events = (short) ai_events;

    return 0;
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_poll(
    struct apl_pollset_t*   aps_pollset,
    struct apl_pollfd_t*    aps_outfds,
    apl_size_t              ai_outfdslen,
    apl_time_t              ai64_timeout)
{
    int                     li_timeout;
    apl_int_t               li_ret;
    apl_size_t              li_avai;
    apl_size_t              li_itr;

    li_timeout = ai64_timeout < 0 
        ? APL_INT_C(-1) 
        : (ai64_timeout / APL_TIME_MSEC);

    li_ret = poll(
        aps_pollset->mps_pfds, 
        (nfds_t) aps_pollset->mu_cur, 
        li_timeout);
      
    if (li_ret <= 0)
    {
        return li_ret;
    } 

    li_avai = 0;

    for (li_itr = 0;
        li_itr < aps_pollset->mu_cur && li_avai < ai_outfdslen;
        ++li_itr)
    {
        struct apl_pollfd_t*    lps_pfd = &(aps_pollset->mps_pfds[li_itr]);

        if (APL_IS_VALID_HANDLE(lps_pfd->fd) && lps_pfd->revents)
        {
            apl_memcpy(
                &aps_outfds[li_avai], 
                lps_pfd, 
                sizeof(struct apl_pollfd_t));

            ++li_avai;
        }
    }

    assert(li_avai <= li_ret);

    return li_avai;
}

/* ---------------------------------------------------------------------- */

