/**
 * @file poll.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.17 $
 */
#ifndef APL_POLL_H
#define APL_POLL_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include "apl/time.h"

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

#if defined(HAVE_SYS_EPOLL_H) /* LINUX: using epoll  */

#   include <sys/epoll.h> 

#   define  apl_pollfd_t    epoll_event

#   define  APL_POLLFD_GET_HANDLE(pfd)    ((apl_handle_t) (pfd)->data.fd)
#   define  APL_POLLFD_GET_EVENTS(pfd)    ((apl_int_t) (pfd)->events)

#   define  APL_POLLIN      (EPOLLIN)
#   define  APL_POLLOUT     (EPOLLOUT)
#   define  APL_POLLPRI     (EPOLLPRI)
#   if defined(EPOLLRDHUP)
#       define  APL_POLLHUP (EPOLLHUP|EPOLLRDHUP)
#   else
#       define  APL_POLLHUP (EPOLLHUP)
#   endif
#   define  APL_POLLERR     (EPOLLERR)

struct apl_pollset_t
{
    apl_handle_t            mi_epfd;
};


#elif defined(HAVE_SYS_DEVPOLL_H) /* using devpoll */

#   include <poll.h>
#   include <sys/devpoll.h> 

#   define  apl_pollfd_t    pollfd 

#   define  APL_POLLFD_GET_HANDLE(pfd)    ((apl_handle_t) (pfd)->fd)
#   define  APL_POLLFD_GET_EVENTS(pfd)    ((apl_int_t) (pfd)->revents)

#   define  APL_POLLIN      (POLLIN)
#   define  APL_POLLOUT     (POLLOUT)
#   define  APL_POLLPRI     (POLLPRI)
#   define  APL_POLLHUP     (POLLHUP)
#   define  APL_POLLERR     (POLLERR|POLLNVAL)

struct apl_pollset_t
{
    apl_handle_t            mi_dpfd;
};

#elif defined(HAVE_SYS_POLLSET_H) /* AIX: using pollset */

#   include <poll.h>
#   include <sys/pollset.h>
#   include <fcntl.h>

#   define  apl_pollfd_t    pollfd 

#   define  APL_POLLFD_GET_HANDLE(pfd)    ((apl_handle_t) (pfd)->fd)
#   define  APL_POLLFD_GET_EVENTS(pfd)    ((apl_int_t) (pfd)->revents)

#   define  APL_POLLIN      (POLLIN)
#   define  APL_POLLOUT     (POLLOUT)
#   define  APL_POLLPRI     (POLLPRI)
#   define  APL_POLLHUP     (POLLHUP)
#   define  APL_POLLERR     (POLLERR|POLLNVAL)

struct apl_pollset_t
{
    pollset_t              mi_pollset;
};

#else /* POSIX: using poll */

#   include <poll.h> 

#   define  apl_pollfd_t    pollfd 

#   define  APL_POLLFD_GET_HANDLE(pfd)    ((apl_handle_t) (pfd)->fd)
#   define  APL_POLLFD_GET_EVENTS(pfd)    ((apl_int_t) (pfd)->revents)

#   define  APL_POLLIN      (POLLIN)
#   define  APL_POLLOUT     (POLLOUT)
#   define  APL_POLLPRI     (POLLPRI)
#   define  APL_POLLHUP     (POLLHUP)
#   define  APL_POLLERR     (POLLERR|POLLNVAL)

struct apl_pollset_t
{
    struct apl_pollfd_t*    mps_pfds;
    apl_size_t              mu_max;
    apl_size_t              mu_cur;
};

#endif


/**
 * The structure to store poll results.
 *
 * @struct apl_pollfd_t
 */

/**
 * Get file handle from apl_pollfd_t structure.
 *
 * @def APL_POLLFD_GET_HANDLE(pfd)
 *
 * @param[in] pfd Pointer to apl_pollfd_t structure
 *
 * @return File handle associates with the pfd
 */

/**
 * Get pending events from apl_pollfd_t structure.
 *
 * @def APL_POLLFD_GET_EVENTS(pfd)
 *
 * @param[in] pfd Pointer to apl_pollfd_t structure
 *
 * @return Pending events
 */

