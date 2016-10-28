
#ifndef ANF_UTIL_CONNECTMANAGER_H
#define ANF_UTIL_CONNECTMANAGER_H

#include "acl/Synch.h"
#include "acl/ThreadManager.h"
#include "acl/SockAddr.h"
#include "acl/stl/map.h"
#include "anf/IoSockConnector.h"
#include "anf/util/SessionManager.h"

ANF_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//connect node info
struct CConnectNode
{
    std::string    moGroup;
    acl::CSockAddr moRemoteAddress;
    apl_size_t     muCurrNum;
    apl_size_t     muConnNum;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class CConnectManager : public anf::CSessionManager
{
    struct CNode
    {
        acl::CSockAddr moRemoteAddress;
        apl_size_t muConnNum;
    };

    typedef std::map<std::string, CNode> NodeMapType;

public:
    CConnectManager(void);

    virtual ~CConnectManager(void);

    apl_int_t Initialize( anf::CIoConnector* apoConnector, acl::CTimeValue const& aoInterval );
    
    void Close(void);

    apl_ssize_t Connect( 
        char const* apcName, 
        acl::CSockAddr const& aoRemoteAddress, 
        apl_size_t auConnNum, 
        bool abIsWait = true );
    
    void SetForceCloseOnResize( bool abIsEnable );

    bool IsForceCloseOnResize(void);

//Hook event method
public:
    //Connection changed event, when connection count have been changed, and HandleChanged will be called
    virtual void HandleChanged( 
        char const* apcName, 
        acl::CSockAddr const& aoRemoteAddress,
        apl_size_t auCurrNum, 
        apl_size_t auNextNum );

    //Session connected event, when session connected, and HandleConnected will be called
    virtual void HandleConnected( anf::SessionPtrType& aoSession );
    
    //Session closed event, when session closed, and HandleClosed will be called
    virtual void HandleClosed( anf::SessionPtrType& aoSession );

    //Session connecting Exception event, when session connect fail, and HandleConnectException will be called
    virtual void HandleConnectException( 
        char const* apcName, 
        acl::CSockAddr const& aoRemoteAddress, 
        apl_int_t aiState, 
        apl_int_t aiErrno );

protected:
    static void* Svc(void* apvParam);

    apl_ssize_t Connect( std::vector<CConnectNode>& aoConnectNodes );

    void Remove( char const* apcName, apl_size_t auNum );

    void ClearAllClosedSession(void);

private:
    anf::CIoConnector* mpoConnector;
    
    acl::CTimeValue moInterval;

    bool mbIsClosing;

    bool mbIsForceCloseOnResize;

    acl::CThreadManager moThreadManager;

    acl::CLock  moLock;

    NodeMapType moNodes;
};

ANF_NAMESPACE_END

#endif//ANF_UTIL_CONNECTMANAGER_H

