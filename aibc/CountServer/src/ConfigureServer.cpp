#include "ConfigureServer.h"

/////////////////////////////////////////////////////////////////////////////////////////
CIsmg50RequestHandler::CIsmg50RequestHandler(void)
{
    apl_memset(this->macLoginPasswd, 0, sizeof(this->macLoginPasswd) );
}
    
void CIsmg50RequestHandler::SetLoginPasswd( char const* apcPasswd )
{
    apl_strncpy(this->macLoginPasswd, apcPasswd, sizeof(this->macLoginPasswd) );
}

apl_int_t CIsmg50RequestHandler::RecvReloadRequest( acl::CSockStream& aoPeer )
{
    char             lasBuffer[128] = {0};
    char             lacPtlBuffer[128] = {0};
    apl_ssize_t      liResult = 0;
    acl::CTimeValue  loTimeout(10);

    apl_snprintf(lacPtlBuffer, sizeof(lacPtlBuffer), "HELLO %s", this->macLoginPasswd);
    
    if ( (liResult = aoPeer.RecvUntil(lasBuffer, sizeof(lasBuffer), "\r\n", loTimeout) ) < 0 )
    {
        return liResult;
    }
    
    if ( apl_strncmp(lacPtlBuffer, lasBuffer, apl_strlen(lacPtlBuffer) ) != 0 )
    {
        return -1;
    }
    
    if ( (liResult = aoPeer.Send("OK\r\n", apl_strlen("OK\r\n"), loTimeout) ) < 0 )
    {
        return liResult;
    }
    
    if ( (liResult = aoPeer.RecvUntil(lasBuffer, sizeof(lasBuffer), "\r\n", loTimeout) ) < 0 )
    {
        return liResult;
    }
    
    if ( apl_strncmp(lasBuffer, "RELOAD\r\n", apl_strlen("RELOAD\r\n") ) != 0 )
    {
        return -1;
    }
    
    return 0;
}

apl_int_t CIsmg50RequestHandler::SendReloadResponse( acl::CSockStream& aoPeer )
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
CIsmg35RequestHandler::CIsmg35RequestHandler(void)
{
    apl_memset(this->macPtlBuffer, 0, sizeof(this->macPtlBuffer) );
}
    
apl_int_t CIsmg35RequestHandler::RecvReloadRequest( acl::CSockStream& aoPeer )
{
    apl_ssize_t      liResult = 0;
    acl::CTimeValue  loTimeout(10);
    
    //protocol
    apl_int32_t      li32Field1 = 0;
    apl_int32_t      li32Field2 = 0;
    apl_int32_t      li32Field3 = 0;
    
    if ( (liResult = aoPeer.Recv(this->macPtlBuffer, sizeof(this->macPtlBuffer), loTimeout) ) < 0 )
    {
        return liResult;
    }

    apl_memcpy(&li32Field1, this->macPtlBuffer + 0, 4);
    apl_memcpy(&li32Field2, this->macPtlBuffer + 4, 4);
    apl_memcpy(&li32Field3, this->macPtlBuffer + 8, 4);
    
    li32Field1 = apl_ntoh32(li32Field1);
    li32Field2 = apl_ntoh32(li32Field2);
    li32Field3 = apl_ntoh32(li32Field3);

    if ( li32Field1 != 12 || li32Field2 != 0x01001001 || li32Field3 != 0 )
    {
        return -1;
    }

    return 0;
}

