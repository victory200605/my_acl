
#ifndef __AIPROXY_SCPROUTING_H__
#define __AIPROXY_SCPROUTING_H__

#include "../../AIProxyRouter.h"

class clsOuterRouting : public AIProxy::AIRouting
{
public:
    virtual int Error( int aiErrno, const char* apcSender, const char* apcData, size_t aiSize );
    virtual int Route( const char* apcData, size_t aiSize, char* apcAddressee, size_t aiBufferSize );
};

#endif //__AIPROXY_SCPROUTING_H__
