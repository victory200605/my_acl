
#include "DevpollReactor.h"
#include "AITime.h"

#define AI_POLL_INVALID_HANDLE -1

#define AI_BIT_ENABLED(WORD, BIT) (((WORD) & (BIT)) != 0)
#define AI_BIT_DISABLED(WORD, BIT) (((WORD) & (BIT)) == 0)
#define AI_BIT_CMP_MASK(WORD, BIT, MASK) (((WORD) & (BIT)) == MASK)
#define AI_SET_BITS(WORD, BITS) (WORD |= (BITS))
#define AI_CLR_BITS(WORD, BITS) (WORD &= ~(BITS))
    
void AIEventHandler::SetHandle( int aiHandle )
{
}

int AIEventHandler::GetHandle( void )
{
    return AI_POLL_INVALID_HANDLE;
}

int AIEventHandler::HandleInput( int aiHandle )
{
    return 0;
}

int AIEventHandler::HandleOutput( int aiHandle )
{
    return 0;
}

int AIEventHandler::HandleException( int aiHandle )
{
    return 0;
}

int AIEventHandler::HandleClose( int aiHandle, int aiMask )
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
int AIEventHandlerTW::HandleInput( int aiHandle )
{
    return 0;
}

int AIEventHandlerTW::HandleOutput( int aiHandle )
{
    return 0;
}
    
int AIEventHandlerTW::HandleInputTW( int aiHandle )
{
    return 0;
}

int AIEventHandlerTW::HandleOutputTW( int aiHandle )
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
void AIDevpollRepository::Bind( int aiHandle, AIEventHandler* apoEvent, int aiMask )
{
    //AISmartLock loLock( coLock );
    
    stEventNode& loEventNode = this->coHandlerRep[aiHandle];
    
    loEventNode.cpoEvent = apoEvent;
    loEventNode.ciMask   = aiMask;
    loEventNode.cbIsSuspended = false;
}

void AIDevpollRepository::Unbind( int aiHandle )
{
    //AISmartLock loLock( coLock );
    
    this->coHandlerRep.erase(aiHandle);
}
    
AIEventHandler* AIDevpollRepository::Find( int aiHandle )
{
    //AISmartLock loLock( coLock );
    
    AI_STD::hash_map<int, stEventNode>::iterator loIter = this->coHandlerRep.find(aiHandle);
        
    if ( loIter != this->coHandlerRep.end() )
    {
        return loIter->second.cpoEvent;
    }
    else
    {
        return NULL;
    }
}
    
int AIDevpollRepository::Mask( int aiHandle )
{
    //AISmartLock loLock( coLock );
    
    AI_STD::hash_map<int, stEventNode>::iterator loIter = this->coHandlerRep.find(aiHandle);
        
    if ( loIter != this->coHandlerRep.end() )
    {
        return loIter->second.ciMask;
    }
    else
    {
        return 0;
    }
}

void AIDevpollRepository::Mask( int aiHandle, int aiMask )
{
    //AISmartLock loLock( coLock );
    
    AI_STD::hash_map<int, stEventNode>::iterator loIter = this->coHandlerRep.find(aiHandle);
        
    if ( loIter != this->coHandlerRep.end() )
    {
        loIter->second.ciMask = aiMask;
    }
}

AIDevpollRepository::STATE AIDevpollRepository::HandleState( int aiHandle )
{
    AI_STD::hash_map<int, stEventNode>::iterator loIter = this->coHandlerRep.find(aiHandle);
        
    if ( loIter != this->coHandlerRep.end() )
    {
        return loIter->second.cbIsSuspended ? SUSPENDED : ACTIVED;
    }
    else
    {
        return UNEXISTED;
    }
}
    
void AIDevpollRepository::HandleState( int aiHandle, AIDevpollRepository::STATE aiState )
{
    AI_STD::hash_map<int, stEventNode>::iterator loIter = this->coHandlerRep.find(aiHandle);
        
    if ( loIter != this->coHandlerRep.end() )
    {
        if ( aiState == SUSPENDED )
        {
            loIter->second.cbIsSuspended = true;
        }
        else if ( aiState == ACTIVED )
        {
            loIter->second.cbIsSuspended = false;
        }
    }
}  

