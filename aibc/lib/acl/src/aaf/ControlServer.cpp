
#include "aaf/ControlServer.h"
#include "acl/IniConfig.h"

AAF_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////////////
CCmdLineControlProtocol::CCmdLineControlProtocol(void)
    : moRecvBuffer(1024)
    , moSendBuffer(1024)
    , moTokenizer(" ")
{
}
    
CCmdLineControlProtocol::~CCmdLineControlProtocol(void)
{
}

apl_int_t CCmdLineControlProtocol::Login( 
    acl::CSockStream& aoPeer,
    char const* apcUser,
    char const* apcPasswd,
    acl::CTimeValue const& aoTimeout )
{
    apl_int_t liRetCode = -1;
    
    do
    {
        if ( this->Recv( aoPeer, aoTimeout ) < 0 )
        {
            this->Response("FAIL \"timedout\"");
            break;
        }
        
        if ( apl_strcasecmp("LOGIN", this->GetArgv(0) ) != 0 
            && apl_strcasecmp("HELLO", this->GetArgv(0) ) != 0  )
        {
            this->Response("FAIL \"permision denied\"");
            break;
        }

        if (this->GetArgc() > 2)
        {
            if ( apl_strcmp(apcUser, "*" ) != 0 && apl_strcmp(apcUser, this->GetArgv(1) ) != 0 )
            {
                this->Response("FAIL \"permision denied\"");
                break;
            }
            
            if ( apl_strcmp(apcPasswd, "*" ) != 0 && apl_strcmp(apcPasswd, this->GetArgv(2) ) != 0 )
            {
                this->Response("FAIL \"permision denied\"");
                break;
            }
        }
        else if (this->GetArgc() > 1)
        {
            if (apl_strcmp(apcUser, "*" ) != 0 
                || (apl_strcmp(apcPasswd, "*" ) != 0 && apl_strcmp(apcPasswd, this->GetArgv(1) ) != 0) )
            {
                this->Response("FAIL \"permision denied\"");
                break;
            }
        }
        else
        {
            if (apl_strcmp(apcUser, "*" ) != 0 || apl_strcmp(apcPasswd, "*" ) != 0 )
            {
                this->Response("FAIL \"permision denied\"");
                break;
            }
        }
        
        this->Response("OK");
        
        liRetCode = 0;
    }
    while(false);

    this->Send(aoPeer, aoTimeout);
            
    return liRetCode;
}
    
apl_int_t CCmdLineControlProtocol::Recv( acl::CSockStream& aoPeer, acl::CTimeValue const& aoTimeout )
{
    apl_ssize_t     liResult = 0;
    
    this->moRecvBuffer.Reset();
    
    if ( (liResult = aoPeer.RecvUntil(
        this->moRecvBuffer.GetWritePtr(), this->moRecvBuffer.GetSpace(), "\r\n", aoTimeout) ) < 0 )
    {
        return -1;
    }
    
    this->moRecvBuffer.SetWritePtr(liResult - 2);
    this->moRecvBuffer.Write("");
    
    this->moTokenizer.Parse( this->moRecvBuffer.GetReadPtr() );
    
    this->moRecvBuffer.Reset();
    
    return 0;
}
    
apl_int_t CCmdLineControlProtocol::Send( acl::CSockStream& aoPeer, acl::CTimeValue const& aoTimeout )
{
    apl_int_t liRetCode = 0;
    if (this->moSendBuffer.GetLength() > 0)
    {        
        if ( aoPeer.Send(
            this->moSendBuffer.GetReadPtr(),
            this->moSendBuffer.GetLength(),
            aoTimeout ) != (apl_ssize_t)this->moSendBuffer.GetLength() )
        {
            liRetCode = -1;
        }
        else
        {
            liRetCode = 0;
        }
    }
    
    this->moSendBuffer.Reset();
    
    return liRetCode;
}

apl_int_t CCmdLineControlProtocol::Logout( acl::CSockStream& aoPeer )
{
    return 0;
}

void CCmdLineControlProtocol::Response( char const* apcFormat, ... )
{
    va_list    loVaList;
    apl_size_t liLength = 0;
    
    this->moSendBuffer.Reset();
    
    va_start(loVaList, apcFormat);

    apl_vsnprintf(this->moSendBuffer.GetWritePtr(), this->moSendBuffer.GetSpace() - 2, apcFormat, loVaList);
    liLength = apl_strlen(this->moSendBuffer.GetWritePtr() );
    
    va_end(loVaList);
    
    this->moSendBuffer.SetWritePtr(liLength);
    this->moSendBuffer.Write("\r\n");
}

apl_size_t CCmdLineControlProtocol::GetArgc(void)
{
    return this->moTokenizer.GetSize();
}
    
char const* CCmdLineControlProtocol::GetArgv( apl_size_t aiN )
{
    return this->moTokenizer.GetField(aiN);
}

