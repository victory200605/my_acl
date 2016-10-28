
#include "GetMailTask.h"
#include "CmdLineMessage.h"
#include "acl/MemoryStream.h"
#include "acl/StrAlgo.h"

CGetMailTask::CGetMailTask( anf::SessionPtrType& aoSession, char const* apcUserName, apl_int_t aiPOPID, apl_int_t aiTraceLevel )
    : moSession(aoSession)
    , moUserMailBox(apcUserName)
    , miPOPID(aiPOPID)
    , miTraceLevel(aiTraceLevel)
{
}

CGetMailTask::~CGetMailTask(void)
{
}

void CGetMailTask::operator () (void)
{
    apl_int_t      liRetCode = 0;
    CPOPClient     loClient;
    std::string    loContent;
    std::string    loNewUidls;
    CPOPAccountInfo           loPOPAccountInfo;
    std::vector<CPOPUidlInfo> loAllUidls;
    std::vector<std::string>  loHeader;
    acl::CTimeValue loTimedout;

    //Lock user mailbox and get all config item 
    if ( (liRetCode = this->StartPopHandle(&loPOPAccountInfo) ) != 0)
    {
        WRITE_MESSAGE(true, this->moSession, "-ERR %s(start)\r\n", StrError(liRetCode) );
        EMP_LOG_ERROR("Starting handle GetMail task fail,%s (Errno=%"APL_PRIdINT")", StrError(liRetCode), apl_get_errno() );
        return;
    }

    do
    {
        if (loClient.Connect(
            loPOPAccountInfo.moHostName.c_str(),
            loPOPAccountInfo.miPort,
            loPOPAccountInfo.moUser.c_str(),
            loPOPAccountInfo.moPass.c_str(),
            loTimedout ) != 0)
        {
            WRITE_MESSAGE(true, this->moSession, "-ERR Remote host connection refused\r\n");
            EMP_LOG_ERROR("Handle GetMail task fail, Remote host Connection refused (Errno=%"APL_PRIdINT")", apl_get_errno() );
            break;
        }

        if (loClient.Uidl(&loAllUidls, loTimedout) != 0)
        {
            WRITE_MESSAGE(true, this->moSession, "-ERR UIDL command fail\r\n");
            EMP_LOG_ERROR("Handle GetMail task fail, %s (Errno=%"APL_PRIdINT")", loClient.GetLastMessage(), apl_get_errno() );
            break;
        }

        //Check and Download new mail
        for (std::vector<CPOPUidlInfo>::iterator loIter = loAllUidls.begin();
            loIter != loAllUidls.end(); ++loIter)
        {
            if (this->IsDownloaded(loIter->GetID() ) )
            {
                continue;
            }

            {//Check mail box limit
                CPOPListInfo loListInfo;
                if (loClient.List(loIter->GetNum(), &loListInfo, loTimedout) != 0)
                {
                    WRITE_MESSAGE(true, this->moSession, "-ERR LIST %"APL_PRIuINT" command fail\r\n", loIter->GetNum() );
                    EMP_LOG_ERROR("Handle GetMail task fail, %s (Errno=%"APL_PRIdINT")", loClient.GetLastMessage(), apl_get_errno() );
                    break;
                }

                if (!this->CheckMailBoxLimit(loListInfo.GetSize() ) )
                {
                    WRITE_MESSAGE(true, this->moSession, "-ERR Haven't not enough space\r\n");
                    EMP_LOG_ERROR("Handle GetMail task fail, Haven't not enough space (Errno=%"APL_PRIdINT")", apl_get_errno() );
                    break;
                }
            }

            {//Check mail header route info
                if (loClient.Top(loIter->GetNum(), 0, &loHeader, loTimedout) != 0)
                {
                    WRITE_MESSAGE(true, this->moSession, "-ERR TOP %"APL_PRIuINT" command fail\r\n", loIter->GetNum() );
                    EMP_LOG_ERROR("Handle GetMail task fail, %s (Errno=%"APL_PRIdINT")", loClient.GetLastMessage(), apl_get_errno() );
                    break;
                }

                //Check mail header route info
                if (!this->CheckAndModifyMailHeader(loPOPAccountInfo, loHeader) )
                {
                    continue;
                } 
            }

            //Download new mail now
            if (loClient.Retr(loIter->GetNum(), (std::string*)APL_NULL, &loContent, loTimedout) != 0)
            {
                WRITE_MESSAGE(true, this->moSession, "-ERR RETR %"APL_PRIuINT" command fail\r\n", loIter->GetNum() );
                EMP_LOG_ERROR("Handle GetMail task fail, %s (Errno=%"APL_PRIdINT")", loClient.GetLastMessage(), apl_get_errno() );
                break;
            }

            //Post mail
            if ( (liRetCode = this->PostMail(loHeader, loContent) ) != 0)
            {
                WRITE_MESSAGE(true, this->moSession, "-ERR %s\r\n", StrError(liRetCode) );
                EMP_LOG_ERROR("Handle GetMail task post mail fail,%s (Errno=%"APL_PRIdINT")", StrError(liRetCode), apl_get_errno() );
                break;
            }

            //Insert UIDL into new mail list
            loNewUidls += loIter->GetID();
            loNewUidls += "\n";
        }
    }
    while(false);

    //Complete and unlock user mailbox pop update
    if ( (liRetCode = this->EndPopHandle(loNewUidls) ) != 0)
    {
        WRITE_MESSAGE(true, this->moSession, "-ERR %s(end)\r\n", StrError(liRetCode) );
        EMP_LOG_ERROR("Ending handle GetMail task fail,%s (Errno=%"APL_PRIdINT")", StrError(liRetCode), apl_get_errno() );
    }
}