//////////////////////////////////////////////////////////////////////////////
AIDevpollReactor::AIDevpollReactor()
    : ciPollfd(AI_POLL_INVALID_HANDLE)
    , ciSize(0)
#if defined(_HAS_EVENT_POLL)
    , cpoEvents(NULL)
    , cpoStartPevents(NULL)
    , cpoEndPevents(NULL)
#else
    , cpoDPfds(NULL)
    , cpoStartPfds(NULL)
    , cpoEndPfds(NULL)
#endif
{
}
    
AIDevpollReactor::~AIDevpollReactor()
{
    this->Close();
}
    
int AIDevpollReactor::Open( int aiSize )
{
#if defined (_HAS_EVENT_POLL)
    // Allocating event table:
    AI_NEW_N_ASSERT ( this->cpoEvents, epoll_event, aiSize );
    ::memset( this->cpoEvents, 0, sizeof(epoll_event) * aiSize );

    // Initializing epoll:
    this->ciPollfd = ::epoll_create(aiSize);
#else
    // Allocate the array before opening the device to avoid a potential
    // resource leak if allocation fails.
    AI_NEW_N_ASSERT( this->cpoDPfds, pollfd, aiSize );
    ::memset( this->cpoDPfds, 0, sizeof(pollfd) * aiSize );

    // Open the `/dev/poll' character device.
    this->ciPollfd = ::open( "/dev/poll", O_RDWR );
#endif

    this->ciSize = aiSize;

    return ( this->ciPollfd == AI_POLL_INVALID_HANDLE ? -1 : 0 );
}

void AIDevpollReactor::Close( void )
{
#if defined (_HAS_EVENT_POLL)
    AI_DELETE_N( this->cpoEvents );
#else
    AI_DELETE_N( this->cpoDPfds );
#endif

    ::close( this->ciPollfd );
    this->ciPollfd  = AI_POLL_INVALID_HANDLE;
}

int AIDevpollReactor::HandleEvent( int aiTimeout )
{
    int liResult = this->WorkPending( aiTimeout );

    AI_RETURN_IF( liResult,  liResult <= 0 );

    this->Dispatch();

    return 0;
}

int AIDevpollReactor::Dispatch( void )
{
#if defined (_HAS_EVENT_POLL)
    while( this->cpoStartPevents < this->cpoEndPevents )
#else
    while( this->cpoStartPfds < this->cpoEndPfds )
#endif /* _HAS_EVENT_POLL */
    {
        this->DispatchIOEvent();
    }
    
    return 0;
}

