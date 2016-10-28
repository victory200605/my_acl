
#ifndef AAF_CONTROLSERVER_H
#define AAF_CONTROLSERVER_H

#include "acl/SockAcceptor.h"
#include "acl/SockStream.h"
#include "acl/MemoryBlock.h"
#include "acl/SString.h"
#include "acl/ThreadManager.h"
#include "aaf/Server.h"

AAF_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////////////
class IControlProtocol
{
public:
    virtual ~IControlProtocol(void) {}
    
    virtual apl_int_t Login( 
        acl::CSockStream& aoPeer,
        char const* apcUser,
        char const* apcPasswd,
        acl::CTimeValue const& aoTimeout ) = 0;
    
    virtual apl_int_t Recv( acl::CSockStream& aoPeer, acl::CTimeValue const& aoTimeout ) = 0;
    
    virtual apl_int_t Send( acl::CSockStream& aoPeer, acl::CTimeValue const& aoTimeout ) = 0;
    
    virtual apl_int_t Logout( acl::CSockStream& aoPeer ) = 0;
};

//////////////////////////////////////////////////////////////////////////////////
class CCmdLineControlProtocol : public IControlProtocol
{
public:
    CCmdLineControlProtocol(void);
    
    virtual ~CCmdLineControlProtocol(void);
    
    virtual apl_int_t Login( 
        acl::CSockStream& aoPeer,
        char const* apcUser,
        char const* apcPasswd,
        acl::CTimeValue const& aoTimeout );
        
    virtual apl_int_t Recv( acl::CSockStream& aoPeer, acl::CTimeValue const& aoTimeout );
    
    virtual apl_int_t Send( acl::CSockStream& aoPeer, acl::CTimeValue const& aoTimeout );
    
    virtual apl_int_t Logout( acl::CSockStream& aoPeer );
        
    void Response( char const* apcFormat, ... );
    
    apl_size_t GetArgc(void);
    
    char const* GetArgv( apl_size_t aiN );

private:
    acl::CMemoryBlock moRecvBuffer;
    acl::CMemoryBlock moSendBuffer;
    acl::CTokenizer   moTokenizer;
};

//////////////////////////////////////////////////////////////////////////////////
class CControlServer : public IServer
{
public:
    typedef CCmdLineControlProtocol DefaultProtocolType;
    
public:
    CControlServer(void);
    
    virtual ~CControlServer(void);
    
    virtual apl_int_t Startup( char const* apcConfigFileName );
    
    virtual apl_int_t Restart(void);
    
    virtual apl_int_t Shutdown(void);
    
    void SetProtocol( IControlProtocol* apoProtocol );
    
    virtual apl_int_t HandleAccept( acl::CSockStream& aoPeer );
    
    virtual apl_int_t HandleInput( IControlProtocol* apoProtocol );
    
    virtual apl_int_t HandleError( acl::CSockStream& aoPeer, apl_int_t aiErrno );
    
    virtual apl_int_t HandleClose( acl::CSockStream& aoPeer );
    
    bool IsShutdown(void);
    
    void UpdateShutdown( bool abIsShutdown );

protected:
    static void* Svc(void* apvParam);
    
private:
    bool mbIsShutdown;
    
    IControlProtocol* mpoProtocol;
    
    char macIpAddress[30];
    
    char macUser[64];
    
    char macPasswd[128];
    
    apl_int_t miPort;
    
    acl::CTimeValue moTimeout;
    
    acl::CSockAcceptor moAcceptor;
    
    acl::CThreadManager moThreadManager;
};

AAF_NAMESPACE_END

#endif//AAF_CONTROLSERVER_H
