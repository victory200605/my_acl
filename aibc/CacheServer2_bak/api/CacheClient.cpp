
#include "acl/IniConfig.h"
#include "acl/MemoryStream.h"
#include "cacheserver2/CacheClient.h"

AIBC_CACHESERVER_NAMESPACE_START

// Hash string
inline size_t HashString( const char* apcString, size_t aiLength ) 
{
    //unsigned long llH = 0; 
    unsigned int    liRetValue = 1;
    unsigned int    liTempValue = 4;

    while( aiLength-- ) /* from MySQL */
    {
        liRetValue ^= (((liRetValue & 63)+liTempValue)*((unsigned int)(unsigned char) *apcString++))+ (liRetValue << 8);
        liTempValue += 3;
    }

    return liRetValue;
}

//Request command
enum {
    CACHESERVER_REQUEST_PUT            = 0x0001,
    CACHESERVER_RESPONSE_PUT           = 0x8001,
    CACHESERVER_REQUEST_GET            = 0x0002,
    CACHESERVER_RESPONSE_GET           = 0x8002,
    CACHESERVER_REQUEST_GETTIMEOUT     = 0x0004,
    CACHESERVER_RESPONSE_GETTIMEOUT    = 0x8004
};

CCacheClient::CCacheClient(void)
    : moClientMap(100, (ClientType*)NULL)
{
}

CCacheClient::~CCacheClient(void)
{
    this->Close();
}

apl_int_t CCacheClient::Initialize(void)
{
    //Load cache server info
    for (std::vector<CServerInfo>::iterator loIter = this->moServerInfos.begin();
         loIter != this->moServerInfos.end(); ++loIter )
    {
        if (!this->CheckHashRangeValue(loIter->muHashLower, loIter->muHashUpper) )
        {
            apl_errprintf("Hash range value invalid, (Lower=%"APL_PRIuINT", Upper=%"APL_PRIuINT")\n",
                loIter->muHashLower, loIter->muHashUpper);

            return -1;
        }

        ClientType* lpoClient = NULL;
        ACL_NEW_ASSERT(lpoClient, ClientType);

        lpoClient->GetFilterChain()->AddFirst("simple-async-message", new anf::CSimpleAsyncMessageFilter);
        lpoClient->GetConfiguration()->SetProcessorPoolSize(loIter->muProcessorPoolSize);

        if (lpoClient->Initialize(loIter->muWindSize, loIter->moTimeout) != 0)
        {
            ACL_DELETE(lpoClient);
            return -1;
        }

        lpoClient->Connect("main", loIter->moRemoteAddress, loIter->muConnNum);

        this->SetServer(loIter->muHashLower, loIter->muHashUpper, lpoClient);
    }

    apl_int_t liUndefineValue = 0;
    if (!this->CheckAllHashRangeValue(&liUndefineValue) )
    {
        apl_errprintf("Hash range value undefine, (UndefineValue=%"APL_PRIdINT")", liUndefineValue);

        return -1;
    }

    return 0;
}

void CCacheClient::Close(void)
{
    for (std::vector<ClientType*>::iterator loIter = this->moClients.begin();
         loIter != this->moClients.end(); ++loIter)
    {
        ClientType* lpoClient = *loIter;
        lpoClient->Close();
        ACL_DELETE(lpoClient);
    }

    this->moClients.clear();
}

void CCacheClient::AddServer(
    acl::CSockAddr const& aoRemoteAddress,
    apl_size_t auConnNum,
    apl_size_t auProcessorPoolSize,
    apl_size_t auHashLower,
    apl_size_t auHashUpper,
    acl::CTimeValue const& aoTimeout,
    apl_size_t auWindSize )
{
    CServerInfo loServerInfo;

    loServerInfo.moRemoteAddress.Set(aoRemoteAddress);
    loServerInfo.muConnNum = auConnNum;
    loServerInfo.muProcessorPoolSize = auProcessorPoolSize;
    loServerInfo.muHashLower = auHashLower;
    loServerInfo.muHashUpper = auHashUpper;
    loServerInfo.muWindSize = auWindSize;
    loServerInfo.moTimeout = aoTimeout;

    this->moServerInfos.push_back(loServerInfo);
}