int AIDevpollReactor::DispatchIOEvent( void )
{
    // Since the underlying event demultiplexing mechansim (`/dev/poll'
    // or '/dev/epoll') is stateful, and since only one result buffer is
    // used, all pending events (i.e. those retrieved from a previous
    // poll) must be dispatched before any additional event can be
    // polled.  As such, the Dev_Poll_Reactor keeps track of the
    // progress of events that have been dispatched.
    
    // Dispatch the events.
    //
    // Select the first available handle with event(s) pending. Check for
    // event type in defined order of dispatch: output, exception, input.
    // When an event is located, clear its bit in the dispatch set. If there
    // are no more events for the handle, also increment the pfds pointer
    // to move to the next handle ready.
    //
    // Notice that pfds only contains file descriptors that have
    // received events.
#if defined (_HAS_EVENT_POLL)
    struct epoll_event*& lpFds = this->cpoStartPevents;
    if ( lpFds < this->cpoEndPevents )
#else
    struct pollfd*& lpFds = this->cpoStartPfds;
    if ( lpFds < this->cpoEndPfds )
#endif /* _HAS_EVENT_POLL */
    {
#if defined (_HAS_EVENT_POLL)
        const int liHandle   = lpFds->data.fd;
        uint32_t& liRevents  = lpFds->events;
#else
        const int liHandle   = lpFds->fd;
        short&    liRevents  = lpFds->revents;
#endif /* ACE_HAS_EVENT_POLL */

        // Figure out what to do first in order to make it easier to manage
        // the bit twiddling and possible pfds increment before releasing
        // the token for dispatch.
        // Note that if there's an error (such as the handle was closed
        // without being removed from the event set) the EPOLLHUP and/or
        // EPOLLERR bits will be set in revents.
        bool lbIsDispOut = false;
        bool lbIsDispExc = false;
        bool lbIsDispIn  = false;
        
        if ( AI_BIT_ENABLED( liRevents, OUT_EVENT ) )
        {
            lbIsDispOut = true;
            AI_CLR_BITS( liRevents, OUT_EVENT );
        }
        else if ( AI_BIT_ENABLED( liRevents, EXC_EVENT ) )
        {
            lbIsDispExc = true;
            AI_CLR_BITS( liRevents, EXC_EVENT );
        }
        else if ( AI_BIT_ENABLED( liRevents, IN_EVENT ) )
        {
            lbIsDispIn = true;
            AI_CLR_BITS( liRevents, IN_EVENT );
        }
        else if ( AI_BIT_ENABLED( liRevents, ERR_EVENT ) )
        {
            this->RemoveHandler( liHandle, AIEventHandler::ALL_EVENTS_MASK );
            ++lpFds;
            return 1;
        }
        else
        {
            printf( "Unkown event = %0x\n", liRevents );
            assert(0);
        }
        
        // Increment the pointer to the next element before we
        // release the token.  Otherwise event handlers end up being
        // dispatched multiple times for the same poll.
        if ( liRevents == 0 )
        {
            ++lpFds;
        }
        
        /* When using sys_epoll, we can attach arbitrary user
           data to the descriptor, so it can be delivered when
           activity is detected. Perhaps we should store event
           handler together with descriptor, instead of looking
           it up in a repository ? Could it boost performance ?
        */
        AIEventHandler* lpoEh = this->coHandlerRep.Find( liHandle );

        if ( lpoEh )
        {            
            // Dispatch the detected event
            if (lbIsDispOut)
            {
                const int liStatus = this->Upcall( lpoEh, &AIEventHandler::HandleOutput, liHandle );
            
                if ( liStatus < 0 )
                {
                    // Note that the token is reacquired in remove_handler().
                    this->RemoveHandler( liHandle, AIEventHandler::WRITE_MASK );
                }
                
                return 1;
            }
            
            if (lbIsDispExc)
            {
                const int liStatus = this->Upcall( lpoEh, &AIEventHandler::HandleException, liHandle );
            
                if ( liStatus < 0 )
                {
                    // Note that the token is reacquired in remove_handler().
                    this->RemoveHandler( liHandle, AIEventHandler::EXCEPT_MASK );
                }
                
                return 1;
            }
            
            if (lbIsDispIn)
            {
                const int liStatus = this->Upcall( lpoEh, &AIEventHandler::HandleInput, liHandle );
            
                if ( liStatus < 0 )
                {
                    // Note that the token is reacquired in remove_handler().
                    this->RemoveHandler( liHandle, AIEventHandler::READ_MASK );
                }
                
                return 1;
            }
        } // The reactor token is reacquired upon leaving this scope.
    }

    return 0;
}

int AIDevpollReactor::Upcall( AIEventHandler* apoEventHandler,
                              int ( AIEventHandler::*callback )( int ),
                              int aiHandle )
{
    // If the handler returns positive value (requesting a reactor
    // callback) just call back as many times as the handler requests
    // it.  Other threads are off handling other things.
    int liStatus = 0;

    do
    {
        liStatus = ( apoEventHandler->*callback )( aiHandle );
    }
    while( liStatus > 0);

    return liStatus;
}

int AIDevpollReactor::WorkPending( int aiTimeout )
{
    int liNfds = 0;

#if defined (_HAS_EVENT_POLL)
    do
    {
        // Wait for events.
        liNfds = ::epoll_wait( 
            this->ciPollfd, this->cpoEvents, this->ciSize, aiTimeout );
    }
    while( liNfds < 0 && errno == EINTR );
    
    this->cpoStartPevents = this->cpoEvents;
    this->cpoEndPevents   = this->cpoStartPevents + ( liNfds >= 0 ? liNfds : 0 );
#else
    struct dvpoll loDvp;
    
    loDvp.dp_fds = this->cpoDPfds;
    loDvp.dp_nfds = this->ciSize;
    loDvp.dp_timeout = ( aiTimeout < 0 ? -1 : aiTimeout * 1000 );  // Milliseconds
    
    do
    {
        // Poll for events
        liNfds = ::ioctl( this->ciPollfd, DP_POLL, &loDvp );
    }
    while( liNfds < 0 && errno == EINTR );

    // Retrieve the results from the pollfd array.
    this->cpoStartPfds = loDvp.dp_fds;
    
    // If nfds == 0 then end_pfds_ == start_pfds_ meaning that there is
    // no work pending.  If nfds > 0 then there is work pending.
    // Otherwise an error occurred.
    this->cpoEndPfds   = this->cpoStartPfds + ( liNfds >= 0 ? liNfds : 0 );
#endif

    return liNfds;
}
    
