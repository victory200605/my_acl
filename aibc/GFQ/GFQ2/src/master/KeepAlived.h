
#ifndef AIBC_GFQ_KEEPALIVED_H
#define AIBC_GFQ_KEEPALIVED_H

#include "gfq2/Utility.h"
#include "acl/stl/string.h"
#include "acl/SockDgramMcast.h"
#include "acl/ThreadManager.h"
#include "aaf/DaemonService.h"

AIBC_GFQ_NAMESPACE_START

class CKeepAlived : public aaf::CDaemonService
{
public:
    CKeepAlived(void);

    ~CKeepAlived(void);

    apl_int_t Startup( bool abIsEnableKeepAlived, char const* apcConfigCommand );

    virtual apl_int_t Initialize(void);

    virtual void Finalize(void);

protected:
    bool IsAlived( acl::CSockAddr const& aoGroupAddr );

    void RequestHandle( acl::CTimeValue const& aoTimedout );

    static void* Srv( void* apvParam );

private:
    bool mbIsEnableKeepAlived;

    bool mbIsClosed;

    bool mbIsActive;

    std::string moConfigCommand;

    std::string moLocalAddress;
    
    apl_int_t   miLocalPort;
    
    apl_int_t   miTimedout;
    
    apl_int_t   miRetryTimes;

    acl::CSockAddr moMcastAddr;
    
    acl::CSockDgram moWatcher;
    
    acl::CSockDgramMcast moMcast;

    acl::CThreadManager moThreadManager;
};

AIBC_GFQ_NAMESPACE_END

#endif//AIBC_GFQ_KEEPALIVED_H

