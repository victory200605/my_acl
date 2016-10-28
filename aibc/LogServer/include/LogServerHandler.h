#ifndef LOG_SERVER_HANDLER_H
#define LOG_SERVER_HANDLER_H

#include "LogServerProtocol.h"
#include "acl/Reactor.h"
#include "acl/SockAcceptor.h"
#include "acl/MemoryBlock.h"





class CLogServerBase : public acl::IEventHandler
{
public:
    static void Release(void);
    
    CLogServerBase(void);
    
    virtual ~CLogServerBase(void);
    
    apl_int_t Startup( acl::CReactor& aoReactor, char const* apcIpAddress, apl_uint16_t au16Port );
    
    void Close(void);
    
    void SetRequestNumLimit( apl_int_t aiRequestNumLimit );

protected:
    void InsertHandler( acl::IEventHandler* apoHandler );
    void RemoveHandler( acl::IEventHandler* apoHandler );
    void RemoveAll(void);
    virtual apl_int_t HandleInput( apl_handle_t aiHandle );
    
private:
    friend class CLogServerHandler;
    
    acl::CSockAcceptor moAcceptor;
    acl::CReactor*     mpoReactor;
    apl_int_t          miRequestNumLimit;
    
    std::map<acl::IEventHandler*, apl_int_t> moConnHandlers;
};



///////////////////////////////////////////////////////////////////////////////





class CLogServerHandler : public acl::IEventHandler
{
public:
    CLogServerHandler( acl::CReactor& aoReactor, apl_size_t auBuffSize, apl_int_t aiRequestNumLimit );
    
    virtual ~CLogServerHandler(void);

protected:
    apl_ssize_t DispatchRequest( apl_handle_t aiHandle, apl_ssize_t auRecvLen );
    
    void RequestEntry(CLogServerProtocol& aoRequest, CLogServerProtocol& aoResponse);

protected:
    virtual apl_int_t HandleInput( apl_handle_t aiHandle );
    
    virtual apl_int_t HandleOutput( apl_handle_t aiHandle );
    
    virtual apl_int_t HandleClose( apl_handle_t aiHandle, apl_int_t aiMask );
    
private:
    acl::CMemoryBlock moRecvBuff;
    
    acl::CMemoryBlock moSendBuff;
        
    acl::CReactor&    moReactor;
    apl_int_t         miEvents;
    
    apl_int_t miRequestNumLimit;
};

#endif// LOG_SERVER_HANDLER_H
