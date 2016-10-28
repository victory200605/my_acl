
#include "KeepAlived.h"
#include "MasterConfig.h"
#include "acl/Thread.h"

AIBC_GFQ_NAMESPACE_START

CKeepAlived::CKeepAlived(void)
    : mbIsEnableKeepAlived(false)
    , mbIsClosed(false)
    , miLocalPort(0)
{
}

CKeepAlived::~CKeepAlived(void)
{
}

apl_int_t CKeepAlived::Startup( bool abIsEnableKeepAlived, char const* apcConfigCommand )
{
    this->moConfigCommand = apcConfigCommand;
    this->mbIsEnableKeepAlived = abIsEnableKeepAlived;

    return aaf::CDaemonService::Startup(APL_NULL);
}

apl_int_t CKeepAlived::Initialize(void)
{
    if (!this->mbIsEnableKeepAlived)
    {
        //Disable keepalived
        return 0;
    }

    CMasterConfig loConfig;

    if (loConfig.Initialize(this->moConfigCommand.c_str() ) != 0)
    {
        apl_errprintf("Load config from local file fail %s\n", (apl_get_errno() != 0 ? apl_strerror(apl_get_errno() ) : "" ) );

        return -1;
    }
    
    this->moMcastAddr.Set(loConfig.miGroupPort, loConfig.moGroupAddress.c_str() );

    if (this->moWatcher.Open() != 0)
    {
        apl_errprintf("Open watcher sock handle fail, %s\n", (apl_get_errno() != 0 ? apl_strerror(apl_get_errno() ) : "" ) );
        
        return -1;
    }
    
    if (this->IsAlived(this->moMcastAddr) )
    {
        this->WriteLog(acl::CDefaultFormatter<1024>(acl::LOG_LVL_INFO, "Master is alived, Entering STANDBY state\n") );

        this->mbIsActive = false;
    }
    else
    {
        this->WriteLog(acl::CDefaultFormatter<1024>(acl::LOG_LVL_INFO, "Master is un-alived, Entering ACTIVE state\n") );

        this->mbIsActive = true;
    }

    if (this->moMcast.Join(this->moMcastAddr) != 0)
    {
        apl_errprintf("Open mcast sock handle fail, %s\n", (apl_get_errno() != 0 ? apl_strerror(apl_get_errno() ) : "" ) );
        
        return -1;
    }

    this->moWatcher.Enable(acl::CSockDgram::OPT_NONBLOCK);
    this->moMcast.Enable(acl::CSockDgram::OPT_NONBLOCK);
    this->moLocalAddress = loConfig.moLocalAddress;
    this->miLocalPort = loConfig.miLocalPort;
    this->miTimedout = loConfig.miKeepalivedTimedout;
    this->miRetryTimes = loConfig.miKeepalivedRetryTimes;
    this->mbIsClosed = false;

    if (acl::CThread::Spawn(Srv, this) != 0)
    {
        apl_errprintf("Start server thread fail, %s\n", (apl_get_errno() != 0 ? apl_strerror(apl_get_errno() ) : "" ) );
        
        return -1;
    }

    return 0;
}

void CKeepAlived::Finalize(void)
{
    this->mbIsClosed = true;
    this->moThreadManager.WaitAll();
    this->moMcast.Close();
}

bool CKeepAlived::IsAlived( acl::CSockAddr const& aoGroupAddr )
{
    acl::CSockAddr loRemote;
    apl_int_t liTimes = this->miRetryTimes;
    char lacRequest[64] = "who is master";
    char lacResponse[64] = {0,};

    //Clear mcast buffer
    while(this->moMcast.Recv(lacResponse, sizeof(lacResponse), &loRemote) > 0)
    {
        //NULL
    }

    //Detecting
    while(--liTimes > 0)
    {
        if (this->moWatcher.Send(lacRequest, 14, aoGroupAddr, this->miTimedout) != 14)
        {
            continue;
        }

        apl_ssize_t liResult = this->moWatcher.Recv(lacResponse, sizeof(lacResponse), &loRemote, this->miTimedout);
        if (liResult > 0)
        {
            return true;
        }
    }

    return false;
}

void CKeepAlived::RequestHandle( acl::CTimeValue const& aoTimedout )
{
    acl::CSockAddr loRemote;
    char lacRequest[64] = {0,};
    char lacResponse[64] = {0,};

    apl_snprintf(lacResponse, sizeof(lacResponse), "%s:%"APL_PRIdINT"", this->moLocalAddress.c_str(), this->miLocalPort);

    apl_ssize_t liResult = this->moMcast.Recv(lacRequest, sizeof(lacRequest) - 1, &loRemote, aoTimedout);
    if (liResult > 0)
    {
        if (apl_strcmp(lacRequest, "who is master") == 0)
        {
            this->moMcast.Send(lacResponse, apl_strlen(lacResponse) + 1, loRemote, aoTimedout);
        }
    }
}

void* CKeepAlived::Srv( void* apvParam )
{
    acl::CTimeValue loTimedout(3);
    CKeepAlived* lpoKeepAlived = static_cast<CKeepAlived*>(apvParam);

    if (!lpoKeepAlived->mbIsActive)
    {
        while(!lpoKeepAlived->mbIsClosed)
        {
            if (!lpoKeepAlived->IsAlived(lpoKeepAlived->moMcastAddr) )
            {
                break;
            }

            apl_sleep(APL_TIME_SEC);
        }
        
        lpoKeepAlived->WriteLog(
            acl::CDefaultFormatter<1024>(acl::LOG_LVL_INFO, "Master is un-alived, Entering ACTIVE state\n") );

        lpoKeepAlived->mbIsActive = true;
    }

    while(!lpoKeepAlived->mbIsClosed)
    {
        lpoKeepAlived->RequestHandle(loTimedout);
    }

    return APL_NULL;
}

AIBC_GFQ_NAMESPACE_END