apl_int_t CCacheClient::Put( 
    char const* apcKey, 
    apl_size_t auKeyLen, 
    char const* apcValue, 
    apl_size_t auValueLen, 
    acl::CTimeValue const& aoTimeout )
{
    anf::CSimpleAsyncMessage  loRequest;
    anf::CSimpleAsyncMessage* lpoResponse = NULL;
    acl::CMemoryStream loOutput(&loRequest.GetBody() );
    apl_int32_t        liRetCode = 0;
    
    ClientType* lpoClient = this->GetServer(apcKey, auKeyLen);
    ACL_ASSERT(lpoClient != NULL);

    loRequest.GetBody().Resize(20 + auKeyLen + auValueLen);

    loOutput.WriteToN((apl_int32_t)(20 + auKeyLen + auValueLen) );
    loOutput.WriteToN((apl_int32_t)CACHESERVER_REQUEST_PUT);
    loOutput.WriteToN((apl_int32_t)1);
    loOutput.WriteToN((apl_int32_t)auKeyLen);
    loOutput.Write(apcKey, auKeyLen);
    loOutput.WriteToN((apl_int32_t)auValueLen);
    loOutput.Write(apcValue, auValueLen);
    
    if ( (liRetCode = lpoClient->Request(loRequest, &lpoResponse, aoTimeout) ) != 0)
    {
        apl_errprintf("Async Client send (Put) request fail, (RetCode=%"APL_PRIdINT")\n", (apl_int_t)liRetCode );
        
        return -1;
    }

    if (lpoResponse->GetBody().GetLength() < 12)
    {
        apl_errprintf("Invalid put response body, (Length=%"APL_PRIdINT")", (apl_int_t)lpoResponse->GetBody().GetLength() );
        
        ACL_DELETE(lpoResponse);

        return -1;
    }
    else
    {
        acl::CMemoryStream loInput(&lpoResponse->GetBody() );
        
        lpoResponse->GetBody().SetReadPtr(8);
        loInput.ReadToH(liRetCode);
        
        ACL_DELETE(lpoResponse);

        return liRetCode;
    }
}

apl_int_t CCacheClient::Put(
    char const* apcKey,
    char const* apcValue,
    apl_size_t auValueLen,
    acl::CTimeValue const& aoTimeout )
{
    return this->Put(apcKey, apl_strlen(apcKey) + 1, apcValue, auValueLen, aoTimeout);
}

apl_int_t CCacheClient::Put(
    char const* apcKey,
    acl::CMemoryBlock const& aoValue,
    acl::CTimeValue const& aoTimeout )
{
    return this->Put(apcKey, apl_strlen(apcKey) + 1, aoValue.GetReadPtr(), aoValue.GetLength(), aoTimeout);
}

apl_int_t CCacheClient::Get(
    char const* apcKey,
    apl_size_t auKeyLen,
    acl::CMemoryBlock* apoValue,
    acl::CTimeValue const& aoTimeout )
{
    anf::CSimpleAsyncMessage  loRequest;
    anf::CSimpleAsyncMessage* lpoResponse = NULL;
    acl::CMemoryStream loOutput(&loRequest.GetBody() );
    apl_int32_t        liRetCode = 0;
    
    ClientType* lpoClient = this->GetServer(apcKey, auKeyLen);
    ACL_ASSERT(lpoClient != NULL);

    loRequest.GetBody().Resize(12 + auKeyLen);

    loOutput.WriteToN((apl_int32_t)(12 + auKeyLen) );
    loOutput.WriteToN((apl_int32_t)CACHESERVER_REQUEST_GET);
    loOutput.WriteToN((apl_int32_t)auKeyLen);
    loOutput.Write(apcKey, auKeyLen);
    
    if ( (liRetCode = lpoClient->Request(loRequest, &lpoResponse, aoTimeout) ) != 0)
    {
        apl_errprintf("Async Client send (Get) request fail, (RetCode=%"APL_PRIdINT")\n", (apl_int_t)liRetCode );

        return -1;
    }

    do
    {
        apl_int32_t liLength = lpoResponse->GetBody().GetLength();
        acl::CMemoryStream loInput(&lpoResponse->GetBody() );

        if (liLength < 12)
        {
            apl_errprintf("Invalid put response body, (Length=%"APL_PRIdINT")", (apl_int_t)liLength );
            
            liRetCode = -1;

            break;
        }
        else
        {
            lpoResponse->GetBody().SetReadPtr(8);
            loInput.ReadToH(liRetCode);
        }

        if (liRetCode == 0 && apoValue != NULL)
        {
            apl_int32_t liValueSize;
            
            if (loInput.ReadToH(liValueSize) != 4)
            {
                apl_errprintf("Invalid get response body, (Length=%"APL_PRIdINT", ExpectLength=%"APL_PRIdINT")", 
                    (apl_int_t)liLength, (apl_int_t)16 );
                
                liRetCode = -1;

                break;
            }

            if (liLength < 16 + liValueSize)
            {
                apl_errprintf("Invalid get response body, (Length=%"APL_PRIdINT", ExpectLength=%"APL_PRIdINT")", 
                    (apl_int_t)liLength, (apl_int_t)(16 + liValueSize) );
                
                liRetCode = -1;

                break;
            }
            
            apoValue->Reset();
            apoValue->Resize(liValueSize);
            apl_memcpy(apoValue->GetWritePtr(), lpoResponse->GetBody().GetReadPtr(), liValueSize);
            apoValue->SetWritePtr(liValueSize);
        }
    }
    while(false);

    ACL_DELETE(lpoResponse);

    return liRetCode;
}

apl_int_t CCacheClient::Get(
    char const* apcKey,
    acl::CMemoryBlock* apoValue,
    acl::CTimeValue const& aoTimeout )
{
    return this->Get(apcKey, apl_strlen(apcKey) + 1, apoValue, aoTimeout);
}