int AIDevpollReactor::RegisterHandler( int aiHandle, AIEventHandler* apoEvent, int aiMask )
{
    AIEventHandler* lpoEh = this->coHandlerRep.Find(aiHandle);
    unsigned int liEvents = 0;
    
    if ( aiHandle == AI_POLL_INVALID_HANDLE
      || aiMask == AIEventHandler::NULL_MASK )
    {
        errno = EINVAL;
        return -1;
    }
    
    if ( lpoEh == NULL )
    {
        liEvents = this->ReactorMaskToPollEvent( aiMask );
        
        if ( this->AddMaskOpts( aiHandle, liEvents ) != 0 )
        {
            return -1;
        }
        this->coHandlerRep.Bind( aiHandle, apoEvent, aiMask );
    }
    else
    {
        int liNewMask = this->coHandlerRep.Mask(aiHandle);
        
        if ( AI_BIT_ENABLED( liNewMask, aiMask ) )
        {
            return 0;
        }
        
        AI_SET_BITS( liNewMask, aiMask );
        
        liEvents = this->ReactorMaskToPollEvent( liNewMask );
        
        if ( this->UpdateMaskOpts( aiHandle, liEvents ) != 0 )
        {
            return -1;
        }
        
        this->coHandlerRep.Mask(aiHandle, liNewMask );
    }

    return 0;
}
    
int AIDevpollReactor::RemoveHandler( int aiHandle, int aiMask )
{
    AIEventHandler* lpoEh = this->coHandlerRep.Find(aiHandle);
    int liOldMask = this->coHandlerRep.Mask(aiHandle);
    int liNewMask = liOldMask;
    int liEvent   = 0;

    AI_CLR_BITS( liNewMask, aiMask );
    
    liEvent = this->ReactorMaskToPollEvent(liNewMask);
    
    if ( lpoEh == NULL || this->UpdateMaskOpts( aiHandle, liEvent ) != 0 )
    {
        return -1;
    }
    
    this->coHandlerRep.Mask( aiHandle, liNewMask );

    if ( AI_BIT_DISABLED( liNewMask, AIEventHandler::DONT_CALL ) )
    {
        lpoEh->HandleClose( aiHandle, aiMask );
    }

    // If there are no longer any outstanding events on the given handle
    // then remove it from the handler repository.
    if ( liNewMask == AIEventHandler::NULL_MASK )
    {
        this->coHandlerRep.Unbind( aiHandle );
    }

    return 0; 
}
    
int AIDevpollReactor::SuspendHandler( int aiHandle )
{
    if ( this->coHandlerRep.HandleState(aiHandle) != AIDevpollRepository::ACTIVED )
    {
        return -1;
    }
    
    return this->UpdateMaskOpts( aiHandle, RM_EVENT ) != 0 ? -1 : 0;
}

int AIDevpollReactor::ResumeHandler( int aiHandle )
{
    int liMask  = 0;
    int liEvent = 0;
    
    if ( this->coHandlerRep.HandleState(aiHandle) != AIDevpollRepository::SUSPENDED )
    {
        return -1;
    }
    
    liMask  = this->coHandlerRep.Mask( aiHandle );
    liEvent = this->ReactorMaskToPollEvent( liMask );
    
    AI_RETURN_IF( -1,  liMask == AIEventHandler::NULL_MASK );

    return this->UpdateMaskOpts( aiHandle, liEvent ) != 0 ? -1 : 0;
}

