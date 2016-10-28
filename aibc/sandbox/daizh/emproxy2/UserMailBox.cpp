
#include "UserMailBox.h"

CUserMailBox::CUserMailBox( char const* apcUserName )
    : moUserName(apcUserName)
{
}

apl_int_t CUserMailBox::Init(void)
{
    /*
    {//User certify
        char  lacUser[NAME_MAX] = {0};
        char  lacDomain[NAME_MAX] = {0};
        apl_int_t liRetCode = 0;
        class clsDataList loList;
        class clsListMap  loMap;
        
        loList.ciAppend(UAS_ATTR_UID);
        loList.ciAppend(UAS_ATTR_MBID);
        loList.ciAppend(UAS_ATTR_MAILROOT);

        giParseEmailAddress(apcUserName, lacUser, sizeof(lacUser), lacDomain,  sizeof(lacDomain) );
        liRetCode = gpUASc->ciCertifyUser(lacUser, lacDomain, APL_NULL, UAS_VAL_SERVICE_EMPROXY, &loList, &loMap);
        if (liRetCode > 0)
        {
            this->moMBID = loMap.csGetValue(UAS_ATTR_MBID);
            this->moUID = loMap.csGetValue(UAS_ATTR_UID);
            this->miMailRoot = apl_strtoi32(loMap.csGetValue(UAS_ATTR_MAILROOT), APL_NULL, 10);
        }
        else
        {
            return ERROR_CERTIFY_USER;
        }
    }

    {//Mail box count limit
        UserLevel_t* lpoUserLevel = APL_NULL;
        if( (lpoUserLevel = gpUserLevel->cptGetUserLevel(loMailboxAttr.iUserLevel) ) == NULL)
        {
            //TODO ERROR
            this->miMaxMailCount = -1;
        }
        else
        {
            this->miMaxMailCount = lpoUserLevel->ciMBMaxMailCount;
        }
        ACL_FREE(lpoUserLevel);
    }
    */
    return 0;
}

apl_int_t CUserMailBox::OpenMailBox(void)
{
    /*
    if (this->mpoMailBox == APL_NULL)
    {
        ACL_ASSERT_NEW(this->mpoMailBox, 
            clsMailBox(this->moMBID.c_str(), this->moUserName.c_str(), this->miMailRoot) );
    }
    */

    return 0;
}

apl_int_t CUserMailBox::GetPOPAccountInfo( apl_int_t aiPOPID, CPOPAccountInfo* apoInfo )
{
    /*
    char lacField[128] = {0};
    char lacUser[64] = {0};
    char lacHostName[64] = {0};
    char lacPasswd[64] = {0};
    char lacPort[8] = {0};
    char lacDelMail[8] = {0};
    char lacFoldID[24] = {0};
    
    apl_snprintf(lacField, sizeof (lacField), "%s%s", EXTERNAL_POP_KEY, aiPOPID);

    this->mpoMailbox->csGetSetting(lacField, USERKEY, lacUser, sizeof(lacUser) );
    this->mpoMailbox->csGetSetting(lacField, SERVERKEY, lacHostName, sizeof(lacHostName) );
    this->mpoMailbox->csGetSetting(lacField, PASSWORDKEY, lacPasswd, sizeof(lacPasswd) );
    this->mpoMailbox->csGetSetting(lacField, DELMAILKEY, lacDelMail, sizeof(lacDelMail) );
    this->mpoMailbox->csGetSetting(lacField, POPFID, lacFoldID, sizeof(lacFoldID) );
    this->mpoMailbox->csGetSetting(lacField, PORTKEY, lacPort, sizeof(lacPort) );
    this->DecodePassword(lacPasswd, sizeof(lacPasswd) );

    apoInfo->moUser = lacUser;
    apoInfo->moPass = lacPasswd;
    apoInfo->moHostName = lacHostName;
    apoInfo->miPort = apl_strtoi32(lacPort, APL_NULL, 10);
    apoInfo->miFoldID = apl_strtoi32(lacFoldID, APL_NULL, 10);
    */
    return 0;
}

apl_int_t CUserMailBox::ReadUidlFile( apl_int_t aiPOPID, acl::CMemoryBlock* apoBuffer )
{
    /*
    char lacTemp[64];

    apl_snprintf(lacTemp, sizeof(lacTemp), "%s%d.uidl", LOCAL_UIDL_FILENAME, this->miPOPID);

    apl_int_t liSize = this->mpoMailbox->ciGetDataFileSize(lacTemp);
    if (liSize > 0)
    {
        apoBuffer->Reset();
        apoBuffer->Resize(liSize);

        if (this->mpoMailbox->ciReadDataFile(lacTemp, apoBuffer->GetWritePtr(), liSize ) != liSize)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }
    else if (liSize < 0)
    {
        return -1;
    }
    */

    return 0;
}