apl_int_t CCacheClient::GetTimeout(
    acl::CMemoryBlock* apoKey,
    acl::CMemoryBlock* apoValue,
    acl::CTimeValue const& aoTimeout )
{
    static apl_size_t sluIndex = 0;

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    anf::CSimpleAsyncMessage  loRequest;
    anf::CSimpleAsyncMessage* lpoResponse = NULL;
    acl::CMemoryStream loOutput(&loRequest.GetBody() );
    apl_int32_t        liRetCode = 0;
    apl_size_t         luStartIndex = sluIndex;
    
    loRequest.GetBody().Resize(12);
    loOutput.WriteToN((apl_int32_t)12);
    loOutput.WriteToN((apl_int32_t)CACHESERVER_REQUEST_GETTIMEOUT);
    loOutput.WriteToN((apl_int32_t)1);

    do
    {
        if ( (liRetCode = this->moClients[sluIndex]->Request(loRequest, &lpoResponse, aoTimeout) ) != 0)
        {
            apl_errprintf("Async Client send (GetTimeout) request fail, (RetCode=%"APL_PRIdINT")\n", (apl_int_t)liRetCode );
            
            apl_size_t luTemp(++sluIndex);
            sluIndex = luTemp % this->moClients.size();

            liRetCode = -1;

            continue;
        }

        do
        {
            apl_int32_t liLength = lpoResponse->GetBody().GetLength();
            acl::CMemoryStream loInput(&lpoResponse->GetBody() );

            if (liLength < 12)
            {
                apl_errprintf("Invalid put response body, (Length=%"APL_PRIdINT")", (apl_int_t)liLength );
                liRetCode = -1;
                break;
            }
            else
            {
                lpoResponse->GetBody().SetReadPtr(8);
                loInput.ReadToH(liRetCode);
            }

            if (liRetCode == 0)
            {
                apl_int32_t liKeySize = 0;
                apl_int32_t liValueSize = 0;

                if (loInput.ReadToH(liKeySize) != 4 
                    || liLength < 16 + liKeySize )
                {
                    apl_errprintf("Invalid get response body, (Length=%"APL_PRIdINT", ExpectLength=%"APL_PRIdINT")", 
                        (apl_int_t)liLength, (apl_int_t)(16 + liKeySize) );
                    liRetCode = -1;
                    break;
                }
                
                if (apoKey != NULL)
                {
                    apoKey->Reset();
                    apoKey->Resize(liKeySize);
                    apl_memcpy(apoKey->GetWritePtr(), lpoResponse->GetBody().GetReadPtr(), liKeySize);
                    apoKey->SetWritePtr(liKeySize);
                    lpoResponse->GetBody().SetReadPtr(liKeySize);
                }
                
                if (loInput.ReadToH(liValueSize) != 4 
                    || liLength < 16 + liKeySize + 4 + liValueSize )
                {
                    apl_errprintf("Invalid get response body, (Length=%"APL_PRIdINT", ExpectLength=%"APL_PRIdINT")", 
                        (apl_int_t)liLength, (apl_int_t)(20 + liKeySize + liValueSize) );
                    liRetCode = -1;
                    break;
                }

                if (apoValue != NULL)
                {
                    apoValue->Reset();
                    apoValue->Resize(liValueSize);
                    apl_memcpy(apoValue->GetWritePtr(), lpoResponse->GetBody().GetReadPtr(), liValueSize);
                    apoValue->SetWritePtr(liValueSize);
                    lpoResponse->GetBody().SetReadPtr(liKeySize);
                }
            }
            else
            {
                apl_size_t luTemp(++sluIndex);
                sluIndex = luTemp % this->moClients.size();
            }
        }
        while(false);
        
        ACL_DELETE(lpoResponse);

        //Success and break
        if (liRetCode == 0)
        {
            break;
        }
    }
    while(luStartIndex != sluIndex);

    return liRetCode;
}

bool CCacheClient::CheckHashRangeValue( apl_size_t auLower, apl_size_t auUpper )
{
    for (apl_size_t auN = auLower; auN <= auUpper; auN++)
    {
        if (this->moClientMap[auN] != NULL)
        {
            return false;
        }
    }

    return true;
}

bool CCacheClient::CheckAllHashRangeValue( apl_int_t* apiUndefineValue )
{
    for (apl_int_t aiN = 0; aiN < 100; aiN++)
    {
        if (this->moClientMap[aiN] == NULL)
        {
            *apiUndefineValue = aiN;
            return false;
        }
    }

    return true;
}

void CCacheClient::SetServer( apl_size_t auLower, apl_size_t auUpper, ClientType* apoServer )
{
    for (apl_size_t auN = auLower; auN <= auUpper; auN++)
    {
        this->moClientMap[auN] = apoServer;
    }

    this->moClients.push_back(apoServer);
}

CCacheClient::ClientType* CCacheClient::GetServer( char const* apcKey, apl_size_t auLen )
{
    apl_int_t liKey = HashString(apcKey, auLen) % 100;

    return this->moClientMap[liKey];
}

AIBC_CACHESERVER_NAMESPACE_END