apl_int_t CGetMailTask::StartPopHandle( CPOPAccountInfo* apoInfo )
{
    apl_int_t liRetCode = 0;

    do
    {
        if (this->moUserMailBox.Init() != 0)
        {
            liRetCode = EMP_ERROR_INIT_MAILBOX;
            break;
        }

        if (this->moUserMailBox.OpenMailBox() != 0)
        {
            liRetCode = EMP_ERROR_OPEN_MAILBOX;
            break;
        }

        //Lock current POP account
        if (this->moUserMailBox.LockPOPAccount(this->miPOPID) != 0)
        {
            liRetCode = EMP_ERROR_LOCK_ACCOUNT;
            break;
        }

        //Load POP account info
        if (this->moUserMailBox.GetPOPAccountInfo(this->miPOPID, apoInfo) != 0)
        {
            liRetCode = EMP_ERROR_READ_ACCOUNT_INFO;
            break;
        }

        //Make uidl file name
        acl::CMemoryBlock loBuffer(APL_NULL, 0, acl::CMemoryBlock::DONT_DELETE);
        acl::CMemoryStream loStream(&loBuffer);
        std::string loLine;

        if (this->moUserMailBox.ReadUidlFile(this->miPOPID, &loBuffer) != 0)
        {
            liRetCode = EMP_ERROR_READ_UIDLLIST;
            break;
        }
        
        //Load current user mail list
        while(loStream.ReadLine(loLine) > 0)
        {
            this->InsertDownloaded(loLine);
        }
    }
    while(false);

    this->moUserMailBox.CloseMailBox();

    return liRetCode;
}

apl_int_t CGetMailTask::EndPopHandle( std::string const& aoUidlList )
{
    apl_int_t liRetCode = 0;

    do
    {
        if (this->moUserMailBox.OpenMailBox() != 0)
        {
            liRetCode = EMP_ERROR_OPEN_MAILBOX;
            break;
        }

        //Make uidl file name
        if (this->moUserMailBox.AppendUidlFile(this->miPOPID, aoUidlList) != 0)
        {
            liRetCode = EMP_ERROR_APPEND_UIDLLIST;
            break;
        }

        //Lock current POP account
        if (this->moUserMailBox.UnlockPOPAccount(this->miPOPID) != 0)
        {
            liRetCode = EMP_ERROR_UNLOCK_ACCOUNT;
            break;
        }
    }
    while(false);

    this->moUserMailBox.CloseMailBox();

    return liRetCode;
}

