
#ifndef COUNTSERVER_H
#define COUNTSERVER_H

#include "../protocol/CountServerProtocol.h"
#include "../include/CountServerError.h"
#include "acl/Reactor.h"
#include "acl/SockAcceptor.h"
#include "acl/MemoryBlock.h"

class CCountServer : public acl::IEventHandler
{
    friend class CCountServerHandler;
    
public:
    static CCountServer* Instance(void);
    
    static void Release(void);
    
    virtual ~CCountServer(void);
    
    apl_int_t Startup( acl::CReactor& aoReactor, char const* apcIpAddress, apl_uint16_t au16Port );
    
    void Close(void);
    
    void SetRequestNumLimit( apl_int_t aiRequestNumLimit );

protected:
    void InsertHandler( acl::IEventHandler* apoHandler );
    void RemoveHandler( acl::IEventHandler* apoHandler );
    void RemoveAll(void);

protected:
    static CCountServer* cpoInstance;

protected:
    CCountServer(void);
    
    virtual apl_int_t HandleInput( apl_handle_t aiHandle );
    
private:
    acl::CSockAcceptor moAcceptor;
    acl::CReactor*     mpoReactor;
    apl_int_t          miRequestNumLimit;
    
    std::map<acl::IEventHandler*, apl_int_t> moConnHandlers;
};

class CCountServerHandler : public acl::IEventHandler
{
public:
    typedef std::map<apl_int_t, apl_int_t> ConnMapType;
        
public:
    CCountServerHandler( acl::CReactor& aoReactor, apl_size_t auBuffSize, apl_int_t aiRequestNumLimit );
    
    virtual ~CCountServerHandler(void);

protected:
    apl_ssize_t DispatchRequest( apl_handle_t aiHandle );
    
    apl_int_t RequestEntry( CCSPRequest& aoRequest, CCSPResponse& aoResponse );
    
    apl_int_t CountRequest( char const* apcKey, apl_int32_t ai32Num, apl_int32_t ai32LastNum );
    
    apl_int_t SpeedRequest( char const* apcKey, apl_int32_t ai32Num, acl::CTimestamp& aoTimestamp );

protected:
    virtual apl_int_t HandleInput( apl_handle_t aiHandle );
    
    virtual apl_int_t HandleOutput( apl_handle_t aiHandle );
    
    virtual apl_int_t HandleClose( apl_handle_t aiHandle, apl_int_t aiMask );
    
private:
    acl::CMemoryBlock moRecvBuff;
    
    acl::CMemoryBlock moSendBuff;
        
    acl::CReactor&    moReactor;
    apl_int_t         miEvents;
    
    ConnMapType moConnMap;
    
    apl_int_t miRequestNumLimit;
};

#endif// COUNTSERVER_H
