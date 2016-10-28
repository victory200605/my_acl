#ifndef COUNTSERVER_CONFIGURESERVER_H
#define COUNTSERVER_CONFIGURESERVER_H

#include "acl/Utility.h"
#include "acl/stl/vector.h"
#include "acl/Reactor.h"
#include "acl/SockHandle.h"
#include "acl/SockAcceptor.h"
#include "acl/Thread.h"
#include "acl/IniConfig.h"

#include "Table.h"
#include "LogServer.h"

#include "lber.h"
#include "ldap.h"

////////////////////////////////////////// protocol handler ////////////////////////////////////////////
class IRequestHandler
{
public:
    virtual ~IRequestHandler(){};
    virtual apl_int_t RecvReloadRequest( acl::CSockStream& aoPeer ) = 0;
    virtual apl_int_t SendReloadResponse( acl::CSockStream& aoPeer ) = 0;
};

class CIsmg50RequestHandler : public IRequestHandler
{
public:
    CIsmg50RequestHandler(void);
    
    void SetLoginPasswd( char const* apcPasswd );
    
    virtual apl_int_t RecvReloadRequest( acl::CSockStream& aoPeer );
    
    virtual apl_int_t SendReloadResponse( acl::CSockStream& aoPeer );

private:
    char macLoginPasswd[32];
};

class CIsmg35RequestHandler : public IRequestHandler
{
public:
    CIsmg35RequestHandler(void);
    
    virtual apl_int_t RecvReloadRequest( acl::CSockStream& aoPeer );
    
    virtual apl_int_t SendReloadResponse( acl::CSockStream& aoPeer );

private:
    char macPtlBuffer[12];
};
/////////////////////////////////////////////// end protocol handler ////////////////////////////////////////

class CConfigureServer : public acl::IEventHandler
{
public:
    struct CTableNode
    {
        char macTable[128];
        char macKey[128];
        char macKeySuffix[64];
        char macMaxConnection[128];
        char macMaxSpeed[128];
        apl_int_t miSpeedBucketMultipleOf;
    };
    
    typedef std::vector<CTableNode> TableListType;

public:
    static CConfigureServer* Instance();

    static void Release();
    
    virtual ~CConfigureServer();

    apl_int_t Startup(acl::CReactor& aoReactor, const char* apcIniFileName);

    void Close();

    const char* GetIpAddress();

    apl_uint16_t GetServerPort();

    apl_uint16_t GetAdminPort();

    const char* GetAdminLoginPasswd();

    apl_time_t GetLogMaxTime();

    apl_size_t GetLogMaxSize();

    apl_size_t GetLogCacheSize();
    
    apl_int_t GetRequestNumLimit(void);

protected:
    CConfigureServer();

    apl_int_t LoadIniConfig(const char* apcIniFileName);
    
    apl_int_t LoadUUMConfig( bool abIsCommit );
    
    apl_int_t LDAPInitialize( LDAP** appoLDAP );
    apl_int_t LDAPLoadTable( LDAP* apoLDAP, CTableNode &aoUUMItem );
    void      LDAPClose( LDAP** appoLDAP );
    
    apl_int_t RequestCommit(void);
    
    virtual apl_int_t HandleInput( apl_handle_t aiHandle );
    
    virtual apl_int_t HandleOutput( apl_handle_t aiHandle );

    static void* Svc(void* apvParam);

private:
    static CConfigureServer *cpoInstance;

    IRequestHandler* mpoRequestHandler;

    //UUM
    apl_int_t miIsmgNo;
    char macUUMIp[INET_ADDRSTRLEN];
    apl_uint16_t mu16UUMPort;
    char macBaseDN[256];
    TableListType moTables;

    // Log
    apl_time_t mi64MaxTime;
    apl_size_t muMaxSize;
    apl_size_t muCacheSize;

    //CountServer
    char macIpAddress[INET_ADDRSTRLEN];
    apl_uint16_t mu16ServerPort;
    apl_uint16_t mu16AdminPort;
    char macAdminLoginPasswd[32];
    apl_int_t miRequestNumLimit;

    //Other
    acl::CSockAcceptor moSockAcceptor;
    acl::CThreadManager moThreadManager;
    bool mbIsShutdown;
    
    //Reactor server
    acl::CReactor* mpoReactor;
    acl::CSockHandlePair moHandlePair;
};

#endif//COUNTSERVER_CONFIGURESERVER_H