bool CGetMailTask::CheckMailBoxLimit( apl_size_t auMailSize )
{
    bool lbIsPass = true;
    do
    {
        if (this->moUserMailBox.OpenMailBox() != 0)
        {
            //liRetCode = EMP_ERROR_OPEN_MAILBOX;
            break;
        }
        
        apl_size_t luCount = 0;
        apl_size_t luSpace = 0;

        if (this->moUserMailBox.GetStat(&luCount, &luSpace) != 0)
        {
            //liRetCode = EMP_ERROR_STAT_MAILBOX;
            break;
        }

        if (this->moUserMailBox.GetMaxMailCount() > 0 && luCount >= (apl_size_t)this->moUserMailBox.GetMaxMailCount() )
        {
            lbIsPass = false;
        }

        if (auMailSize > luSpace)
        {
            lbIsPass = false;
        }
    }
    while(false);

    this->moUserMailBox.CloseMailBox();

    return lbIsPass;
}

bool CGetMailTask::CheckAndModifyMailHeader( CPOPAccountInfo const& aoInfo,  std::vector<std::string>& aoHeader )
{
    char lacTemp[EMP_LINE_MAX];
    apl_int_t liIndex1 = -1;
    apl_int_t liIndex2 = -1;
    
    //Search tag X-AIMC-EMPROXY/X-AIMC-POP2FID, if existed and this mail is pop from AIMC
    for (std::vector<std::string>::iterator loIter = aoHeader.begin();
        loIter != aoHeader.end() && (liIndex1 == -1 || liIndex2 == -1); ++loIter)
    {
        if (loIter->find(HEADER_TAG_AIMC) != std::string::npos)
        {
            liIndex1 = loIter - aoHeader.begin();
        }
        else if (loIter->find(HEADER_TAG_POPFID) != std::string::npos)
        {
            liIndex2 = loIter - aoHeader.begin();
        }
    }
    
    if (liIndex1 == -1)
    {
        apl_snprintf(lacTemp,sizeof(lacTemp), "%s: %s@%s,%s,\r\n", 
            HEADER_TAG_AIMC, 
            aoInfo.moUser.c_str(),
            aoInfo.moHostName.c_str(),
            this->moUserMailBox.GetUserName() );
                
        aoHeader.push_back(lacTemp);
    }
    else
    {
        acl::stralgo::EraseTail(aoHeader[liIndex1], 2);//erase CRLF
        aoHeader[liIndex1] += this->moUserMailBox.GetUserName();
        aoHeader[liIndex1] += "\r\n";
    }

    if (aoInfo.miFoldID > 0)
    {
        apl_snprintf(lacTemp,sizeof(lacTemp), "%s: %d\r\n", HEADER_TAG_POPFID, aoInfo.miFoldID);
        
        if (liIndex2 == -1)
        {
            aoHeader.push_back(lacTemp);
        }
        else
        {
            aoHeader[liIndex2] = lacTemp;
        }
    }
    else
    {
        if (liIndex2 >= 0)
        {
            aoHeader[liIndex2] = "";//clear
        }
    }

    return liIndex1 == -1 ? true : 
        (aoHeader[liIndex1].length() < EMP_LINE_MAX ? true : false);
}

apl_int_t CGetMailTask::PostMail( std::vector<std::string> const& aoHeader, std::string const& aoBody )
{
    std::string loContent;

    for (std::vector<std::string>::const_iterator loIter = aoHeader.begin();
        loIter != aoHeader.end(); ++loIter)
    {
        if (loIter->length() > 2)//Skip blank line
        {
            loContent.append(*loIter);
        }
    }

    loContent += "\r\n";
    loContent += aoBody;

    return this->moUserMailBox.PostMail(loContent);
}

bool CGetMailTask::IsDownloaded( std::string const& aoUidl )
{
    std::set<std::string>::iterator loIter = this->moDownloadeds.find(aoUidl);
    if (loIter == this->moDownloadeds.end() )
    {
        return false;
    }
    else
    {
        return true;
    }
}

void CGetMailTask::InsertDownloaded( std::string const& aoUidl )
{
    this->moDownloadeds.insert(aoUidl);
}

