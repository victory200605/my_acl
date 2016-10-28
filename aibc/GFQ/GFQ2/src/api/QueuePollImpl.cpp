
#include "QueuePollImpl.h"
#include "GFQueueClientImpl.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Poll result cache to reduce repeat request times
 */
CPollResultCache::CPollResultCache(void)
    : mpoResult(APL_NULL)
    , moTimedout(1)
    , moTimestamp(0)
{
}

CPollResultCache::~CPollResultCache(void)
{
    ACL_DELETE(this->mpoResult);
}

void CPollResultCache::SetTimedout( acl::CTimeValue const& aoTimedout )
{
    this->moTimedout = aoTimedout;
}

acl::CIndexDict* CPollResultCache::GetResult(void)
{
    if (this->mpoResult == APL_NULL)
    {
        return APL_NULL;
    }
    else
    {
        acl::CTimestamp loCurrTime;
            
        loCurrTime.Update(acl::CTimestamp::PRC_MSEC);
        
        if (loCurrTime.Msec() - this->moTimestamp.Msec() > this->moTimedout.Msec() )
        {
            return APL_NULL;
        }
        else
        {
            return this->mpoResult;
        }
    }
}

void CPollResultCache::Update( acl::CIndexDict* apoResult )
{
    ACL_DELETE(this->mpoResult);
    
    this->mpoResult = apoResult;
    
    this->moTimestamp.Update();
}

/** 
 * @brief Queue pollset impl
 */
CQueuePollImpl::CQueuePollImpl( CGFQueueClientImpl* apoClient )
    : mpoClient(apoClient)
    , mi64Timestamp(0)
{
}

CQueuePollImpl::~CQueuePollImpl(void)
{
}

void CQueuePollImpl::SetResultCacheTime( acl::CTimeValue const& aoTimedout )
{
    this->moResultCache.SetTimedout(aoTimedout);
}

void CQueuePollImpl::Add( char const* apcQueueName )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moQueueLock);
    
    this->moAddQueue.push_back(apcQueueName);
}

void CQueuePollImpl::Remove( char const* apcQueueName )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moQueueLock);
    
    this->moRemoveQueue.push_back(apcQueueName);
}

void CQueuePollImpl::GetAllQueue( std::vector<std::string>& aoResult )
{
    for (std::map<std::string, apl_size_t>::iterator loIter = this->moNameMap.begin();
         loIter != this->moNameMap.end(); ++loIter)
    {
        aoResult.push_back(loIter->first);
    }
}

apl_int_t CQueuePollImpl::UpdateQueueIDSet(void)
{
    apl_int_t liRetCode = 0;

    if (this->mi64Timestamp == 0)
    {
        acl::CTimestamp loTimestamp;

        if ( (liRetCode = this->mpoClient->GetStartupTimestamp(loTimestamp) ) != 0)
        {
            return liRetCode;
        }
        
        this->mi64Timestamp = loTimestamp.Nsec();
    }

    if (this->moAddQueue.size() > 0)
    {//check add-queue
        CQueueInfo loInfo;
        std::vector<std::string> loTemp;

        {
            acl::TSmartLock<acl::CLock> loGuard(this->moQueueLock);
            
            loTemp = this->moAddQueue;

            this->moAddQueue.clear();
        }

        //Add interested queue
        for (std::vector<std::string>::iterator loIter = loTemp.begin();
             loIter != loTemp.end(); ++loIter )
        {
            //Get queue id from server
            if ( (liRetCode = this->mpoClient->CreateQueue(loIter->c_str(), IGNORE_QUEUE_EXISTED, &loInfo) ) != 0)
            {
                break;
            }

            //Update map info and queue-id-set
            this->moIDMap[loInfo.GetQueueID()] = *loIter;
            this->moNameMap[*loIter] = loInfo.GetQueueID();

            if (loInfo.GetQueueID() >= this->moQueueIDSet.GetSize() )
            {
                this->moQueueIDSet.Resize(loInfo.GetQueueID() + 1);
            }

            this->moQueueIDSet.Set(loInfo.GetQueueID(), true);
        }
        
        //Fail and save back unprocessing queue
        if (liRetCode != 0)
        {
            acl::TSmartLock<acl::CLock> loGuard(this->moQueueLock);
            
            this->moAddQueue.insert(this->moAddQueue.end(), loTemp.begin(), loTemp.end() );

            return liRetCode;
        }
    }

    if (this->moRemoveQueue.size() > 0)
    {
        std::vector<std::string> loTemp;

        acl::TSmartLock<acl::CLock> loGuard(this->moQueueLock);
        
        //Remove uninterested queue
        for (std::vector<std::string>::iterator loIter = loTemp.begin();
             loIter != loTemp.end(); ++loIter )
        {
            std::map<std::string, apl_size_t>::iterator loNameIter = this->moNameMap.find(*loIter);
            if (loNameIter != this->moNameMap.end() )
            {
                this->moQueueIDSet.Set(loNameIter->second, false);
                this->moIDMap.erase(loNameIter->second);
                this->moNameMap.erase(loNameIter);
            }
        }
        
        this->moRemoveQueue.clear();
    }

    return 0;
}

