
#include "POPHandler.h"

////////////////////////////////////////////////////////////////////////////////////////////
CPOPConfigInfo::CPOPConfigInfo(void)
    : miPort(0)
{
}

bool CPOPConfigInfo::IsDownloaded( std::string const& aoID )
{
    std::set<std::string>::iterator loIter = this->moDownloadeds.find(aoID);
    if (loIter == this->moDownloadeds.end() )
    {
        return false;
    }
    else
    {
        return true;
    }
}

void CPOPConfigInfo::InsertDownloaded( std::string const& aoID )
{
    this->moDownloadeds.insert(aoID);
}

void CPOPConfigInfo::SetUser( char const* apcUser )
{
    this->moUser = apcUser;
}

char const* CPOPConfigInfo::GetUser(void)
{
    return this->moUser.c_str();
}

void CPOPConfigInfo::SetPass( char const* apcPass )
{
    this->moPass = apcPass;
}

char const* CPOPConfigInfo::GetPass(void)
{
    return this->moPass.c_str();
}

void CPOPConfigInfo::SetHostName( char const* apcHostName )
{
    this->moHostName = apcHostName;
}

char const* CPOPConfigInfo::GetHostName(void)
{
    return this->moHostName.c_str();
}

void CPOPConfigInfo::SetPort( apl_int_t aiPort )
{
    this->miPort = aiPort;
}

apl_int_t CPOPConfigInfo::GetPort(void)
{
    return this->miPort;
}

void CPOPConfigInfo::SetDownloadAndDelete( bool abIsEnable )
{
    this->mbIsDownloadAndDelete = abIsEnable;
}