/**
 * The readable events.
 *
 * @def APL_POLLIN
 */

/**
 * The readable events.
 *
 * @def APL_POLLOUT
 */

/**
 * The priority readable events
 *
 * @def APL_POLLPRI
 */

/**
 * The disconnected events.
 *
 * @def APL_POLLHUP
 */

/**
 * Pending errors.
 *
 * @def APL_POLLERR
 */

/**
 * Structure to maintain a set of file handles and their associated events.
 *
 * @struct apl_pollset_t
 */


/**
 * Initialize an apl_pollset_t structure.
 *
 * @param aps_pollset Pointer to apl_pollset_t structure
 * @param ai_maxfd Max number of file handles that pollset can hold (0 ~ ai_maxfd-1)
 *
 * @retval 0 Success
 * @retval -1 Error
 */
apl_int_t apl_pollset_init(
    struct apl_pollset_t*   aps_pollset, 
    apl_size_t              ai_maxfd);

/**
 * Destroy an initialized apl_pollset_t structure.
 *
 * @param aps_pollset Pointer to apl_pollset_t structure
 *
 * @retval 0 Success
 * @retval -1 Error
 */
apl_int_t apl_pollset_destroy(
    struct apl_pollset_t*   aps_pollset);

/**
 * Add the file handle to the pollset.
 *
 * @param[in,out] aps_pollset Pointer to the pollset
 * @param[in] ai_fildes The file handle to be added
 * @param[in] ai_events The events to set
 *
 * @retval 0 Success
 * @retval -1 Errors
 */
apl_int_t apl_pollset_add_handle(
    struct apl_pollset_t*   aps_pollset, 
    apl_handle_t            ai_fildes,
    apl_int_t               ai_events);

/**
 * Remove the file handle from the pollset
 *
 * @param[in,out] aps_pollset Pointer to the pollset
 * @param[in] ai_fildes The file handle to be removed
 *
 * @retval 0 Success
 * @retval -1 Errors
 */
apl_int_t apl_pollset_del_handle(
    struct apl_pollset_t*   aps_pollset, 
    apl_handle_t            ai_fildes);

/**
 * Set the monitored events of the file handle in the pollset
 *
 * @param[in,out] aps_pollset Pointer to the pollset
 * @param[in] ai_fildes The file handle
 * @param[in] ai_events The monitored events to be associates with the file handle
 *
 * @retval 0 Success
 * @retval -1 Errors
 */
apl_int_t apl_pollset_set_events(
    struct apl_pollset_t*   aps_pollset, 
    apl_handle_t            ai_fildes, 
    apl_int_t               ai_events);

/**
 * Wait for any pending events of all file handles in the pollset.
 *
 * @param[in,out] aps_pollset Pointer to the pollset
 * @param[out] aps_outfds Array of apl_pollfd_t to store the poll result
 * @param[in] ai_outfdslen Array size of aps_outfds
 * @param[in] ai64_timeout Max block time of the apl_poll operate:
 *                          < 0, block forever;
 *                         == 0, return at once;
 *                          > 0, block no more than the time interval specified by ai64_timeout.
 * @retval 0 Success
 * @retval -1 Errors
 */
apl_int_t apl_poll(
    struct apl_pollset_t*   aps_pollset,
    struct apl_pollfd_t*    aps_outfds,
    apl_size_t              ai_outfdslen,
    apl_time_t              ai64_timeout);

/**
 * Wait for any pending events of the file handle
 *
 * @param[in] ai_fildes File handle to wait
 * @param[in] ai_events The monitored events
 * @param[out] api_revents The pending events that returned
 * @param[in] ai64_timeout Same as ai64_timeout in apl_poll
 *
 * @retval 0 Success
 * @retval -1 Errors
 */
apl_int_t apl_poll_one(
    apl_handle_t            ai_fildes,
    apl_int_t               ai_events,
    apl_int_t*              api_revents,
    apl_time_t              ai64_timeout);


/* ------------------------------ */  APL_DECLARE_END  /* ------------------------------ */

#endif /* APL_POLL_H */