apl_int_t CUserMailBox::AppendUidlFile( apl_int_t aiPOPID, std::string const& aoStr )
{
    /*
    char lacTemp[64];

    apl_snprintf(lacTemp, sizeof(lacTemp), "%s%d.uidl", LOCAL_UIDL_FILENAME, this->miPOPID);

    if (this->mpoMailbox->ciWriteDataFile(lacTemp, aoStr.c_str(), aoStr.length(), 0, DF_APPEND) != aoStr.length() )
    {
        return -1;
    }
    else
    {
        return 0;
    }
    */
    return 0;
}

apl_int_t CUserMailBox::LockPOPAccount( apl_int_t aiPOPID )
{
    /*
    char lacTemp[64];
    char lacValue[512];
    
    //Make field name
    apl_snprintf(lacTemp, sizeof(lacTemp), "%s%d", EXTERNAL_POP_KEY, aiItemNo);

    //Load POP process lock status
    this->mpoMailbox->csGetSetting(lacTemp, LOCKKEY, lacValue, sizeof(lacValue) );
    apl_int_t liLastTime = apl_strtoi32(lacValue, APL_NULL, 10);
    if (liLastTime > 0 && apl_time() / APL_TIME_SEC - liLastTime < this->miPOPTimedout)
    {
        //TODO ERROR other process is popping now
        return -1;
    }
    else
    {
        //Locking mailbox pop operation
        apl_snprintf(lacValue, sizeof(lacValue), "%"APL_PRIdINT, apl_time()/APL_TIME_SEC);
        this->mpoMailbox->csSaveSetting(lacTemp, LOCKKEY, lacValue);

        return 0;
    }
    */
    return 0;
}

apl_int_t CUserMailBox::UnlockPOPAccount( apl_int_t aiAccount )
{
    /*
    char lacTemp[64];
    
    //Make field name
    apl_snprintf(lacTemp, sizeof(lacTemp), "%s%d", EXTERNAL_POP_KEY, aiItemNo);

    //Load POP process lock status
    this->mpoMailbox->csSaveSetting(lacTemp, LOCKKEY, "0" );
    */
    return 0;
}

apl_int_t CUserMailBox::GetStat( apl_size_t* apuCount, apl_size_t* apuSpace )
{
    /*
    MailBoxAttr_t loMailboxAttr;

    this->mpoMailbox->ciGetAttrib(&loMailboxAttr);

    if (apuCount != APL_NULL)
    {
        *apuCount = loMailboxAttr.iTotalCount;
    }

    if (apuSpace != APL_NULL)
    {
        *apuSpace = loMailboxAttr.iSizeLimit - loMailboxAttr.iSizeOccupied;
    }
    */
    return 0;
}

char const* CUserMailBox::DecodePassword( char* apcPassword, apl_size_t auBufferSize )
{
    /*
    if (apl_strncmp(apcPopPassword, CRYPT_PREFIX, strlen(CRYPT_PREFIX) ) == 0 ) 
    {    
        //The password has crypt, must decrypt it
        char* lpcTemp = (char*)gsDesDecrypt(apcPassword + strlen(CRYPT_PREFIX), this->moUID.c_str() );
        if( lpcTemp != NULL )
        {
            apl_strncpy(apcPassword, lacTemp, auBufferSize);

            ACL_FREE(lpcTmp);
        }
    }
    */
    return apcPassword;
}

apl_int_t CUserMailBox::PostMail( std::string const& aoContent )
{
    /*
    clsDataList loRecipients;
    clsDataList loErrMsgs;

    char lacTemp[256];
    apl_snprintf(lacTemp, sizeof(lacTemp), "%s %"APL_PRIdINT" %s",
        this->moMBID.c_str(), 
        this->miMailRoot, 
        this->moUserName.c_str() );

    loRecipients.ciAppend(lacTemp);
    loErrMsgs.ciAppend("Error occured");
    
    if (giMAPIcDeliver(
        aoContent.c_str(), 
        this->moUserName, 
        (clsDataList*)&loRecipients, 
        (clsDataList*)&loErrMsgs, 
        MAPI_MODE_BUFFER ) != 0 )
    {
        //TODO ERROR
    }
    */
    return 0;
}

void CUserMailBox::CloseMailBox(void)
{
    /*
    ACL_DELETE(this->mpoMailBox);
    */
}

char const* CUserMailBox::GetUserName(void)
{
    return this->moUserName.c_str();
}

char const* CUserMailBox::GetUID(void)
{
    return this->moUID.c_str();
}

char const* CUserMailBox::GetMBID(void)
{
    return this->moMBID.c_str();
}

apl_int_t CUserMailBox::GetMailRoot(void)
{
    return this->miMailRoot;
}

apl_int_t CUserMailBox::GetMaxMailCount(void)
{
    return this->miMaxMailCount;
}