bool CPOPConfigInfo::IsDownloadAndDelete(void)
{
    return this->mbIsDownloadAndDelete;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CPOPHandler::CPOPHandler( char const* apcUID, char const* apcMBID, apl_int_t aiMailRoot )
    : moUID(apcUID)
    , moMBID(apcMBID)
    , miMailRoot(aiMailRoot)
{
}

CPOPHandler::~CPOPHandler(void)
{
}

void CPOPHandler::operator () (void)
{
    CPOPClient     loClient;
    std::string    loContent;
    CPOPConfigInfo loPOPConfigInfo;
    std::vector<CPOPUidlInfo> loAllUidls;
    std::vector<CPOPUidlInfo> loNewUidls;
    acl::CTimeValue loTimedout;

    //Lock user mailbox and get all config item 
    if (this->StartPopHandle(loPOPConfigInfo) )
    {
        //TODO ERROR
    }

    do
    {
        if (loClient.Connect(
            loPOPConfigInfo.GetHostName(),
            loPOPConfigInfo.GetPort(),
            loPOPConfigInfo.GetUser(),
            loPOPConfigInfo.GetPass(),
            loTimedout ) != 0)
        {
            //TODO ERROR
            break;
        }

        if (loClient.Uidl(loAllUidls, loTimedout) != 0)
        {
            //TODO ERROR
            break;
        }

        //Check and Download new mail
        for (std::vector<CPOPUidlInfo>::iterator loIter = loAllUidls.begin();
            loIter != loAllUidls.end(); ++loIter)
        {
            if (loPOPConfigInfo.IsDownloaded(loIter->GetID() ) )
            {
                continue;
            }

            {//Check mail box limit
                CPOPListInfo loListInfo;
                if (loClient.List(loIter->GetNum(), loListInfo, loTimedout) != 0)
                {
                    //TODO ERROR
                    break;
                }

                if (!this->CheckMailBoxLimit(loListInfo.GetSize() ) )
                {
                    //TODO Limited
                    break;
                }
            }

            {//Check mail header route info
                if (loClient.Top(loIter->GetNum(), 0, loContent, loTimedout) != 0)
                {
                    //TODO ERROR
                    break;
                }

                if (!this->CheckMailHeader(loContent) )
                {
                    //TODO Skip
                    continue;
                }
            }

            //Download new mail now
            if (loClient.Retr(loIter->GetNum(), loContent, loTimedout) != 0)
            {
                //TODO ERROR
                break;
            }

            //Post mail
            if (this->PostMail(loContent) != 0)
            {
                //TODO ERROR
                break;
            }

            //Insert UIDL into new mail list
            loNewUidls.push_back(*loIter);
        }
    }
    while(false);

    //Complete and unlock user mailbox pop update
    if (this->EndPopHandle(loNewUidls) != 0)
    {
        //TODO ERROR
    }
}

apl_int_t CPOPHandler::StartPopHandle( CPOPConfigInfo& aoConfigInfo )
{
    clsMailBox* lpcMailBox = APL_NULL;
    ACL_ASSERT_NEW(lpcMailBox, 
        clsMailBox(this->moMBID.c_str(), this->moUserName.c_str(), this->miMailRoot) );
    
    do
    {
        if (lpcMailBox->csGetErrMsg() != APL_NULL)
        {
            //TODO ERROR
            break;
        }

        char lacTemp[64];
        char lacValue[512];
        
        //Make field name
        apl_snprintf(lacTemp, sizeof(lacTemp), "%s%d", EXTERNAL_POP_KEY, this->miPopItemNo);

        //Load POP user
        lpoMailbox->csGetSetting(lacTemp, USERKEY, lacValue, sizeof(lacValue) );
        aoConfigInfo.SetUser(lacValue);

        //Load POP pass
        lpoMailbox->csGetSetting(lacTemp, PASSKEY, lacValue, sizeof(lacValue) );
        aoConfigInfo.SetPass(this->DecodePassword(lacValue, sizeof(lacValue) ) );

        //Load POP delemail
        lpoMailbox->csGetSetting(lacTemp, DELMAILKEY, lacValue, sizeof(lacValue) );
        aoConfigInfo.SetDownloadAndDelete(apl_strtoi32(lacValue, APL_NULL, 10) );
        
        //Load POP fid
        lpoMailbox->csGetSetting(lacTemp, POPFID, lacValue, sizeof(lacValue) );
        aoConfigInfo.SetFoldID(apl_strtoi32(lacValue, APL_NULL, 10) );

        //Load POP hostname
        lpoMailbox->csGetSetting(lacTemp, SERVERKEY, lacValue, sizeof(lacValue) );
        aoConfigInfo.SetHostName(lacValue);

        //Load POP port
        lpoMailbox->csGetSetting(lacTemp, PORTKEY, lacValue, sizeof(lacValue) );
        aoConfigInfo.SetPort(apl_strtoi32(lacValue, APL_NULL, 10) );
        
        //Mail box size limit
        MailBoxAttr_t loMailboxAttr;
        lpoMailbox->ciGetAttrib(&loMailboxAttr);
        this->miMaxSpaceSize = loMailboxAttr.iSizeLimit;

        //Mail box count limit
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
        
        //Make uidl file name
        apl_snprintf(lacTemp, sizeof(lacTemp), "%s%d.uidl", LOCAL_UIDL_FILENAME, this->miPopItemNo);
        apl_int_t liFileSize = lpoMailbox->ciGetDataFileSize(lacTemp);
        if (liFileSize < 0 )
        {
            //TODO ERROR
        }
        else if (liFileSize > 0)
        {
            //Load current user mail list
            acl::CMemoryBlock  loBlock(liFileSize);
            acl::CMemoryStream loStream(&loBlock);
            std::string loLine;

            if (lpoMailbox->ciReadDataFile(lacTemp, loBlock.GetWritePtr(), liFileSize) != liFileSize)
            {
                //TODO ERROR
            }
            
            loBlock.SetWritePtr(liFileSize);

            while(loStream.ReadLine(loLine) > 0)
            {
                aoConfigInfo.InsertDownloaded(loLine);
            }
        }
    }
    while(false);

    ACL_DELETE(lpcMailBox);

    return 0;
}

apl_int_t CPOPHandler::EndPopHandle( std::vector<CPOPUidlInfo> const& aoNewUidls )
{
    std::string loUidlList;

    {//Make uidl list string
        loUidlList.reserve(4096);

        for (std::vector<CPOPUidlInfo>::iterator loIter = aoNewUidls.begin();
            loIter != aoNewUidls.end(); ++loIter)
        {
            loUidlList.append(loIter->GetID() );
            loUidlList.append('\n');
        }
    }

    //Open mail box and write file
    clsMailBox* lpcMailBox = APL_NULL;
    ACL_ASSERT_NEW(lpcMailBox, 
        clsMailBox(this->moMBID.c_str(), this->moUserName.c_str(), this->miMailRoot) );
    
    do
    {
        if (lpcMailBox->csGetErrMsg() != APL_NULL)
        {
            //TODO ERROR
            break;
        }

        //Make uidl file name
        char lacTemp[128];
        apl_snprintf(lacTemp, sizeof(lacTemp), "%s%d.uidl", LOCAL_UIDL_FILENAME, this->miPopItemNo);

        if (lpoMailbox->ciWriteDataFile(
            this->macUidl, loUidlList.c_str(), loUidlList.length(), 0, DF_APPEND) != loUidlList.length() )
        {
            //TODO ERROR
            break;
        }
    }
    while(false);
    
    ACL_DELETE(lpcMailBox);

    return 0;
}

bool CPOPHandler::CheckMailBoxLimit( apl_size_t auMailSize )
{
    bool lbIsPass = true;
    clsMailBox* lpcMailBox = APL_NULL;
    ACL_ASSERT_NEW(lpcMailBox, 
        clsMailBox(this->moMBID.c_str(), this->moUserName.c_str(), this->miMailRoot) );
    
    do
    {
        if (lpcMailBox->csGetErrMsg() != APL_NULL)
        {
            //TODO ERROR
            break;
        }
        
        MailBoxAttr_t loMailboxAttr;
        lpoMailbox->ciGetAttrib(&loMailboxAttr);
        if(loMailboxAttr.iTotalCount >= this->miMaxMailCount)
        {
            lbIsPass = false;
        }
        else if ( (loMailboxAttr.iSizeLimit - loMailboxAttr.iSizeOccupied) < auMailSize)
        {
            lbIsPass = false;
        }
    }
    while(false);
    
    ACL_DELETE(lpcMailBox);

    return lbIsPass;
}

bool CPOPHandler::CheckAndModifyMailHeader( std::string& aoHeader )
{
    //Search tag X-AIMC-EMPROXY, if existed and this mail is pop from AIMC
    std::string::size_type luPos = aoHeader.find(HEADTAG);
    if (luPos == std::string::npos)
    {
        char lacTemp[128];
        snprintf(lacTemp,sizeof(lacTemp), "%s: %s@%s,%s,%s", 
            HEADTAG, 
            this->moUser.c_str(),
            this->moHostName.c_str(),
            this->moUserName.c_str(),
            POP_CRLF );

        aoHeader.append(lacTemp);

        return true;
    }

    std::string::size_type luEndPos = aoHeader.find(POP_CRLF, luPos);
    if (luEndPos == std::string::npos)
    {
        //TODO ERROR

        return false;
    }

    //Search my name, if existed and this mail is pop from my account
    luPos = aoHeader.find(this->moUserName, luPos);
    if (luPos == std::string::npos)
    {
        return true;
    }

    return false;
}

apl_int_t CPOPHandler::PostMail( std::string& aoContent )
{
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

    return 0;
}

char const* CPOPHandler::DecodePassword( char* apcPassword, apl_size_t auBufferSize )
{
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
    
    return apcPassword;
}

