
#ifndef AIBC_GFQ_MASTERCONNECTMANAGER_H
#define AIBC_GFQ_MASTERCONNECTMANAGER_H

#include "gfq2/Utility.h"
#include "anf/util/ConnectManager.h"

AIBC_GFQ_NAMESPACE_START

class CMasterConnectManager: public anf::CConnectManager 
{
public:
    CMasterConnectManager(void);

    apl_int_t Connect( acl::CSockAddr const& aoRemoteAddress );

    apl_int_t GetRemoteAddress( acl::CSockAddr* apoRemoteAddress, apl_int_t aiTryTimes );

    virtual void HandleConnectException(
        char const* apcName,
        acl::CSockAddr const& aoRemoteAddress,
        apl_int_t aiState,
        apl_int_t aiErrno );

protected:
    bool mbIsMcastAddr;
    
    acl::CSockDgram moDgram;
    
    acl::CSockAddr moRemoteAddress;
};

AIBC_GFQ_NAMESPACE_END

#endif//AIBC_GFQ_MASTERCONNECTMANAGER_H

