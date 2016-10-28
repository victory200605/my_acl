
#include "../../AIProxyServer.h"
#include "ScpRouting.h"

int clsOuterRouting::Error( int aiErrno, const char* apcSender, const char* apcData, size_t aiSize )
{
    AI_PROXY_ERROR( "Output Route data fail, [Recode=%d]", aiErrno );
    //AIProxy::AIServer* lpoServer = AIProxy::GetInnerServer();
    //lpoServer->Send( apcSender, apcData, aiSize );
    return 0;
}

int clsOuterRouting::Route( const char* apcData, size_t aiSize, char* apcAddressee, size_t aiBufferSize )
{
    static int sliCount = 0;
    
    if ( sliCount % 2 == 0 )
    {
        ::snprintf( apcAddressee, aiSize, "scpserver_0" );
    }
    else
    {
        ::snprintf( apcAddressee, aiSize, "scpserver_1" );
    }
    return 0;
}
