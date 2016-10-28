
#ifndef AAF_TRAPSERVER_H
#define AAF_TRAPSERVER_H

#include "aaf/Server.h"
#include "acl/Reactor.h"
#include "acl/Synch.h"
#include "acl/SockHandle.h"
#include "acl/Singleton.h"
#include "acl/ThreadManager.h"
#include "acl/SockStream.h"

AAF_NAMESPACE_START

class CTrapServer : public aaf::IServer, public acl::IEventHandler
{
    struct CTrap
    {
        char macTitle[128];
        char macContent[512];
    };
    
public:
    CTrapServer(void);
    
    virtual ~CTrapServer(void);
    
    virtual apl_int_t Startup( char const* apcConfigFileName );
    
    virtual apl_int_t Restart(void);
    
    virtual apl_int_t Shutdown(void);
    
    apl_int_t Trap( char const* apcTitle, char const* apcFormat, ... );
        
    apl_int_t TrapNonBlock( char const* apcTitle, char const* apcFormat, ... );
    
    void SetReactor( acl::CReactor* apoReactor );

protected:
    static void* Svc(void*);
    
    apl_int_t ConnectToServer(void);
    
    void CheckConnection(void);

    virtual apl_int_t HandleOutput( apl_handle_t aiHandle );

private:
    bool mbIsShutdown;
    
    acl::CReactor* mpoReactor;
    apl_int_t      miEvents;
    
    acl::CSockHandlePair moHandlePair;
        
    acl::CThreadManager moThreadManager;
    
    acl::CMemoryBlock moBuffer;
    
    //trap server table
    apl_uint32_t muCommandID;
    
    apl_uint32_t muInvokeID;
    
    std::string moServerIp;
    
    apl_uint16_t mu16ServerPort;
    
    std::string moTableName;
    
    std::string moProgramName;
    
    std::string moType;
    
    std::string moLevel;
    
    std::string moCounter;
    
    std::string moDelResult;
    
    std::string moWarningStat;
    
    acl::CSockStream moPeer;
        
    acl::CLock moLock;
};

AAF_NAMESPACE_END

//////////////////////////////////////////////////////////////////////////////
//non-block
#define AAF_TRAP_NB( title, ... ) \
    acl::TSingleton<aaf::CTrapServer>::Instance()->TrapNonBlock(title, __VA_ARGS__);

#define AAF_TRAP( title, ... ) \
    acl::TSingleton<aaf::CTrapServer>::Instance()->Trap(title, __VA_ARGS__);

#endif//AAF_TRAPSERVER_H