int AIDevpollReactor::UpdateMaskOpts( int aiHandle, unsigned int aiEvent )
{
#if defined (_SUN)
    // Apparently events cannot be updated on-the-fly on Solaris so
    // remove the existing events, and then add the new ones.
    struct pollfd loPfd[2];
    
    loPfd[0].fd      = aiHandle;
    loPfd[0].events  = RM_EVENT;
    loPfd[0].revents = 0;
    loPfd[1].fd      = (aiEvent == RM_EVENT ? AI_POLL_INVALID_HANDLE : aiHandle);
    loPfd[1].events  = aiEvent;
    loPfd[1].revents = 0;
    
    // Change the events associated with the given file descriptor.
    if ( ::write( this->ciPollfd, loPfd, sizeof (loPfd) ) != sizeof(loPfd) )
    {
        return -1;
    }
#elif defined (_HAS_EVENT_POLL)
    struct epoll_event loEpev;
    int liOp;
      
    ::memset( &loEpev, 0, sizeof (loEpev) );

    // ACE_Event_Handler::NULL_MASK ???
    if ( aiEvent == RM_EVENT )
    {
        liOp          = EPOLL_CTL_DEL;
        loEpev.events = 0;
    }
    else
    {
        liOp          = EPOLL_CTL_MOD;
        loEpev.events = aiEvent;
    }

    loEpev.data.fd = aiHandle;

    if ( ::epoll_ctl( this->ciPollfd, liOp, aiHandle, &loEpev ) == -1 )
    {
        // If a handle is closed, epoll removes it from the poll set
        // automatically - we may not know about it yet. If that's the
        // case, a mod operation will fail with ENOENT. Retry it as
        // an add.
        if ( liOp == EPOLL_CTL_MOD && errno == ENOENT &&
             ::epoll_ctl( this->ciPollfd, EPOLL_CTL_ADD, aiHandle, &loEpev ) == -1 )
        {
            return -1;
        }
    }
#else
    struct pollfd loPfd[1];
    
    loPfd[0].fd      = aiHandle;
    loPfd[0].events  = aiEvent;
    loPfd[0].revents = 0;
    
    // Change the events associated with the given file descriptor.
    if ( ::write (this->ciPollfd, loPfd, sizeof (loPfd)) != sizeof(loPfd) )
    {
        return -1;
    }
#endif

    return 0;
}

int AIDevpollReactor::AddMaskOpts( int aiHandle, unsigned int aiEvent )
{
#if defined (_HAS_EVENT_POLL)
    struct epoll_event loEpev;
    int liOp;
      
    ::memset( &loEpev, 0, sizeof (loEpev) );

    // ACE_Event_Handler::NULL_MASK ???
    liOp           = EPOLL_CTL_ADD;
    loEpev.events  = aiEvent;
    loEpev.data.fd = aiHandle;

    if ( ::epoll_ctl( this->ciPollfd, liOp, aiHandle, &loEpev ) == -1 )
    {
        return -1;
    }
#else
    struct pollfd loPfd[1];
    
    loPfd[0].fd      = aiHandle;
    loPfd[0].events  = aiEvent;
    loPfd[0].revents = 0;
    
    // Change the events associated with the given file descriptor.
    if ( ::write (this->ciPollfd, loPfd, sizeof (loPfd)) != sizeof(loPfd) )
    {
        return -1;
    }
#endif

    return 0;
}

unsigned int AIDevpollReactor::ReactorMaskToPollEvent( int aiMask )
{
    if ( aiMask == AIEventHandler::NULL_MASK )
    {
        // No event.  Remove from interest set.
        return RM_EVENT;
    }
    
    unsigned int liEvents = 0;

    // READ, ACCEPT, and CONNECT flag will place the handle in the
    // read set.
    if ( AI_BIT_ENABLED( aiMask, AIEventHandler::READ_MASK)
      || AI_BIT_ENABLED( aiMask, AIEventHandler::ACCEPT_MASK)
      || AI_BIT_ENABLED( aiMask, AIEventHandler::CONNECT_MASK) )
    {
        AI_SET_BITS( liEvents, IN_EVENT );
    }

    // WRITE and CONNECT flag will place the handle in the write set.
    if ( AI_BIT_ENABLED( aiMask, AIEventHandler::WRITE_MASK)
      || AI_BIT_ENABLED( aiMask, AIEventHandler::CONNECT_MASK) )
    {
        AI_SET_BITS( liEvents, OUT_EVENT );
    }

    // EXCEPT flag will place the handle in the except set.
    if ( AI_BIT_ENABLED( aiMask, AIEventHandler::EXCEPT_MASK) )
    {
        AI_SET_BITS( liEvents, EXC_EVENT );
    }

    return liEvents;
}