apl_int_t CQueuePollImpl::Poll( acl::CTimeValue const& aoTimedout, std::vector<std::string>* apoResult )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict* lpoResponse = APL_NULL;

    //Clear result frist
    if (apoResult != APL_NULL)
    {
        apoResult->clear();
    }
    
    acl::TSmartLock<acl::CLock> loGuard(this->moPollLock);

    if ( (lpoResponse = this->moResultCache.GetResult() ) == APL_NULL)
    {
        while(true)
        {
            if ( (liRetCode = this->UpdateQueueIDSet() ) != 0)
            {
                return liRetCode;
            }

            if (this->moQueueIDSet.GetSize() <= 0)
            {
                return 0;
            }

            acl::CIndexDict  loRequest;
            
            loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_POLL_QUEUE);
            loRequest.Set(GFQ_TAG_TIMEDOUT, aoTimedout.Sec() );
            loRequest.Set(GFQ_TAG_TIMESTAMP, this->mi64Timestamp);
            loRequest.Set(GFQ_TAG_QUEUE_BITVEC, this->moQueueIDSet.GetCStr(), this->moQueueIDSet.GetCLength() );

            liRetCode = this->mpoClient->CommonRequest(loRequest, &lpoResponse, aoTimedout);
            switch(liRetCode)
            {
                case 0:
                    break;
                case ERROR_TIMEDOUT:
                {
                    //if timedout will retry all queue
                    if (apoResult != APL_NULL)
                    {
                        this->GetAllQueue(*apoResult);
                    }

                    return this->moNameMap.size();
                }
                case ERROR_INVALID_TIMESTAMP:
                {
                    //update master server startup timestamp, 
                    //if startup timestamp changed, clear all cache data and update poll set again 
                    this->Reset();

                    continue;
                }
                default:
                {
                    return liRetCode;
                }
            }

            break;
        }
        
        this->moResultCache.Update(lpoResponse);
    }

    apl_int_t luSize = 0;
    acl::CIndexDict::ValueType loValue = lpoResponse->GetStr(GFQ_TAG_QUEUE_BITVEC, "");
    acl::CBitArray loBitArray(loValue.GetStr(), loValue.GetLength(), acl::CBitArray::DONT_COPY);

    for (apl_size_t luN = loBitArray.Find(true); 
         luN != acl::CBitArray::INVALID_POS; luN = loBitArray.Find(luN + 1, true) )
    {
        std::map<apl_size_t, std::string>::iterator loIter = this->moIDMap.find(luN);

        ACL_ASSERT(loIter != this->moIDMap.end() );
    
        if (apoResult != APL_NULL)
        {
            apoResult->push_back(loIter->second);
        }

        luSize++;
    }

    return luSize;
}

void CQueuePollImpl::Reset(void)
{
    for (std::map<std::string, apl_size_t>::iterator loIter = this->moNameMap.begin();
         loIter != this->moNameMap.end(); ++loIter)
    {
        this->Add(loIter->first.c_str() );
    }

    this->moNameMap.clear();
    this->moIDMap.clear();
    this->moQueueIDSet.SetAll(false);
    this->mi64Timestamp = 0;
}

AIBC_GFQ_NAMESPACE_END

