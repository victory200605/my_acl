
#include "aaf/SyncServer.h"
#include "acl/SockStream.h"

AAF_NAMESPACE_START

CSyncServer::CSyncServer(void)
    : mpoReactor(NULL)
    , miEvents(0)
    , moBuffer( sizeof(CRequest) )
{
}
    
CSyncServer::~CSyncServer(void)
{
}
    
apl_int_t CSyncServer::Startup( char const* apcConfigFileName )
{
    apl_int_t liRetCode = 0;
    
    //Start sockpair
    if ( ( liRetCode = this->moHandlePair.Open(APL_AF_UNIX, APL_SOCK_STREAM) ) != 0)
    {
        apl_errprintf(
            "ConfigureServer opend sockpair fail,%s [errno=%"APL_PRIdINT"]\n",
            apl_strerror(apl_get_errno()),
            apl_get_errno() );
        return -1;
    }
    
    this->moHandlePair.GetFirst().Disable(acl::CIOHandle::OPT_NONBLOCK);
    this->moHandlePair.GetSecond().Enable(acl::CIOHandle::OPT_NONBLOCK);
        
    if ( ( liRetCode = this->mpoReactor->RegisterHandler(
        this->moHandlePair.GetSecond().GetHandle(),
        this,
        acl::IEventHandler::READ_MASK ) ) != 0 )
    {
        apl_errprintf(
            "ConfigureServer register handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]\n",
            apl_strerror(apl_get_errno()),
            apl_get_errno(),
            (apl_int_t)acl::IEventHandler::READ_MASK );
                
        return -1;
    }
    
    return 0;
}
    
apl_int_t CSyncServer::Restart(void)
{
    return -1;
}
    
apl_int_t CSyncServer::Shutdown(void)
{
    this->moHandlePair.GetFirst().Close();
    this->moHandlePair.GetSecond().Close();
    
    return 0;
}

void CSyncServer::SetReactor( acl::CReactor* apoReactor )
{
    this->mpoReactor = apoReactor;
}
    
apl_int_t CSyncServer::Request( EmptyType* apoBase, MFuncType apfMFunc )
{
    static unsigned char lcValue  = 0;
    acl::CSockStream     loPeer;
    apl_ssize_t          liResult = 0;
    unsigned char        lcRecv   = 0;
    CRequest             loRequest;
    
    apl_memset(&loRequest, 0, sizeof(CRequest) );
    loRequest.mpoBase = apoBase;
    loRequest.mpfFunc = apfMFunc;
    
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    loRequest.mcMask = ++lcValue;
        
    loPeer.SetHandle( this->moHandlePair.GetFirst().GetHandle() );
    
    liResult = loPeer.Send(&loRequest, sizeof(loRequest), acl::CTimeValue::MAXTIME );
    if (liResult != sizeof(loRequest))
    {
        return -1;
    }
    
    while(true)
    {
        liResult = loPeer.Recv(&lcRecv, sizeof(lcValue), acl::CTimeValue(30) );
        if (liResult <= 0)
        {
            return -1;
        }
        if (lcValue == lcRecv)
        {
            return 0;
        }
    }
    
    return 0;
}

apl_int_t CSyncServer::HandleInput( apl_handle_t aiHandle )
{
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    
    loPeer.SetHandle(aiHandle);
    
    liResult = loPeer.Recv(this->moBuffer.GetWritePtr(), this->moBuffer.GetSpace() );
    if (liResult <= 0)
    {
        //Log here
        apl_errprintf(
            "ConfigureServer Input recv fail,%s [Handle=%"APL_PRIdINT"]/[errno=%"APL_PRIdINT"]\n",
            apl_strerror(apl_get_errno()),
            (apl_int_t)aiHandle,
            apl_get_errno() );
        
        return 0;
    }
    
    this->moBuffer.SetWritePtr(liResult);
    
    if (this->moBuffer.GetLength() >= sizeof(CRequest) )
    {
        apl_memcpy(&this->moRequest, this->moBuffer.GetReadPtr(), sizeof(CRequest) );
        
        (this->moRequest.mpoBase->*this->moRequest.mpfFunc)();
        
        this->moBuffer.Reset();
        
        liResult = loPeer.Send(&this->moRequest.mcMask, sizeof(this->moRequest.mcMask) );
        if (liResult != 1)
        {
            if ( this->mpoReactor->RegisterHandler(aiHandle, this, acl::IEventHandler::WRITE_MASK) != 0 )
            {
                apl_errprintf(
                    "ConfigureServer register handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]\n",
                    apl_strerror(apl_get_errno()),
                    apl_get_errno(),
                    (apl_int_t)acl::IEventHandler::WRITE_MASK );
            }
        }
    }

    return 0;
}
    
apl_int_t CSyncServer::HandleOutput( apl_handle_t aiHandle )
{
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    
    loPeer.SetHandle(aiHandle);
    
    liResult = loPeer.Send(&this->moRequest.mcMask, sizeof(this->moRequest.mcMask) );
    if (liResult <= 0)
    {
        apl_errprintf(
            "ConfigureServer output send close connection,%s [Handle=%"APL_PRIdINT"]/[errno=%"APL_PRIdINT"]\n",
            apl_strerror(apl_get_errno()),
            (apl_int_t)aiHandle,
            apl_get_errno() );
    }
    
    return 0;
}

AAF_NAMESPACE_END
