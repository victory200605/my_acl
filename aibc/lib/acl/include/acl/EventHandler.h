
#ifndef ACL_EVENTHANDLER_H
#define ACL_EVENTHANDLER_H

#include "acl/Utility.h"
#include "acl/Timestamp.h"
#include "acl/IOHandle.h"

ACL_NAMESPACE_START

/////////////////////////////////////////// event handler ////////////////////////////////////////////
class IEventHandler
{
public:
    enum EMask
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
    virtual ~IEventHandler() {}
    
    virtual void SetHandle( apl_handle_t aiHandle );
    
    virtual apl_handle_t GetHandle(void);

    virtual apl_int_t HandleInput( apl_handle_t aiHandle );
    
    virtual apl_int_t HandleOutput( apl_handle_t aiHandle );
    
    virtual apl_int_t HandleException( apl_handle_t aiHandle );
    
    virtual apl_int_t HandleTimeout( CTimestamp const& aoCurrentTime, void const* apvAct = NULL );
    
    virtual apl_int_t HandleClose( apl_handle_t aiHandle, apl_int_t aiMask );
};

///////////////////////////////////////////// inline implement ////////////////////////////////
inline void IEventHandler::SetHandle( apl_handle_t aiHandle )
{
}

inline apl_handle_t IEventHandler::GetHandle(void)
{
    return ACL_INVALID_HANDLE;
}

inline apl_int_t IEventHandler::HandleInput( apl_handle_t aiHandle )
{
    return 0;
}

inline apl_int_t IEventHandler::HandleOutput( apl_handle_t aiHandle )
{
    return 0;
}

inline apl_int_t IEventHandler::HandleException( apl_handle_t aiHandle )
{
    return 0;
}

inline apl_int_t IEventHandler::HandleTimeout( CTimestamp const& aoCurrentTime, void const* apvAct )
{
    return 0;
}

inline apl_int_t IEventHandler::HandleClose( apl_handle_t aiHandle, apl_int_t aiMask )
{
    return 0;
}

ACL_NAMESPACE_END

#endif//ACL_EVENTHANDLER_H