apl_int_t CIsmg35RequestHandler::SendReloadResponse( acl::CSockStream& aoPeer )
{
    apl_ssize_t      liResult = 0;
    acl::CTimeValue  loTimeout(5);

    if ( (liResult = aoPeer.Send(this->macPtlBuffer, sizeof(this->macPtlBuffer), loTimeout) ) < 0 )
    {
        return liResult;
    }
    
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

#define LDAP_MSGFREE(p) if( p ){ ldap_msgfree(p); p=NULL; }           
#define LDAP_UNBIND(ld) if((ld)!=NULL){ldap_unbind_s((ld));(ld)=NULL;}
    
#define CONF_STRING_EMPTYRETURN( config, field, key, buffer, ret )                   \
    {                                                                                \
        acl::CIniConfig::KeyIterType __loIter = config.GetValue(field, key, NULL);   \
        if( !__loIter.IsEmpty() )                                                    \
        {                                                                            \
            apl_strncpy(buffer, __loIter.ToString(), sizeof(buffer));                \
        }                                                                            \
        else                                                                         \
        {                                                                            \
            CNTSRV_LOG_ERROR( "Configure error, %s.%s can't be empty", field, key);  \
            return ret;                                                              \
        }                                                                            \
    }

#define CONF_INT_EMPTYRETURN( config, field, key, value, ret )                       \
    {                                                                                \
        acl::CIniConfig::KeyIterType __loIter = config.GetValue(field, key, NULL);   \
        if( !__loIter.IsEmpty() )                                                    \
        {                                                                            \
            value = __loIter.ToInt();                                                \
        }                                                                            \
        else                                                                         \
        {                                                                            \
            CNTSRV_LOG_ERROR( "Configure error, %s.%s can't be empty", field, key);  \
            return ret;                                                              \
        }                                                                            \
    }

#define CONF_STRING_DEFAULT( config, field, key, buffer, def )                       \
    {                                                                                \
        acl::CIniConfig::KeyIterType __loIter = config.GetValue(field, key, def);    \
        apl_strncpy(buffer, __loIter.ToString(), sizeof(buffer));                    \
    }

#define CONF_INT_DEFAULT( config, field, key, value, def )                           \
    {                                                                                \
        acl::CIniConfig::KeyIterType __loIter = config.GetValue(field, key, def);    \
        value = __loIter.ToInt();                                                    \
    }
    
CConfigureServer* CConfigureServer::cpoInstance = NULL;

CConfigureServer::CConfigureServer()
    : mu16UUMPort(0)
    , mi64MaxTime(0)
    , muMaxSize(0)
    , muCacheSize(0)
    , mu16ServerPort(0)
    , mu16AdminPort(0)
    , mbIsShutdown(false)
    , mpoReactor(NULL)
{
    apl_memset(this->macUUMIp, 0, sizeof(this->macUUMIp));
    apl_memset(this->macBaseDN, 0, sizeof(this->macBaseDN));
    apl_memset(this->macIpAddress, 0, sizeof(this->macIpAddress));
    apl_memset(this->macAdminLoginPasswd, 0, sizeof(this->macAdminLoginPasswd));
}

CConfigureServer::~CConfigureServer()
{
    this->Close();
    
    ACL_DELETE(this->mpoRequestHandler);
}

CConfigureServer* CConfigureServer::Instance()
{
    if(cpoInstance == NULL)
    {
        ACL_NEW_ASSERT(cpoInstance, CConfigureServer);
    }

    return cpoInstance;
}

void CConfigureServer::Release()
{
    if(cpoInstance != NULL)
    {
        ACL_DELETE(cpoInstance);
    }
}

apl_int_t CConfigureServer::LoadIniConfig(const char* apcIniFileName)
{
    apl_int_t       liRetCode = 0;
    acl::CIniConfig loConfig;
    
    liRetCode = loConfig.Open(apcIniFileName);
    if (liRetCode != 0)
    {
        CNTSRV_LOG_ERROR( "Open config file %s fail, [errno=%"APL_PRIdINT"]", apcIniFileName, apl_get_errno() );
        return liRetCode;
    }
    
    //[UUM]
    CONF_STRING_EMPTYRETURN(loConfig, "UUM", "BaseDN", this->macBaseDN, -1);
    CONF_STRING_EMPTYRETURN(loConfig, "UUM", "UUMIp", this->macUUMIp, -1);
    CONF_INT_EMPTYRETURN(loConfig,    "UUM", "UUMPort", this->mu16UUMPort, -1);
    CONF_INT_EMPTYRETURN(loConfig,    "UUM", "ISMGNo", this->miIsmgNo, -1);

    //[CountServer]
    CONF_STRING_EMPTYRETURN(loConfig, "CountServer", "IpAddress", this->macIpAddress, -1);
    CONF_INT_EMPTYRETURN(loConfig,    "CountServer", "ServerPort", this->mu16ServerPort, -1);
    CONF_INT_EMPTYRETURN(loConfig,    "CountServer", "AdminPort", this->mu16AdminPort, -1);
    CONF_STRING_EMPTYRETURN(loConfig, "CountServer", "AdminLoginPasswd", this->macAdminLoginPasswd, -1);
    CONF_INT_DEFAULT(loConfig,        "CountServer", "RequestNumLimit", this->miRequestNumLimit, "1");
    
    //[Log]
    CONF_INT_DEFAULT(loConfig, "Log", "MaxTime", this->mi64MaxTime, "900");
    CONF_INT_DEFAULT(loConfig, "Log", "MaxSize", this->muMaxSize, "5120000");
    CONF_INT_DEFAULT(loConfig, "Log", "CacheSize", this->muCacheSize, "5120000");

    if(this->miIsmgNo == 0)
    {
        ACL_NEW_ASSERT(this->mpoRequestHandler, CIsmg50RequestHandler);        
        dynamic_cast<CIsmg50RequestHandler*>(this->mpoRequestHandler)->SetLoginPasswd(this->macAdminLoginPasswd);
    }
    else
    {
        ACL_NEW_ASSERT(this->mpoRequestHandler, CIsmg35RequestHandler);
    }

    CNTSRV_LOG_INFO("--------------- [UUM] ---------------");
    CNTSRV_LOG_INFO("** %-20s=%s",           "BaseDN", this->macBaseDN);
    CNTSRV_LOG_INFO("** %-20s=%s",           "UUMIp", this->macUUMIp);
    CNTSRV_LOG_INFO("** %-20s=%"APL_PRIu16,  "UUMPort", this->mu16UUMPort);
    CNTSRV_LOG_INFO("** %-20s=%"APL_PRIdINT, "Ismg NO", this->miIsmgNo);

    CNTSRV_LOG_INFO("--------------- [CountServer] ---------------");
    CNTSRV_LOG_INFO("** %-20s=%s",           "IpAddress", this->macIpAddress);
    CNTSRV_LOG_INFO("** %-20s=%"APL_PRIu16,  "ServerPort", this->mu16ServerPort);
    CNTSRV_LOG_INFO("** %-20s=%"APL_PRIu16,  "AdminPort", this->mu16AdminPort);
    CNTSRV_LOG_INFO("** %-20s=%s",           "AdminLoginPasswd", this->macAdminLoginPasswd);
    CNTSRV_LOG_INFO("** %-20s=%"APL_PRIdINT, "RequestNumLimit", this->miRequestNumLimit);

    CNTSRV_LOG_INFO("--------------- [Log] ---------------");
    CNTSRV_LOG_INFO("** %-20s=%"APL_PRId64,  "MaxTime", this->mi64MaxTime);
    CNTSRV_LOG_INFO("** %-20s=%"APL_PRIuINT, "MaxSize", this->muMaxSize);
    CNTSRV_LOG_INFO("** %-20s=%"APL_PRIuINT, "CacheSize", this->muCacheSize);

    /* [TableN] */
    for ( acl::CIniConfig::FieldIterType loIter = loConfig.Begin("^Table[0-9]+");
        loIter != loConfig.End(); ++loIter )
    {
        CTableNode loTempNode;
        apl_memset(&loTempNode, 0, sizeof(loTempNode));
        
        CONF_STRING_EMPTYRETURN(loConfig, loIter.GetFieldName(), "Table", loTempNode.macTable, -1);
        CONF_STRING_EMPTYRETURN(loConfig, loIter.GetFieldName(), "Key",   loTempNode.macKey, -1);
        CONF_STRING_DEFAULT(loConfig, loIter.GetFieldName(), "KeySuffix", loTempNode.macKeySuffix, "");
        CONF_STRING_EMPTYRETURN(loConfig, loIter.GetFieldName(), "MaxConnection", loTempNode.macMaxConnection, -1);
        CONF_STRING_EMPTYRETURN(loConfig, loIter.GetFieldName(), "MaxSpeed", loTempNode.macMaxSpeed, -1);
        CONF_INT_EMPTYRETURN(loConfig, loIter.GetFieldName(), "SpeedBucketMultipleOf", loTempNode.miSpeedBucketMultipleOf, -1);

        this->moTables.push_back(loTempNode);
        
        CNTSRV_LOG_INFO("--------------- [%s] ---------------", loIter.GetFieldName() );
        CNTSRV_LOG_INFO("** %-20s=%s",           "Table", loTempNode.macTable);
        CNTSRV_LOG_INFO("** %-20s=%s",           "Key", loTempNode.macKey);
        CNTSRV_LOG_INFO("** %-20s=%s",           "KeySuffix", loTempNode.macKeySuffix);
        CNTSRV_LOG_INFO("** %-20s=%s",           "MaxConnection", loTempNode.macMaxConnection);
        CNTSRV_LOG_INFO("** %-20s=%s",           "MaxSpeed",  loTempNode.macMaxSpeed);
        CNTSRV_LOG_INFO("** %-20s=%"APL_PRIdINT, "SpeedBucketMultipleOf", loTempNode.miSpeedBucketMultipleOf);
    }

    return 0;
}
    
apl_int_t CConfigureServer::LoadUUMConfig( bool abIsCommit )
{
    apl_int_t liRetCode = 0;
    LDAP*     lpoLDAP   = NULL;
    TableListType::iterator loIter;
    
    if ( (liRetCode = LDAPInitialize(&lpoLDAP) ) != 0 )
    {
        return -1;
    }

    CTable::Instance()->StartUpdate();

    for(loIter = moTables.begin(); loIter != this->moTables.end(); ++loIter)
    {
        if ( (liRetCode = LDAPLoadTable(lpoLDAP, *loIter) ) != 0 )
        {
            break;
        }
    }
    
    LDAPClose(&lpoLDAP);
    
    if (liRetCode == 0 && abIsCommit)
    {
        CTable::Instance()->Commit();
    }

    return liRetCode;
}

apl_int_t CConfigureServer::LDAPInitialize( LDAP** appoLDAP )
{
    apl_int_t liRetCode = 0;
    LDAP*     lpoLDAP   = NULL;
    
    if ( (lpoLDAP = ldap_init(this->macUUMIp, this->mu16UUMPort) ) == NULL )
    {
        CNTSRV_LOG_ERROR("LDAP init fail, [errno=%"APL_PRIdINT"]", apl_get_errno() );
        return -1;
    }

    if( (liRetCode = ldap_bind_s(lpoLDAP, "", "", LDAP_AUTH_SIMPLE) ) == LDAP_SUCCESS)
    {
        *appoLDAP = lpoLDAP;
        return 0;
    }
    else
    {
        CNTSRV_LOG_ERROR(
            "LDAP bind fail,%s [retcode=%"APL_PRIdINT"]",
            ldap_err2string(liRetCode),
            liRetCode );
            
        LDAP_UNBIND(lpoLDAP);
        
        return -1;
    }
}

apl_int_t CConfigureServer::LDAPLoadTable( LDAP* apoLDAP, CTableNode& aoNode )
{
    apl_int_t liRetCode = 0;
    int       liMsgId = -1;
    char      lacFilter[] = "(objectclass=*)";
    char      lacRelativeDN[128] = {0};
    
    apl_snprintf(lacRelativeDN, sizeof(lacRelativeDN), "t=%s,%s", aoNode.macTable, this->macBaseDN);
    liRetCode = ldap_search_ext(
        apoLDAP,
        lacRelativeDN,
        LDAP_SCOPE_SUBTREE,
        lacFilter,
        NULL,
        0,
        NULL,
        NULL,
        0,
        0,
        &liMsgId );
    if(liRetCode != LDAP_SUCCESS)
    {
        CNTSRV_LOG_ERROR(
            "LDAP search fail,%s [retcode=%"APL_PRIdINT"]",
            ldap_err2string(liRetCode), 
            liRetCode );
        
        return -1;
    }
    
    CNTSRV_LOG_DEBUG("Table Name:%s", aoNode.macTable);
    CNTSRV_LOG_DEBUG("--------------------------------------------------------------------------------------");
    CNTSRV_LOG_DEBUG("|%20s| %30s| %30s|", aoNode.macKey, aoNode.macMaxConnection, aoNode.macMaxSpeed);
    CNTSRV_LOG_DEBUG("--------------------------------------------------------------------------------------");
    
    while(true)
    {
        LDAPMessage* lpoEntry = NULL; 
        LDAPMessage* lpoRes = NULL;

        if ( (liRetCode = ldap_result(apoLDAP, liMsgId, 0, NULL, &lpoRes) ) < 0 )
        {
            CNTSRV_LOG_ERROR(
                "LDAP result fail,%s [retcode=%"APL_PRIdINT"]",
                ldap_err2string(liRetCode), 
                liRetCode );
            
            return -1;
        }

        //loop for entry
        for( lpoEntry = ldap_first_entry(apoLDAP, lpoRes); lpoEntry != NULL; 
             lpoEntry = ldap_next_entry(apoLDAP, lpoEntry) )
        {
            char      lacTempNo[128] = {0};
            apl_int_t liMaxConn   = -1;
            apl_int_t liMaxSpeed  = -1;
            apl_int_t liHits      = 0;
            char*     lpcAtt      = NULL; 
            char**    lppVals     = NULL;
            BerElement* lpoBerPtr = NULL; 
    
            for ( lpcAtt = ldap_first_attribute(apoLDAP, lpoEntry, &lpoBerPtr); lpcAtt != NULL;
                  lpcAtt = ldap_next_attribute(apoLDAP, lpoEntry, lpoBerPtr))               
            {
                lppVals = ldap_get_values(apoLDAP, lpoEntry, lpcAtt);
                if(apl_strcasecmp(lpcAtt, aoNode.macKey) == 0)
                {
                    apl_snprintf(lacTempNo, sizeof(lacTempNo), "%s%s", lppVals[0], aoNode.macKeySuffix);
                    liHits++;
                } 
                else if(apl_strcasecmp(lpcAtt, aoNode.macMaxConnection) == 0)
                {
                    liMaxConn = apl_strtoi32(lppVals[0], NULL, 10);
                    liHits++;
                }
                else if(apl_strcasecmp(lpcAtt, aoNode.macMaxSpeed) == 0)
                {
                    liMaxSpeed = apl_strtoi32(lppVals[0], NULL, 10);
                    liHits++;
                }
#if ((LDAP_API_VERSION - 3000) > 0)
                ldap_memfree(lpcAtt);    
#endif
                ldap_value_free(lppVals);
            }
            
            ber_free(lpoBerPtr, 0);
            
            if (liHits != 3)
            {
                CNTSRV_LOG_ERROR(
                    "LDAP table field name match fail,[Table=%s]/[Key=%s]/[MaxConnection=%s]/[MaxSpeed=%s]",
                    aoNode.macTable, 
                    aoNode.macKey,
                    aoNode.macMaxConnection,
                    aoNode.macMaxSpeed );
                    
                LDAP_MSGFREE(lpoRes);
                return -1;
            }
            else
            {
                CNTSRV_LOG_DEBUG( "|%20s| %30"APL_PRIdINT"| %30"APL_PRIdINT"|", lacTempNo, liMaxConn, liMaxSpeed );
                    
                CTable::Instance()->UpdateCounter(lacTempNo, liMaxConn);
                CTable::Instance()->UpdateSpeeder(lacTempNo, liMaxSpeed, aoNode.miSpeedBucketMultipleOf * liMaxSpeed);
            }
        }
    
        LDAP_MSGFREE(lpoRes);

        if(liRetCode == LDAP_RES_SEARCH_RESULT)      
        { 
            break;
        }
    }
    
    CNTSRV_LOG_DEBUG("--------------------------------------------------------------------------------------");
    
    return 0;
}

void CConfigureServer::LDAPClose( LDAP** appoLDAP )
{
    LDAP_UNBIND(*appoLDAP);
}

apl_int_t CConfigureServer::Startup(acl::CReactor& aoReactor, const char* apcIniFileName)
{
    apl_int_t liRetCode = -1;
    
    this->mpoReactor = &aoReactor;
    
    {//Configure
        //Load from ini file
        if ( (liRetCode = this->LoadIniConfig(apcIniFileName) ) != 0 )
        {
            return -1;
        }
    
        //Load from uum
        if ( (liRetCode = this->LoadUUMConfig(true) ) != 0 )
        {
            return -1;
        }
    }
    
    {//Reload sync
        //Start sockpair
        if ( ( liRetCode = this->moHandlePair.Open(APL_AF_UNIX, APL_SOCK_STREAM) ) != 0)
        {
            CNTSRV_LOG_ERROR(
                "ConfigureServer opend sockpair fail,%s [errno=%"APL_PRIdINT"]",
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
            CNTSRV_LOG_ERROR(
                "ConfigureServer register handler fail,%s [errno=%"APL_PRIdINT"]/[event=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno(),
                (apl_int_t)acl::IEventHandler::READ_MASK );
                    
            return -1;
        }
    }
    
    {//Reload server
        acl::CSockAddr loLocalAddr(this->mu16AdminPort, this->macIpAddress);
        if((liRetCode = this->moSockAcceptor.Open(loLocalAddr)) != 0)
        {
            CNTSRV_LOG_ERROR(
                "Acceptor open [%s:%"APL_PRIu16"] fail,%s [errno=%"APL_PRIdINT"]",
                this->macIpAddress,
                this->mu16AdminPort,
                apl_strerror(apl_get_errno()),
                apl_get_errno() );
                
            return -1;
        }
        
        if( (liRetCode = this->moThreadManager.Spawn(Svc, (void*)this) ) != 0)
        {
            CNTSRV_LOG_ERROR(
                "Spawn svc thread fail,%s [errno=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                apl_get_errno() );
            
            return -1;
        }
    }
    
    return 0;
}

void CConfigureServer::Close()
{
    this->mbIsShutdown = true;
    
    this->moSockAcceptor.Close();
    this->moHandlePair.GetFirst().Close();
    this->moHandlePair.GetSecond().Close();
    
    this->moThreadManager.WaitAll();
}

void* CConfigureServer::Svc(void* apvParam)
{
    apl_int_t        liRetCode = -1;
    acl::CSockStream loPeer;
    acl::CSockAddr   loRemote;
    acl::CTimeValue  loTimeout(2);
    char             lacIpAddress[64] = {0};
    
    CConfigureServer* lpoServer = (CConfigureServer*)apvParam;

    while(!lpoServer->mbIsShutdown)
    {
        liRetCode = lpoServer->moSockAcceptor.Accept(loPeer, NULL, loTimeout);  
        if(liRetCode != 0)
        {
            if (apl_get_errno() != APL_ETIMEDOUT && !lpoServer->mbIsShutdown)
            {
                CNTSRV_LOG_ERROR(
                    "ConfigureServer accept connection fail,%s [handle=%"APL_PRIdINT"][errno=%"APL_PRIdINT"]",
                    apl_strerror(apl_get_errno()),
                    (apl_int_t)lpoServer->moSockAcceptor.GetHandle(),
                    apl_get_errno() );
            }
            
            continue;
        }
        
        loPeer.GetRemoteAddr(loRemote);
        loRemote.GetIpAddr(lacIpAddress, sizeof(lacIpAddress) );
        CNTSRV_LOG_INFO(
            "ConfigureServer accept a new connection success, [Handle=%"APL_PRIdINT"]/[IpAddress=%s:%"APL_PRIdINT"]",
            (apl_int_t)loPeer.GetHandle(),
            lacIpAddress,
            (apl_int_t)loRemote.GetPort() );
        
        do
        {
            if(lpoServer->mpoRequestHandler->RecvReloadRequest(loPeer) != 0)
            {
                CNTSRV_LOG_ERROR(
                    "ConfigureServer Recv reload request fail,%s [errno=%"APL_PRIdINT"]",
                    apl_strerror(apl_get_errno()),
                    apl_get_errno() );
                    
                break;
            }
            
            CNTSRV_LOG_INFO("ConfigureServer start reload config from uum");
            
            if(lpoServer->LoadUUMConfig(false) != 0)
            {
                break;
            }
    
            if(lpoServer->RequestCommit() != 0)
            {
                CNTSRV_LOG_ERROR(
                    "ConfigureServer request commit fail,%s [errno=%"APL_PRIdINT"]",
                    apl_strerror(apl_get_errno()),
                    apl_get_errno() );
                    
                break;
            }
    
            if(lpoServer->mpoRequestHandler->SendReloadResponse(loPeer) != 0)
            {
                CNTSRV_LOG_ERROR(
                    "ConfigureServer send reload response fail,%s [errno=%"APL_PRIdINT"]",
                    apl_strerror(apl_get_errno()),
                    apl_get_errno() );
                    
                break;
            }
            
            CNTSRV_LOG_INFO("ConfigureServer reload config from uum complete");
        }
        while(false);
        
        CNTSRV_LOG_INFO("ConfigureServer close connection, [Handle=%"APL_PRIdINT"]", (apl_int_t)loPeer.GetHandle() );
        loPeer.Close();
    }
    
    lpoServer->moSockAcceptor.Close();
    
    return NULL;
}

apl_int_t CConfigureServer::RequestCommit(void)
{
    static char      lcValue = 0;
    acl::CSockStream loPeer;
    apl_ssize_t      liResult = 0;
    char             lcRecv  = 0;
        
    lcValue = ++lcValue % 128;
    
    loPeer.SetHandle( this->moHandlePair.GetFirst().GetHandle() );
    
    liResult = loPeer.Send(&lcValue, sizeof(lcValue), acl::CTimeValue(2) );
    if (liResult <= 0)
    {
        return -1;
    }
    
    while(true)
    {
        liResult = loPeer.Recv(&lcRecv, sizeof(lcValue), acl::CTimeValue(5) );
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

apl_int_t CConfigureServer::HandleInput( apl_handle_t aiHandle )
{
    apl_ssize_t      liResult = 0;
    acl::CSockStream loPeer;
    char lacBuffer[128];
    
    loPeer.SetHandle(aiHandle);
    
    do
    {
        liResult = loPeer.Recv(lacBuffer, sizeof(lacBuffer) );
        if (liResult <= 0)
        {
            //Log here
            CNTSRV_LOG_ERROR_NB(
                "ConfigureServer Input recv fail,%s [Handle=%"APL_PRIdINT"]/[errno=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                (apl_int_t)aiHandle,
                apl_get_errno() );
            
            break;
        }
        
        CTable::Instance()->Commit();
    
        liResult = loPeer.Send(lacBuffer, liResult);
        if (liResult <= 0)
        {
            CNTSRV_LOG_ERROR_NB(
                "ConfigureServer output send close connection,%s [Handle=%"APL_PRIdINT"]/[errno=%"APL_PRIdINT"]",
                apl_strerror(apl_get_errno()),
                (apl_int_t)aiHandle,
                apl_get_errno() );
        }
        
    }while(false);
    
    return 0;
}
    
apl_int_t CConfigureServer::HandleOutput( apl_handle_t aiHandle )
{
    return 0;
}

const char* CConfigureServer::GetIpAddress()
{
    return this->macIpAddress;
}

apl_uint16_t CConfigureServer::GetServerPort()
{
    return this->mu16ServerPort;
}

apl_uint16_t CConfigureServer::GetAdminPort()
{
    return this->mu16AdminPort;
}

const char* CConfigureServer::GetAdminLoginPasswd()
{
    return this->macAdminLoginPasswd;
}

apl_time_t CConfigureServer::GetLogMaxTime()
{
    return this->mi64MaxTime;
}

apl_size_t CConfigureServer::GetLogMaxSize()
{
    return this->muMaxSize;
}

apl_size_t CConfigureServer::GetLogCacheSize()
{
    return this->muCacheSize;
}

apl_int_t CConfigureServer::GetRequestNumLimit(void)
{
    return this->miRequestNumLimit;
}
