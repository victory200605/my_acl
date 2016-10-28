
#include "POPClient.h"
#include "acl/SockConnector.h"
#include "acl/StrAlgo.h"

/////////////////////////////////////////////////////////////////////////////////////////////
CPOPListInfo::CPOPListInfo( apl_size_t auNum, apl_size_t auSize )
    : muNum(auNum)
    , muSize(auSize)
{
}
    
apl_int_t CPOPListInfo::Decode( char const* apcBuffer )
{
    //+OK 2 320
    char* lpcNext = APL_NULL;
    char const* lpcLast = apcBuffer + apl_strlen(apcBuffer);

    apl_int_t liNum = apl_strtoi32(apcBuffer, &lpcNext, 10);
    apl_int_t liSize = apl_strtoi32(lpcNext == APL_NULL ? "" : lpcNext + 1, APL_NULL, 10);
    if (lpcNext == APL_NULL 
        || lpcNext == apcBuffer //is not the first character
        || lpcNext == lpcLast //is not the end character
        || liNum <= 0
        || liSize <= 0 ) 
    {
        return -1;
    }

    this->muNum = liNum;
    this->muSize = liSize;

    return 0;
}

apl_size_t CPOPListInfo::GetNum(void) const
{
    return this->muNum;
}

apl_size_t CPOPListInfo::GetSize(void) const
{
    return this->muSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CPOPUidlInfo::CPOPUidlInfo( apl_size_t auNum, char const* apcID )
    : muNum(auNum)
    , moID(apcID)
{
}
    
apl_int_t CPOPUidlInfo::Decode( char const* apcBuffer )
{
    //+OK 2 QhdPYR:00WBw1Ph7x7
    char* lpcNext = APL_NULL;
    char const* lpcLast = apcBuffer + apl_strlen(apcBuffer);

    apl_int_t liNum = apl_strtoi32(apcBuffer, &lpcNext, 10);
    if (lpcNext == APL_NULL 
        || lpcNext == apcBuffer //is not the first character
        || lpcNext == lpcLast //is not the end character
        || lpcNext[0] != ' '
        || liNum < 0 )
    {
        return -1;
    }

    this->muNum = liNum;
    this->moID = lpcNext + 1;

    return 0;
}

apl_size_t CPOPUidlInfo::GetNum(void) const
{
    return this->muNum;
}

std::string const& CPOPUidlInfo::GetID(void) const
{
    return this->moID;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CPOPStatInfo::CPOPStatInfo( apl_size_t auCount, apl_size_t auSize )
    : muCount(auCount)
    , muSize(auSize)
{
}
    
apl_int_t CPOPStatInfo::Decode( char const* apcBuffer )
{
    //+OK 2 320
    char* lpcNext = APL_NULL;
    char const* lpcLast = apcBuffer + apl_strlen(apcBuffer);

    apl_int_t liCount = apl_strtoi32(apcBuffer, &lpcNext, 10);
    apl_int_t liSize = apl_strtoi32(lpcNext == APL_NULL ? "" : lpcNext + 1, APL_NULL, 10);
    if (lpcNext == APL_NULL 
        || lpcNext == apcBuffer //is not the first character
        || lpcNext == lpcLast //is not the end character
        || liCount < 0
        || liSize < 0 ) 
    {
        return -1;
    }

    this->muCount = liCount;
    this->muSize = liSize;

    return 0;
}

apl_size_t CPOPStatInfo::GetCount(void) const
{
    return this->muCount;
}

apl_size_t CPOPStatInfo::GetSize(void) const
{
    return this->muSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////
class CPOPRetrInfo : public IPOPResponse
{
public:
    CPOPRetrInfo(void)
        : muSize(0)
    {
    }

    virtual apl_int_t Decode( char const* apcBuffer )
    {
        //+OK 120 octets
        char* lpcNext = APL_NULL;
        char const* lpcLast = apcBuffer + apl_strlen(apcBuffer);

        apl_int_t liSize = apl_strtoi32(apcBuffer, &lpcNext, 10);
        if (lpcNext == APL_NULL 
            || lpcNext == apcBuffer //is not the first character
            || lpcNext == lpcLast //is not the end character
            || liSize <= 0 )
        {
            return -1;
        }

        this->muSize = liSize;

        return 0;
    }

    apl_size_t GetSize(void)
    {
        return this->muSize;
    }

private:
    apl_size_t muSize;
};

/////////////////////////////////////////////////////////////////////////////////////////////
static char const* POP_CRLF = "\r\n";
static const apl_size_t POP_BUFFER_SIZE = 512;
static const apl_size_t POP_LINE_MAX = 256;

#define POP_CLEAR_LAST_MESSAGE() this->macMessage[0] = '\0'

#define POP_IS_LINE_END(buffer, length) \
    (length >= 2 && ((buffer)[length - 2] == '\r' && (buffer)[length - 1] == '\n') )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPOPClient::CPOPClient(void)
{
}

CPOPClient::~CPOPClient(void)
{
    this->Close();
}

apl_int_t CPOPClient::Connect( 
    char const* apcHostName, 
    apl_uint16_t au16Port, 
    char const* apcUser, 
    char const* apcPass,
    const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    acl::CSockAddr loRemote;
    acl::CSockConnector loConnector;

    //Set remote address
    if (loRemote.Set(apcHostName) != 0 || loRemote.SetPort(au16Port) != 0)
    {
        apl_snprintf(this->macMessage, sizeof(this->macMessage),
            "Remote address invalid,%s", apl_strerror(apl_get_errno() ) );

        return -1;
    }

    //Connect to POP server
    if (loConnector.Connect(this->moPeer, loRemote, aoTimedout) != 0)
    {
        apl_snprintf(this->macMessage, sizeof(this->macMessage),
            "Connect fail,%s", apl_strerror(apl_get_errno() ) );
        
        return -1;
    }

    //Recv welcome message
    char lacBuffer[POP_BUFFER_SIZE];
    if (this->moPeer.RecvUntil(lacBuffer, sizeof(lacBuffer), POP_CRLF, aoTimedout) <= 0)
    {
        apl_snprintf(this->macMessage, sizeof(this->macMessage),
            "Recv welcome message fail,%s", apl_strerror(apl_get_errno() ) );

        return -1;
    }

    char lacCmd[64];
 
    //POP user login now
    apl_snprintf(lacCmd, sizeof(lacCmd), "%s %s", "USER", apcUser);
    if (this->SendCommand(lacCmd, APL_NULL, aoTimedout) != 0)
    {
        return -1;
    }

    apl_snprintf(lacCmd, sizeof(lacCmd), "%s %s", "PASS", apcPass);
    if (this->SendCommand(lacCmd, APL_NULL, aoTimedout) != 0)
    {
        return -1;
    }

    return 0;
}

void CPOPClient::Close(void)
{
    if (this->moPeer.GetHandle() != ACL_INVALID_HANDLE)
    {
        this->SendCommand("QUIT", APL_NULL, acl::CTimeValue(5) );

        this->moPeer.Close();
    }
}

apl_int_t CPOPClient::Stat( CPOPStatInfo* apoStatInfo, const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    return this->SendCommand("STAT", apoStatInfo, aoTimedout);
}

apl_int_t CPOPClient::List( ListResultType* apoResult, const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    //List [msg]
    //C: LIST
    //S: +OK 2 messages (320 octets)
    //S: 1 120
    //S: 2 200
    //S: .

    //Clear last record all
    apoResult->clear();

    apl_int_t liRetCode = this->SendCommand("LIST", APL_NULL, aoTimedout);
    if (liRetCode == 0)
    {
        //apl_size_t luCount = apl_strtoi32(loResponse.GetField(0), APL_NULL, 10);
        
        CPOPListInfo loListInfo;

        //Recv all message
        while(true)
        {
            char lacBuffer[64];
            apl_ssize_t liResult = this->moPeer.RecvUntil(lacBuffer, sizeof(lacBuffer), POP_CRLF, aoTimedout);
            if (!POP_IS_LINE_END(lacBuffer, liResult) )
            {
                //Fail
                apl_snprintf(this->macMessage, sizeof(this->macMessage),
                    "Recv LIST all message fail,%s", apl_strerror(apl_get_errno() ) );
                
                return -1;
            }
            else if (lacBuffer[0] == '.')
            {
                //Complete
                break;
            }
            else
            {
                lacBuffer[liResult - 2] = '\0';

                if (loListInfo.Decode(lacBuffer) != 0)
                {
                    //Fail
                    apl_snprintf(this->macMessage, sizeof(this->macMessage),
                        "Recv LIST all message fail,invalid format %s", lacBuffer);
                    
                    return -1;
                }
                else
                {
                    //Continue
                    apoResult->push_back(loListInfo);
                }
            }
        }
    }

    return liRetCode;
}

apl_int_t CPOPClient::List( apl_size_t auNum, CPOPListInfo* apoListInfo, const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    char lacCmd[64];
    apl_snprintf(lacCmd, sizeof(lacCmd), "LIST %"APL_PRIuINT, auNum);

    return this->SendCommand(lacCmd, apoListInfo, aoTimedout);
}

apl_int_t CPOPClient::Uidl( UidlResultType* apoResult, const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    //C: UIDL
    //S: +OK
    //S: 1 whqtswO00WBw418f9t5JxYwZ
    //S: 2 QhdPYR:00WBw1Ph7x7
    //S: .

    //Clear last record all
    apoResult->clear();

    apl_int_t liRetCode = this->SendCommand("UIDL", APL_NULL, aoTimedout);
    if (liRetCode == 0)
    {
        CPOPUidlInfo loUidlInfo;

        //Recv all message
        while(true)
        {
            char lacBuffer[64];
            apl_ssize_t liResult = this->moPeer.RecvUntil(lacBuffer, sizeof(lacBuffer), POP_CRLF, aoTimedout);
            if (!POP_IS_LINE_END(lacBuffer, liResult) )
            {
                //Fail
                apl_snprintf(this->macMessage, sizeof(this->macMessage),
                    "Recv UIDL all message fail,%s", apl_strerror(apl_get_errno() ) );
                
                return -1;
            }
            else if (lacBuffer[0] == '.')
            {
                //Complete
                break;
            }
            else
            {
                lacBuffer[liResult - 2] = '\0';
                
                if (loUidlInfo.Decode(lacBuffer) != 0) 
                {
                    //Fail
                    apl_snprintf(this->macMessage, sizeof(this->macMessage),
                        "Recv UIDL all message fail,invalid format %s", lacBuffer);
                    
                    return -1;
                }
                else
                {
                    //Continue
                    apoResult->push_back(loUidlInfo);
                }
            }
        }
    }

    return liRetCode;
}

apl_int_t CPOPClient::Uidl( apl_size_t auNum, CPOPUidlInfo* apoUidlInfo, const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    char lacCmd[64];
    apl_snprintf(lacCmd, sizeof(lacCmd), "UIDL %"APL_PRIuINT, auNum);

    return this->SendCommand(lacCmd, apoUidlInfo, aoTimedout);
}

apl_int_t CPOPClient::Uidl( char const* apcUidl, CPOPUidlInfo* apoUidlInfo, const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    //C: UIDL
    //S: +OK
    //S: 1 whqtswO00WBw418f9t5JxYwZ
    //S: 2 QhdPYR:00WBw1Ph7x7
    //S: .

    apl_int_t liRetCode = this->SendCommand("UIDL", APL_NULL, aoTimedout);
    if (liRetCode == 0)
    {
        //Recv all message
        while(true)
        {
            char lacBuffer[64];
            apl_ssize_t liResult = this->moPeer.RecvUntil(lacBuffer, sizeof(lacBuffer), POP_CRLF, aoTimedout);
            if (!POP_IS_LINE_END(lacBuffer, liResult) )
            {
                //Fail
                apl_snprintf(this->macMessage, sizeof(this->macMessage),
                    "Recv UIDL all message fail,%s", apl_strerror(apl_get_errno() ) );
                
                return -1;
            }
            else if (lacBuffer[0] == '.')
            {
                //Complete
                break;
            }
            else
            {
                lacBuffer[liResult - 2] = '\0';
                
                if (apoUidlInfo->Decode(lacBuffer) != 0) 
                {
                    //Fail
                    apl_snprintf(this->macMessage, sizeof(this->macMessage),
                        "Recv UIDL all message fail,invalid format %s", lacBuffer);
                    
                    return -1;
                }
                else
                {
                    if (apoUidlInfo->GetID() == apcUidl)
                    {
                        //Hit
                        break;
                    }
                    //Continue
                }
            }
        }
    }

    return liRetCode;
}

apl_int_t CPOPClient::RecvMail( bool abIsHeaderOnly, std::string* apoContent, const acl::CTimeValue& aoTimedout )
{
    bool lbIsBlankLine = false;
    char lacBuffer[POP_LINE_MAX];

    while(true)
    {
        apl_ssize_t liResult = this->moPeer.RecvUntil(
            lacBuffer, sizeof(lacBuffer), POP_CRLF, aoTimedout);
        if (POP_IS_LINE_END(lacBuffer, liResult) )
        {
            if (abIsHeaderOnly)
            {
                if (liResult == 2)
                {
                    break;
                }
            }
            else
            {
                if (liResult == 3 && lacBuffer[0] == '.' && lbIsBlankLine)
                {
                    if (apoContent != APL_NULL)
                    {
                        acl::stralgo::EraseTail(*apoContent, 2);
                    }
                    break;
                }
                else if (liResult == 2)
                {
                    lbIsBlankLine = true;
                }
                else
                {
                    lbIsBlankLine = false;
                }
            }
            
            if (apoContent  != APL_NULL)
            {
                apoContent->append(lacBuffer, liResult);
            }
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

apl_int_t CPOPClient::RecvMail( bool abIsHeaderOnly, std::vector<std::string>* apoContent, const acl::CTimeValue& aoTimedout )
{
    bool lbIsBlankLine = false;
    char lacBuffer[POP_LINE_MAX];

    while(true)
    {
        apl_ssize_t liResult = this->moPeer.RecvUntil(
            lacBuffer, sizeof(lacBuffer), POP_CRLF, aoTimedout);
        if (POP_IS_LINE_END(lacBuffer, liResult) )
        {
            if (abIsHeaderOnly)
            {
                if (liResult == 2)
                {
                    break;
                }
            }
            else
            {
                if (liResult == 3 && lacBuffer[0] == '.' && lbIsBlankLine)
                {
                    if (apoContent != APL_NULL)
                    {
                        apoContent->erase(--apoContent->end() );
                    }
                    break;
                }
                else if (liResult == 2)
                {
                    lbIsBlankLine = true;
                }
                else
                {
                    lbIsBlankLine = false;
                }
            }
            
            if (apoContent != APL_NULL)
            {
                apoContent->push_back(std::string(lacBuffer, liResult) );
            }
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

apl_int_t CPOPClient::Top( apl_size_t auNum, apl_size_t auN, std::string* apoContent, const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    char lacCmd[64];
    apl_snprintf(lacCmd, sizeof(lacCmd), "TOP %"APL_PRIuINT" %"APL_PRIuINT, auNum, auN);

    *apoContent = "";

    apl_int_t liRetCode = this->SendCommand(lacCmd, APL_NULL, aoTimedout);
    if (liRetCode == 0)
    {
        if (this->RecvMail(false, apoContent, aoTimedout) != 0)
        {
            //Fail
            apl_snprintf(this->macMessage, sizeof(this->macMessage),
                "Recv TOP command content fail,%s", apl_strerror(apl_get_errno() ) );
            
            return -1;
        }
    }

    return liRetCode;
}

apl_int_t CPOPClient::Top(
    apl_size_t auNum,
    apl_size_t auN,
    std::vector<std::string>* apoContent,
    const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    char lacCmd[64];
    apl_snprintf(lacCmd, sizeof(lacCmd), "TOP %"APL_PRIuINT" %"APL_PRIuINT, auNum, auN);

    apoContent->clear();

    apl_int_t liRetCode = this->SendCommand(lacCmd, APL_NULL, aoTimedout);
    if (liRetCode == 0)
    {
        if (this->RecvMail(false, apoContent, aoTimedout) != 0)
        {
            //Fail
            apl_snprintf(this->macMessage, sizeof(this->macMessage),
                "Recv TOP command content fail,%s", apl_strerror(apl_get_errno() ) );
            
            return -1;
        }
    }

    return liRetCode;
}

apl_int_t CPOPClient::Retr( apl_size_t auNum, std::string* apoContent, const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    //CPOPRetrInfo loRetrInfo;
    char lacCmd[64];
    apl_snprintf(lacCmd, sizeof(lacCmd), "RETR %"APL_PRIuINT, auNum);

    *apoContent = "";

    apl_int_t liRetCode = this->SendCommand(lacCmd, APL_NULL, aoTimedout);
    if (liRetCode == 0)
    {
        if (this->RecvMail(false, apoContent, aoTimedout) != 0)
        {
            //Fail
            apl_snprintf(this->macMessage, sizeof(this->macMessage),
                "Recv RETR command content fail,%s", apl_strerror(apl_get_errno() ) );
            
            return -1;
        }
    }

    return liRetCode;
}

apl_int_t CPOPClient::Retr( apl_size_t auNum, std::string* apoHeader, std::string* apoBody, const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    //CPOPRetrInfo loRetrInfo;
    char lacCmd[64];
    apl_snprintf(lacCmd, sizeof(lacCmd), "RETR %"APL_PRIuINT, auNum);

    *apoHeader = "";
    *apoBody = "";

    apl_int_t liRetCode = this->SendCommand(lacCmd, APL_NULL, aoTimedout);
    if (liRetCode == 0)
    {
        if (this->RecvMail(true, apoHeader, aoTimedout) != 0)
        {
            //Fail
            apl_snprintf(this->macMessage, sizeof(this->macMessage),
                "Recv RETR command header fail,%s", apl_strerror(apl_get_errno() ) );
            
            return -1;
        }
        
        if (this->RecvMail(false, apoBody, aoTimedout) != 0)
        {
            //Fail
            apl_snprintf(this->macMessage, sizeof(this->macMessage),
                "Recv RETR command body fail,%s", apl_strerror(apl_get_errno() ) );
            
            return -1;
        }
    }

    return liRetCode;
}

apl_int_t CPOPClient::Retr(
    apl_size_t auNum,
    std::vector<std::string>* apoHeader,
    std::string* apoBody,
    const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    //CPOPRetrInfo loRetrInfo;
    char lacCmd[64];
    apl_snprintf(lacCmd, sizeof(lacCmd), "RETR %"APL_PRIuINT, auNum);

    apoHeader->clear();
    *apoBody = "";

    apl_int_t liRetCode = this->SendCommand(lacCmd, APL_NULL, aoTimedout);
    if (liRetCode == 0)
    {
        if (this->RecvMail(true, apoHeader, aoTimedout) != 0)
        {
            //Fail
            apl_snprintf(this->macMessage, sizeof(this->macMessage),
                "Recv RETR command header fail,%s", apl_strerror(apl_get_errno() ) );
            
            return -1;
        }
        
        if (this->RecvMail(false, apoBody, aoTimedout) != 0)
        {
            //Fail
            apl_snprintf(this->macMessage, sizeof(this->macMessage),
                "Recv RETR command body fail,%s", apl_strerror(apl_get_errno() ) );
            
            return -1;
        }
    }

    return liRetCode;
}

apl_int_t CPOPClient::Rset( const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    return this->SendCommand("RSET", APL_NULL, aoTimedout);
}

apl_int_t CPOPClient::Dele( apl_size_t auNum, const acl::CTimeValue& aoTimedout )
{
    POP_CLEAR_LAST_MESSAGE();

    char lacCmd[64];
    apl_snprintf(lacCmd, sizeof(lacCmd), "DELE %"APL_PRIuINT, auNum);

    return this->SendCommand(lacCmd, APL_NULL, aoTimedout);
}

apl_int_t CPOPClient::SendCommand( 
    char const* apcCmd, 
    IPOPResponse* apoResponse, 
    const acl::CTimeValue& aoTimedout )
{
    apl_ssize_t liLength = 0;
    char lacBuffer[POP_BUFFER_SIZE];

    //Make List command line
    apl_snprintf(lacBuffer, sizeof(lacBuffer), "%s%s", apcCmd, POP_CRLF);
    liLength = apl_strlen(lacBuffer);

    if (this->moPeer.Send(lacBuffer, liLength, aoTimedout) != liLength)
    {
        apl_snprintf(this->macMessage, sizeof(this->macMessage),
            "Send %s command request fail,%s", apcCmd, apl_strerror(apl_get_errno() ) );
        
        return -1;
    }

    //Recv response
    if ( (liLength= this->moPeer.RecvUntil(lacBuffer, sizeof(lacBuffer), POP_CRLF, aoTimedout) ) <= 0)
    {
        apl_snprintf(this->macMessage, sizeof(this->macMessage),
            "Recv %s command response fail,%s", apcCmd, apl_strerror(apl_get_errno() ) );

        return -1;
    }

    //Response must end of CRLF
    if (!POP_IS_LINE_END(lacBuffer, liLength) )
    {
        apl_snprintf(this->macMessage, sizeof(this->macMessage),
            "Recv %s command unknow error", apcCmd);
        
        return -1;
    }
    else
    {
        lacBuffer[liLength - 2] = '\0';
    }

    //Check response format
    if (liLength >= 4 && apl_strncmp(lacBuffer, "+OK", 3) == 0)
    {
        if (apoResponse == APL_NULL)
        {
            return 0;
        }
        
        if (apoResponse->Decode(lacBuffer + 4) != 0)
        {
            apl_snprintf(this->macMessage, sizeof(this->macMessage), 
                "Recv %s command response invalid,%s", apcCmd, lacBuffer);

            return -1;
        }
        else
        {
            return 0;
        }
    }
    else if (liLength >= 5 && apl_strncmp(lacBuffer, "-ERR", 4) == 0)
    {
        apl_strncpy(this->macMessage, lacBuffer + 5, sizeof(this->macMessage) );

        return -1;
    }
    else
    {
        apl_strncpy(this->macMessage, lacBuffer, sizeof(this->macMessage) );

        return -1;
    }
}

char const* CPOPClient::GetLastMessage(void)
{
    return this->macMessage;
}

