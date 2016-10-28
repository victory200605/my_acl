#include "acl/PeriodCounter.h"

ACL_NAMESPACE_START

CPeriodCounter::CPeriodCounter()
{
    this->ciMaxPeriod = 0;
    this->ciTimePeriod = 0;
}

CPeriodCounter::CPeriodCounter(const apl_int_t aiMaxPeriod, const apl_int_t aiTimePeriod)
{
    this->ciMaxPeriod = aiMaxPeriod;
    this->ciTimePeriod = aiTimePeriod;
}

CPeriodCounter::~CPeriodCounter()
{
    CNode* lpoNode = NULL;
    for( std::list<CNode*>::iterator loIter = this->coNodeLink.begin(); loIter != this->coNodeLink.end();)
    {
        lpoNode = *loIter;
        this->coNodeLink.erase(loIter++);
        
        ACL_DELETE(lpoNode);
    }
}

//public
apl_int_t CPeriodCounter::Init(const apl_int_t aiMaxPeriod, const apl_int_t aiTimePeriod)
{
    if( this->ciMaxPeriod != aiMaxPeriod )
    {
        this->ciMaxPeriod = aiMaxPeriod;
    }
    
    if( this->ciTimePeriod != aiTimePeriod )
    {
        this->ciTimePeriod = aiTimePeriod;
    }

    return 0;
}

apl_int_t CPeriodCounter::GetPeriodCount(const char* apcName)
{
    //Lock
    acl::TSmartLock<acl::CLock> loGuard(this->coLock);

    if( this->coNodeLink.size() > 0 )
    {  
        for( std::list<CNode*>::iterator loIter = this->coNodeLink.begin(); loIter != this->coNodeLink.end(); ++loIter )
        {
            if( apl_strcmp(apcName, (*loIter)->csName) == 0 )
            {
                return (*loIter)->coPeriodLink.size();
            }
        }
    }

    return 0;
}

apl_int_t CPeriodCounter::RemoveLastPeriod(const char* apcName)
{
    //Lock
    acl::TSmartLock<acl::CLock> loGuard(this->coLock);
    
    if( this->coNodeLink.size() > 0 )
    {  
        for( std::list<CNode*>::iterator loIter = this->coNodeLink.begin(); loIter != this->coNodeLink.end(); ++loIter )
        {
            if( apl_strcmp(apcName, (*loIter)->csName) == 0 )
            {
                if( (*loIter)->coPeriodLink.size() > 0 )
                {
                    (*loIter)->coPeriodLink.pop_back();
                }
                break;
            }
        }
    }
    
    return 0;
}

apl_int_t CPeriodCounter::RemoveAllPeriod(const char* apcName)
{
    //Lock
    acl::TSmartLock<acl::CLock> loGuard(this->coLock);
    
    if( this->coNodeLink.size() > 0 )
    {  
        for( std::list<CNode*>::iterator loIter = this->coNodeLink.begin(); loIter != this->coNodeLink.end(); ++loIter )
        {
            if( apl_strcmp(apcName, (*loIter)->csName) == 0 )
            {
                (*loIter)->coPeriodLink.clear();
                break;
            }
        }
    }
    
    return 0;
}

apl_int_t CPeriodCounter::AddData(const char* apcName, const apl_int_t aiData)
{   
    if( aiData < 0 || apl_strlen(apcName) > PERIOD_MAX_LENGTH_NAME )
    {
        return -1;
    }
    
    //Lock
    acl::TSmartLock<acl::CLock> loGuard(this->coLock);
    
    for( std::list<CNode*>::iterator loIter = this->coNodeLink.begin(); loIter != this->coNodeLink.end(); ++loIter )
    {
        if( apl_strcmp(apcName, (*loIter)->csName) == 0 )
        {
            return this->AddData((*loIter), aiData);
        }
    }

    //new
    CNode* lpoNode = NULL;
    
    ACL_NEW_ASSERT(lpoNode, CNode);

    apl_memset(lpoNode->csName, 0, sizeof(lpoNode->csName));
    if( this->ciTimePeriod > 0 )
    {
        lpoNode->ciLastPeriodTime = time(NULL);
    }
    else
    {
        lpoNode->ciLastPeriodTime = 0;
    }
    lpoNode->coPeriodLink.clear();
    
    apl_strncpy(lpoNode->csName, apcName, sizeof(lpoNode->csName));
    
    this->AddData(lpoNode, aiData);
    
    this->coNodeLink.push_back(lpoNode);

    return 0;
}