//////////////////////////////////////////////////////////////////////////////////
CControlServer::CControlServer(void)
    : mbIsShutdown(true)
    , mpoProtocol(NULL)
    , miPort(0)
    , moTimeout(10)
{
    ACL_NEW_ASSERT(this->mpoProtocol, CCmdLineControlProtocol);
    
    apl_memset(this->macIpAddress, 0, sizeof(this->macIpAddress) );
    apl_memset(this->macUser, 0, sizeof(this->macUser) );
    apl_memset(this->macPasswd, 0, sizeof(this->macPasswd) );
}
    
CControlServer::~CControlServer(void)
{
    ACL_DELETE(this->mpoProtocol);
}
    
apl_int_t CControlServer::Startup( char const* apcConfigFileName )
{
    acl::CIniConfig loConfig;
    
    if (loConfig.Open(apcConfigFileName) != 0)
    {
        return -1;
    }
    
    //Load log config
    this->miPort = loConfig.GetValue("Control", "Port", "-1").ToInt();
    this->moTimeout = loConfig.GetValue("Control", "Timeout", "5").ToInt();
    apl_strncpy(
        this->macIpAddress,
        loConfig.GetValue("Control", "IpAddress", "127.0.0.1").ToString(),
        sizeof(this->macIpAddress) );
    apl_strncpy(
        this->macUser,
        loConfig.GetValue("Control", "User", "*").ToString(),
        sizeof(this->macUser) );
    apl_strncpy(
        this->macPasswd,
        loConfig.GetValue("Control", "Passwd", "*").ToString(),
        sizeof(this->macPasswd) );
    //end
    
    if (this->miPort <= 0 || this->miPort > (apl_int_t)UINT16_MAX)
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    acl::CSockAddr loAddr(this->miPort, this->macIpAddress);
    if (this->moAcceptor.Open(loAddr) < 0)
    {
        return -1;
    }
    
    this->mbIsShutdown = false;
    
    if (this->moThreadManager.Spawn(CControlServer::Svc, this) < 0 )
    {
        this->mbIsShutdown = true;
        return -1;
    }
    
    return 0;
}
    
apl_int_t CControlServer::Restart(void)
{
    return -1;
}

apl_int_t CControlServer::Shutdown(void)
{
    this->mbIsShutdown = true;
    
    this->moAcceptor.Close();
    
    this->moThreadManager.WaitAll();
    
    return 0;
}

bool CControlServer::IsShutdown(void)
{
    return this->mbIsShutdown;
}

void CControlServer::UpdateShutdown( bool abIsShutdown )
{
    this->mbIsShutdown = abIsShutdown;
}
    
void CControlServer::SetProtocol( IControlProtocol* apoProtocol )
{
    ACL_DELETE(this->mpoProtocol);
    this->mpoProtocol = apoProtocol;
}

apl_int_t CControlServer::HandleAccept( acl::CSockStream& aoPeer )
{
    return 0;
}
    
apl_int_t CControlServer::HandleInput( IControlProtocol* apoProtocol )
{
    return 0;
}
    
apl_int_t CControlServer::HandleError( acl::CSockStream& aoPeer, apl_int_t aiErrno )
{
    return 0;
}
    
apl_int_t CControlServer::HandleClose( acl::CSockStream& aoPeer )
{
    return 0;
}

void* CControlServer::Svc(void* apvParam)
{
    CControlServer* lpoServer = static_cast<CControlServer*>(apvParam);
    acl::CSockStream loPeer;
    acl::CTimeValue  loValue(2);
    
    while(!lpoServer->mbIsShutdown)
    {
        if (lpoServer->moAcceptor.Accept(loPeer, NULL, loValue) < 0)
        {
            continue;
        }
        
        if ( lpoServer->mpoProtocol->Login(loPeer, lpoServer->macUser, lpoServer->macPasswd, lpoServer->moTimeout) < 0 )
        {
            loPeer.Close();
            continue;
        }
        
        lpoServer->HandleAccept(loPeer);

        while(true)
        {
            if ( lpoServer->mpoProtocol->Recv(loPeer, lpoServer->moTimeout) < 0 )
            {
                lpoServer->HandleError(loPeer, apl_get_errno() );
                break;
            }
            
            if ( lpoServer->HandleInput(lpoServer->mpoProtocol) < 0 )
            {
                break;
            }
            
            if ( lpoServer->mpoProtocol->Send(loPeer, lpoServer->moTimeout) < 0 )
            {
                lpoServer->HandleError(loPeer, apl_get_errno() );
                break;
            }
        }
        
        lpoServer->mpoProtocol->Logout(loPeer);
        
        lpoServer->HandleClose(loPeer);
        
        loPeer.Close();
    }
    
    return NULL;
}

AAF_NAMESPACE_END
