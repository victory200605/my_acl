
#ifndef __AILIB_DEVPOLLREACTOR_H__
#define __AILIB_DEVPOLLREACTOR_H__

#include <stdlib.h>
#include "AILib.h"
#include "AISynch.h"
#include "stl/hash_map.h"

#if defined(_HAS_EVENT_POLL) && defined(_LINUX)
#   include<sys/epoll.h>
#else
#   if defined(_LINUX)
#       include<linux/devpoll.h>
#   else
#       include<sys/devpoll.h>
#   endif  /* linux */
#endif  /* _HAS_DEV_POLL */

#define AI_POLL_DEFAULT_TIMEOUT 5

class AIEventHandler
{
public:
    enum MASK
    {
        LO_PRIORITY = 0,
        HI_PRIORITY = 10,
        NULL_MASK = 0,
        READ_MASK = (1 << 0),
        WRITE_MASK = (1 << 1),
        EXCEPT_MASK = (1 << 2),
        ACCEPT_MASK = (1 << 3),
        CONNECT_MASK = (1 << 4),
        TIMER_MASK = (1 << 5),
        SIGNAL_MASK = (1 << 6),
        ALL_EVENTS_MASK = READ_MASK |
                          WRITE_MASK |
                          EXCEPT_MASK |
                          ACCEPT_MASK |
                          CONNECT_MASK |
                          TIMER_MASK |
                          SIGNAL_MASK,
        RWE_MASK = READ_MASK |
                   WRITE_MASK |
                   EXCEPT_MASK,
        DONT_CALL = (1 << 7)
    };
  
public:
    virtual void SetHandle( int aiHandle );
    virtual int GetHandle( void );

    virtual int HandleInput( int aiHandle );
    virtual int HandleOutput( int aiHandle );
    virtual int HandleException( int aiHandle );
    virtual int HandleClose( int aiHandle, int aiMask );
};

//////////////////////////////////////////////////////////////////////////////
class AIEventHandlerTW : public AIEventHandler//transfer window
{
public:
    virtual int HandleInput( int aiHandle );
    virtual int HandleOutput( int aiHandle );
    
    virtual int HandleInputTW( int aiHandle );
    virtual int HandleOutputTW( int aiHandle );

protected:
    size_t GetSize( int aiHandle );
    
    char*  GetBasePtr( int aiHandle );
    
    bool ReadFrom( int aiHandle, char* apcPtr, size_t aiSize );
    
    bool WriteTo( int aiHandle, const char* apcPtr, size_t aiSize );
};

//////////////////////////////////////////////////////////////////////////////
class AIDevpollRepository
{
    struct stEventNode
    {
        stEventNode( AIEventHandler* apoEvent = NULL, int aiMask = 0 ) 
            : ciMask(aiMask)
            , cbIsSuspended(false)
            , cpoEvent(apoEvent)
        {
        }
        
        int ciMask;
        bool cbIsSuspended;
        AIEventHandler* cpoEvent;
    };
    
public:
    enum STATE { UNEXISTED = 0, ACTIVED = 1, SUSPENDED = 2 };
    
public:
    void Bind( int aiHandle, AIEventHandler* apoEvent, int aiMask );
    
    void Unbind( int aiHandle );
    
    AIEventHandler* Find( int aiHandle );
    
    int Mask( int aiHandle );
    
    void Mask( int aiHandle, int aiMask );
    
    STATE HandleState( int aiHandle );
    
    void HandleState( int aiHandle, STATE aiState );
    
protected:
    AIMutexLock coLock;
    AI_STD::hash_map<int, stEventNode> coHandlerRep;
};

//////////////////////////////////////////////////////////////////////////////////
class AIDevpollReactor
{
public:
    enum EVENT {
#if defined (_HAS_EVENT_POLL)
        OUT_EVENT = EPOLLOUT,
        EXC_EVENT = EPOLLPRI,
        IN_EVENT  = EPOLLIN,
        RM_EVENT  = EPOLL_CTL_DEL,
        ERR_EVENT = EPOLLHUP | EPOLLERR
#else
        OUT_EVENT = POLLOUT,
        EXC_EVENT = POLLPRI,
        IN_EVENT  = POLLIN,
        RM_EVENT  = POLLREMOVE,
        ERR_EVENT = POLLHUP | POLLNVAL | POLLERR
#endif /* _HAS_EVENT_POLL */
    };
    
public:
    AIDevpollReactor();
    
    ~AIDevpollReactor();
    
    int Open( int aiSize );
    
    void Close( void );
    
    int HandleEvent( int aiTimeout = AI_POLL_DEFAULT_TIMEOUT );
    
    int Dispatch( void );
    
    int WorkPending( int aiTimeout );
    
    int RegisterHandler( int aiHandle, AIEventHandler* apoEvent, int aiMask );
    
    int RemoveHandler( int aiHandle, int aiMask );
    
    int SuspendHandler( int aiHandle );
    
    int ResumeHandler( int aiHandle );

protected:
    int DispatchIOEvent( void );
    
    int UpdateMaskOpts( int aiHandle, unsigned int aiEvent );
    int AddMaskOpts( int aiHandle, unsigned int aiEvent );
    
    int Upcall( AIEventHandler* apoEventHandler,
                int ( AIEventHandler::*callback )( int ),
                int aiHandle );
                
    unsigned int ReactorMaskToPollEvent( int aiMask );

protected:
    int ciPollfd;
    
    int ciSize;
    
    AIDevpollRepository coHandlerRep;

#if defined (_HAS_EVENT_POLL)
    /// Table of event structures to be filled by epoll_wait:
    struct epoll_event* cpoEvents; 
                                 
    /// Pointer to the next epoll_event array element that contains the next
    /// event to be dispatched.  
    struct epoll_event* cpoStartPevents;
                                 
    /// The last element in the event array plus one.
    /**                          
     * The loop that dispatches IO events stops when this->cpoStartPevents ==
     * this->cpoEndPevents.       
     */                          
    struct epoll_event* cpoEndPevents;

#else
    /// The pollfd array that `/dev/poll' will feed its results to.
    struct pollfd* cpoDPfds;

    /// Pointer to the next pollfd array element that contains the next
    /// event to be dispatched.
    struct pollfd* cpoStartPfds;
    
    /// The last element in the pollfd array plus one.
    /**
     * The loop that dispatches IO events stops when this->cpoStartPfds ==
     * this->cpoEndPfds.
     */
    struct pollfd* cpoEndPfds;
#endif  /* AI_HAS_EVENT_POLL */

};

#endif //__AILIB_DEVPOLLREACTOR_H__