apl_size_t CPeriodCounter::GetCount(const char* apcName, const apl_int_t aiPeriodNo)
{
    CStatData   loStatData;
    apl_size_t  liCounter = 0;
    
    //Lock
    acl::TSmartLock<acl::CLock> loGuard(this->coLock);
    
    apl_memset(&loStatData, 0, sizeof(CStatData));
    
    for( std::list<CNode*>::iterator loNodeIter = this->coNodeLink.begin(); loNodeIter != this->coNodeLink.end(); ++loNodeIter )
    {
        if( apl_strcmp(apcName, (*loNodeIter)->csName) == 0 )
        {
            // move period first
            this->TryMovePeriod((*loNodeIter));
            
            if( aiPeriodNo >= 0 )
            {               
                if( this->LocatePeriod((*loNodeIter), aiPeriodNo, &loStatData) != 0 )
                {
                    return 0;
                }
                return loStatData.ciCounter;
            }
            else
            {
                for( std::list<CStatData>::iterator loIter = (*loNodeIter)->coPeriodLink.begin(); loIter != (*loNodeIter)->coPeriodLink.end(); ++loIter )
                {   
                    if( loIter->ciCounter > 0 )
                    {
                        liCounter += loIter->ciCounter;
                    }
                }
                return liCounter;
            }
        }
    }

    return 0;
}

apl_size_t CPeriodCounter::GetSum(const char* apcName, const apl_int_t aiPeriodNo)
{
    CStatData   loStatData;
    apl_size_t  liTotal = 0;
    
    //Lock
    acl::TSmartLock<acl::CLock> loGuard(this->coLock);
    
    apl_memset(&loStatData, 0, sizeof(CStatData));
    
    for( std::list<CNode*>::iterator loNodeIter = this->coNodeLink.begin(); loNodeIter != this->coNodeLink.end(); ++loNodeIter )
    {
        if( apl_strcmp(apcName, (*loNodeIter)->csName) == 0 )
        {
            // move period first
            this->TryMovePeriod((*loNodeIter));
            
            if( aiPeriodNo >= 0 )
            {
                if( this->LocatePeriod((*loNodeIter), aiPeriodNo, &loStatData) != 0 )
                {
                    return 0;
                }
                return(loStatData.ciTotal);
            }
            else
            {
                for( std::list<CStatData>::iterator loIter = (*loNodeIter)->coPeriodLink.begin(); loIter != (*loNodeIter)->coPeriodLink.end(); ++loIter )
                {   
                    if( loIter->ciCounter > 0 )
                    {
                        liTotal += loIter->ciTotal;
                    }
                }
                return liTotal;
            }
        }
    }

    return 0;
}

apl_int_t CPeriodCounter::GetAverage(const char* apcName, const apl_int_t aiPeriodNo)
{
    CStatData   loStatData;
    apl_size_t  liTotal     = 0;
    apl_size_t  liCounter   = 0;
    apl_int_t   liAverage   = 0;
    
    //Lock
    acl::TSmartLock<acl::CLock> loGuard(this->coLock);
    
    apl_memset(&loStatData, 0, sizeof(CStatData));
    
    for( std::list<CNode*>::iterator loNodeIter = this->coNodeLink.begin(); loNodeIter != this->coNodeLink.end(); ++loNodeIter )
    {
        if( apl_strcmp(apcName, (*loNodeIter)->csName) == 0 )
        {
            // move period first
            this->TryMovePeriod((*loNodeIter));
            
            if( aiPeriodNo >= 0 )
            {
                if( this->LocatePeriod((*loNodeIter), aiPeriodNo, &loStatData) != 0 )
                {
                    return 0;
                }
                if( loStatData.ciCounter > 0 )
                {
                    liAverage = loStatData.ciTotal / loStatData.ciCounter;
                }
                return liAverage;
            }
            else
            {
                for( std::list<CStatData>::iterator loIter = (*loNodeIter)->coPeriodLink.begin(); loIter != (*loNodeIter)->coPeriodLink.end(); ++loIter )
                {   
                    if( loIter->ciCounter > 0 )
                    {
                        liTotal += loIter->ciTotal;
                        
                        liCounter += loIter->ciCounter;
                    }
                }
                if( liCounter > 0 )
                {
                    liAverage = liTotal / liCounter;
                }
                return liAverage;
            }
        }
    }

    return 0;
}

apl_int_t CPeriodCounter::GetMin(const char* apcName, const apl_int_t aiPeriodNo)
{
    CStatData   loStatData;
    apl_int_t   liMin = -1;

    //Lock
    acl::TSmartLock<acl::CLock> loGuard(this->coLock);
    
    apl_memset(&loStatData, 0, sizeof(CStatData));

    for( std::list<CNode*>::iterator loNodeIter = this->coNodeLink.begin(); loNodeIter != this->coNodeLink.end(); ++loNodeIter )
    {
        if( apl_strcmp(apcName, (*loNodeIter)->csName) == 0 )
        {
            // move period first
            this->TryMovePeriod((*loNodeIter));
            
            if( aiPeriodNo >= 0 )
            {
                if( this->LocatePeriod((*loNodeIter), aiPeriodNo, &loStatData) != 0 )
                {
                    return 0;
                }
                
                return loStatData.ciMinValue;
            }
            
            for( std::list<CStatData>::iterator loIter = (*loNodeIter)->coPeriodLink.begin(); loIter != (*loNodeIter)->coPeriodLink.end(); ++loIter )
            {   
                if( loIter->ciCounter > 0 )
                {
                    if( liMin == -1 )
                    {
                        liMin = loIter->ciMinValue;
                    }
                    else
                    {
                        liMin = PERIOD_COUNTER_MIN(liMin, loIter->ciMinValue);
                    }
                }
            }
            
            return liMin;
        }
    }

    return 0;
}

apl_int_t CPeriodCounter::GetMax(const char* apcName, const apl_int_t aiPeriodNo)
{
    CStatData   loStatData;
    apl_int_t   liMax = -1;

    //Lock
    acl::TSmartLock<acl::CLock> loGuard(this->coLock);
    
    apl_memset(&loStatData, 0, sizeof(CStatData));

    for( std::list<CNode*>::iterator loNodeIter = this->coNodeLink.begin(); loNodeIter != this->coNodeLink.end(); ++loNodeIter )
    {
        if( apl_strcmp(apcName, (*loNodeIter)->csName) == 0 )
        {
            // move period first
            this->TryMovePeriod((*loNodeIter));
            
            if( aiPeriodNo >= 0 )
            {
                if( this->LocatePeriod((*loNodeIter), aiPeriodNo, &loStatData) != 0 )
                {
                    return 0;
                }
                
                return loStatData.ciMaxValue;
            }
            
            for( std::list<CStatData>::iterator loIter = (*loNodeIter)->coPeriodLink.begin(); loIter != (*loNodeIter)->coPeriodLink.end(); ++loIter )
            {   
                if( loIter->ciCounter > 0 )
                {
                    if( liMax == -1 )
                    {
                        liMax = loIter->ciMaxValue;
                    }
                    else
                    {
                        liMax = PERIOD_COUNTER_MAX(liMax, loIter->ciMaxValue);
                    }
                }
            }
            
            return liMax;
        }
    }

    return 0;
}

apl_int_t CPeriodCounter::ResetPeriod(const char* apcName, const apl_int_t aiPeriodNo)
{
    apl_int_t liCount = 0;
    CStatData loStatData;
    
    //Lock
    acl::TSmartLock<acl::CLock> loGuard(this->coLock);
    
    apl_memset(&loStatData, 0, sizeof(CStatData));
    
    for( std::list<CNode*>::iterator loNodeIter = this->coNodeLink.begin(); loNodeIter != this->coNodeLink.end(); ++loNodeIter )
    {
        if( apl_strcmp(apcName, (*loNodeIter)->csName) == 0 )
        {
            if( (*loNodeIter)->coPeriodLink.size() <= 0 )
            {
                return 0;
            }
            
            std::list<CStatData>::iterator loIter;
            
            if( aiPeriodNo >= 0 )
            {
                if( aiPeriodNo >= (apl_int_t)(*loNodeIter)->coPeriodLink.size() )
                {
                    return 0;
                }
                
                liCount = aiPeriodNo;
                for( loIter = (*loNodeIter)->coPeriodLink.begin(); loIter != (*loNodeIter)->coPeriodLink.end(); ++loIter )
                {
                    if( liCount <= 0 )
                    {
                        break;
                    }
                    liCount--;
                }
                
                loIter->ciTotal = 0;
                loIter->ciCounter = 0;
                loIter->ciMaxValue = 0;
                loIter->ciMinValue = 0;

                return 0;
            }
            
            for( loIter = (*loNodeIter)->coPeriodLink.begin(); loIter != (*loNodeIter)->coPeriodLink.end(); ++loIter )
            {
                loIter->ciTotal = 0;
                loIter->ciCounter = 0;
                loIter->ciMaxValue = 0;
                loIter->ciMinValue = 0;
            }
            
            return 0;
        }
    }

    return 0;
}

//private
apl_int_t CPeriodCounter::AddData(CNode* apcNode, const apl_int_t aiData)
{
    CStatData   loStatData;
    apl_int_t   liDiffTime = 0;
    apl_int_t   liPeriod = 0;
    apl_int_t   liCount = 0;
    apl_int_t   liN = 0;

    if( aiData < 0 )
    {
        return -1;
    }

    // calculate current time period
    if( this->ciTimePeriod > 0)
    {
        liDiffTime = (time(NULL) - apcNode->ciLastPeriodTime);
        liPeriod = liDiffTime/this->ciTimePeriod;

        // add new period
        if( liPeriod > 0 )
        {
            apcNode->ciLastPeriodTime = time(NULL);
            
            // delete the overflow period first
            if( this->ciMaxPeriod > 0 )
            {
                liCount = apcNode->coPeriodLink.size();
                for( liN=liCount+liPeriod; liN>this->ciMaxPeriod&&liN>liPeriod; --liN)
                {
                    apcNode->coPeriodLink.pop_back();
                }
                
                liPeriod = PERIOD_COUNTER_MIN(liPeriod, this->ciMaxPeriod);
            }

            // insert new periods
            for( liN=1; liN<=liPeriod; ++liN )
            {
                apl_memset(&loStatData, 0, sizeof(CStatData));

                if( liN == liPeriod )
                {
                    loStatData.ciTotal = (apl_size_t)aiData;
                    loStatData.ciCounter = 1;
                    loStatData.ciMaxValue = aiData;
                    loStatData.ciMinValue = aiData;
                }
                
                apcNode->coPeriodLink.push_front(loStatData);
            }

            return 0;
        }
    }
    
    // add the data into current period
    if( apcNode->coPeriodLink.size() <= 0 )
    {
        apl_memset(&loStatData, 0, sizeof(CStatData));
        
        loStatData.ciTotal = (apl_size_t)aiData;
        loStatData.ciCounter = 1;
        loStatData.ciMaxValue = aiData;
        loStatData.ciMinValue = aiData;

        apcNode->coPeriodLink.push_front(loStatData);
    }
    else
    {
        // replace
        std::list<CStatData>::iterator loIter = apcNode->coPeriodLink.begin();

        loIter->ciTotal += aiData;
        loIter->ciCounter += 1;
        loIter->ciMinValue = PERIOD_COUNTER_MIN(loIter->ciMinValue, aiData);
        loIter->ciMaxValue = PERIOD_COUNTER_MAX(loIter->ciMaxValue, aiData);
    }
        
    return 0;
}

apl_int_t CPeriodCounter::TryMovePeriod(CNode* apcNode)
{
    CStatData loStatData;
    apl_int_t liDiffTime = 0;
    apl_int_t liPeriod = 0;
    apl_int_t liCount = 0;
    apl_int_t liN = 0;

    // calculate current time period
    if( this->ciTimePeriod > 0 )
    {
        liDiffTime = (time(NULL) - apcNode->ciLastPeriodTime);
        liPeriod = liDiffTime/this->ciTimePeriod;

        // add new period
        if( liPeriod > 0 )
        {
            apcNode->ciLastPeriodTime = time(NULL);
            
            // delete the overflow period first
            if( this->ciMaxPeriod > 0 )
            {
                liCount = apcNode->coPeriodLink.size();
                for( liN=liCount+liPeriod; liN>this->ciMaxPeriod&&liN>liPeriod; --liN)
                {
                    apcNode->coPeriodLink.pop_back();
                }
            
                liPeriod = PERIOD_COUNTER_MIN(liPeriod, this->ciMaxPeriod);
            }

            // insert new periods
            for( liN=1; liN<=liPeriod; ++liN )
            {
                apl_memset(&loStatData, 0, sizeof(CStatData));
                apcNode->coPeriodLink.push_front(loStatData);
            }
        }
    }

    return 0;
}

apl_int_t CPeriodCounter::LocatePeriod(CNode* apcNode, const apl_int_t aiPeriodNo, CStatData *apcStatData)
{
    apl_int_t liCount = 0;
    std::list<CStatData>::iterator loIter;

    if( aiPeriodNo<0 || apcNode->coPeriodLink.size()<=0 || aiPeriodNo >= (apl_int_t)apcNode->coPeriodLink.size() )
    {
        return -1;
    }

    liCount = aiPeriodNo;
    for( loIter = apcNode->coPeriodLink.begin(); loIter != apcNode->coPeriodLink.end(); ++loIter )
    {
        if( liCount <= 0 )
        {
            break;
        }
        liCount--;
    }

    apcStatData->ciTotal = loIter->ciTotal;
    apcStatData->ciCounter = loIter->ciCounter;
    apcStatData->ciMaxValue = loIter->ciMaxValue;
    apcStatData->ciMinValue = loIter->ciMinValue;

    return(0);
}

ACL_NAMESPACE_END

