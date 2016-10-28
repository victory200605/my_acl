
#include "Table.h"

////////////////////////////////////////////////////////////////////////////////
CCounter::CCounter(void)
    : muBucketSize(0)
    , muTokens(0)
{
}

CCounter::CCounter(apl_size_t auBucketSize)
    : muBucketSize(auBucketSize)
    , muTokens(0)
{
}

void CCounter::SetBucketSize(apl_size_t auBucketSize)
{
    this->muBucketSize = auBucketSize;
}

apl_size_t CCounter::GetBucketSize(void)
{
    return this->muBucketSize;
}

apl_size_t CCounter::GetCurTokens(void)
{
    return this->muTokens;
}

apl_int_t CCounter::Acquire(apl_size_t auTokens, EOption aeOpt)
{
    if((this->muTokens + auTokens > this->muBucketSize) && aeOpt == OPT_UNFORCE)
    {
        return -1;
    }

    this->muTokens += auTokens;

    return 0;
}

apl_int_t CCounter::Release(apl_size_t auTokens)
{
    if(this->muTokens < auTokens)
    {
        this->muTokens = 0;
    }
    else
    {
        this->muTokens -= auTokens;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
CSpeeder::CSpeeder(void)
    : mdTokens(0)
    , muSpeed(0)
    , muBucketSize(0)
{
    this->moLastTime.Update(acl::CTimestamp::PRC_USEC);
}

CSpeeder::CSpeeder(apl_size_t auSpeed, apl_size_t auBucketSize)
    : mdTokens(0)
    , muSpeed(auSpeed)
    , muBucketSize(auBucketSize)
{
    this->moLastTime.Update(acl::CTimestamp::PRC_USEC);
}

void CSpeeder::SetBucketSize(apl_size_t auBucketSize)
{
    this->muBucketSize = auBucketSize;
}

apl_size_t CSpeeder::GetBucketSize()
{
    return this->muBucketSize;
}

apl_size_t CSpeeder::GetSpeed()
{
    return this->muSpeed;
}

double CSpeeder::GetCurTokens()
{
    return this->mdTokens;
}

void CSpeeder::SetSpeed(apl_size_t auSpeed)
{
    this->muSpeed = auSpeed;
}

apl_int_t CSpeeder::Acquire(apl_size_t auTokens, acl::CTimestamp& aoTimestamp)
{
    double ldNewTokens = 0;
    double ldTokens = 0;
    double ldSec = 0;


	if(this->muBucketSize == 0)
	{
		return 0;
	}
    
    if(aoTimestamp > this->moLastTime)
    {
        ldSec = (double)(aoTimestamp.Nsec() - this->moLastTime.Nsec()) / APL_TIME_SEC;
        ldNewTokens = ldSec * this->muSpeed;
    }

    ldTokens = std::min(this->mdTokens + ldNewTokens, (double)this->muBucketSize);
    
    if ( (apl_size_t)ldTokens >= auTokens)
    {   
        this->mdTokens = ldTokens - auTokens;
        
        if (ldNewTokens > 0)
        {
            this->moLastTime = aoTimestamp;
        }
        
        return 0;
    }
    else
    {
        return -1;
    }
}

acl::CTimestamp& CSpeeder::GetLastTime(void)
{
    return this->moLastTime;
}

//////////////////////////////////////////////////////////////////////////////////////////
CTableImpl::CNode::CNode(void)
    : mpoCounter(NULL)
    , mpoSpeeder(NULL)
{
}

CTableImpl::CTableImpl(void)
{
}

CTableImpl::~CTableImpl(void)
{
}

apl_int_t CTableImpl::FindItem(char const* apcKey)
{
    std::map<std::string, apl_int_t>::iterator loIter = this->moMap.find(apcKey);
    if(loIter != this->moMap.end())
    {
        return loIter->second;
    } 
    else
    {
        return -1;
    }
}

CCounter* CTableImpl::GetCounter(char const* apcKey, apl_int_t* apiN)
{
    apl_int_t liIndex = this->FindItem(apcKey);

    if(liIndex == -1)
    {
        return NULL;
    }
    
    if (apiN != NULL)
    {
        *apiN = liIndex;
    }
    
    return this->moList[liIndex].mpoCounter;
}

CCounter* CTableImpl::GetCounter(apl_int_t aiN)
{
    if(aiN < 0 || (apl_size_t)aiN >= this->moList.size() )
    {
        return NULL;
    }

    return this->moList[aiN].mpoCounter; 
}

CSpeeder* CTableImpl::GetSpeeder(char const* apcKey, apl_int_t* apiN)
{
    apl_int_t liIndex = this->FindItem(apcKey);

    if(liIndex == -1)
    {
        return NULL;
    }
    
    if (apiN != NULL)
    {
        *apiN = liIndex;
    }

    return this->moList[liIndex].mpoSpeeder;
}

CSpeeder* CTableImpl::GetSpeeder(apl_int_t aiN)
{
    if(aiN < 0 || (apl_size_t)aiN >= this->moList.size() )
    {
        return NULL;
    }
    
    return this->moList[aiN].mpoSpeeder;
}

apl_int_t CTableImpl::UpdateCounter(char const* apcKey, apl_size_t auMaxConNum)
{
    apl_int_t liIndex = this->FindItem(apcKey);

    if(liIndex >= 0)
    {
        if(this->moList[liIndex].mpoCounter != NULL)
        {
            this->moList[liIndex].mpoCounter->SetBucketSize(auMaxConNum);
        }
        else
        {
            //create counter to add to the struct    
			ACL_NEW_INIT_ASSERT(this->moList[liIndex].mpoCounter, CCounter, auMaxConNum);
        }
    }
    else
    {
        CNode loNew;
        ACL_NEW_INIT_ASSERT(loNew.mpoCounter, CCounter, auMaxConNum);
        
        this->moList.push_back(loNew);
        this->moMap.insert( std::map<std::string, apl_int_t>::value_type(apcKey, this->moList.size() - 1) );     
    } 
    
    return 0; 
}

apl_int_t CTableImpl::UpdateSpeeder(char const* apcKey, apl_size_t auTokens, apl_size_t auMaxBucketSize)
{
    apl_int_t liIndex = this->FindItem(apcKey);

    if(liIndex >= 0)
    {
        if(this->moList[liIndex].mpoSpeeder != NULL)
        {
            this->moList[liIndex].mpoSpeeder->SetBucketSize(auMaxBucketSize);
            this->moList[liIndex].mpoSpeeder->SetSpeed(auTokens);
        }
        else
        {
            //create speeder to add to the struct    
			ACL_NEW_INIT_ASSERT(this->moList[liIndex].mpoSpeeder, CSpeeder, auTokens, auMaxBucketSize);
        }
    }    
    else
    {
        CNode loNew;
        ACL_NEW_INIT_ASSERT(loNew.mpoSpeeder, CSpeeder, auTokens, auMaxBucketSize);
        
        this->moList.push_back(loNew);
        this->moMap.insert(std::map<std::string, apl_int_t>::value_type(apcKey, this->moList.size() - 1) );     
    } 
    
    return 0; 
}

void CTableImpl::Clear()
{
    for (std::vector<CNode>::iterator loIter = this->moList.begin(); loIter != this->moList.end(); ++loIter)
    {
        ACL_DELETE(loIter->mpoCounter);
        ACL_DELETE(loIter->mpoSpeeder);
    }
    
    this->moList.clear();
    this->moMap.clear();
}

void CTableImpl::Copy(CTableImpl& aoTable)
{
	this->moList = aoTable.moList;
	this->moMap = aoTable.moMap;
}

///////////////////////////////////////////////////////////////////////////////////////
CTable* CTable::cpoInstance = NULL;
    
CTable* CTable::Instance()
{
    if(cpoInstance == NULL)
    {
        ACL_NEW_ASSERT(cpoInstance, CTable);
    }
    
    return cpoInstance;
}

void CTable::Release()
{
    if(cpoInstance != NULL)
    {
        ACL_DELETE(cpoInstance);
    }
}

CTable::CTable()
{
    ACL_NEW_ASSERT(this->mpoCurList, CTableImpl);
    ACL_NEW_ASSERT(this->mpoUpdateList, CTableImpl); 
}

CTable::~CTable()
{
    //Clear current list only, because mpoCurList and mpoUpdateList point to same
    //exp:
    //   mpoCurList--->Counter
    //                    ^
    //   mpoUpdateList----|
    this->mpoCurList->Clear();
    
    ACL_DELETE(this->mpoCurList);
    ACL_DELETE(this->mpoUpdateList);
}

CCounter* CTable::GetCounter(char const* apcKey, apl_int_t* aiN)
{
    return this->mpoCurList->GetCounter(apcKey, aiN);
}

CCounter* CTable::GetCounter(apl_int_t aiN)
{
    return this->mpoCurList->GetCounter(aiN);
}

CSpeeder* CTable::GetSpeeder(char const* apcKey, apl_int_t* aiN)
{
    return this->mpoCurList->GetSpeeder(apcKey, aiN);
}

CSpeeder* CTable::GetSpeeder(apl_int_t aiN)
{
    return this->mpoCurList->GetSpeeder(aiN);
}

apl_int_t CTable::UpdateCounter(char const* apcKey, apl_size_t auMaxConNum)
{
    return this->mpoUpdateList->UpdateCounter(apcKey, auMaxConNum); 
}

apl_int_t CTable::UpdateSpeeder(char const* apcKey, apl_size_t auTokens, apl_size_t auMaxBucketSize)
{
    return this->mpoUpdateList->UpdateSpeeder(apcKey, auTokens, auMaxBucketSize);
}

void CTable::StartUpdate(void)
{
    this->mpoUpdateList->Copy(*this->mpoCurList);
}

void CTable::Commit(void)
{
    std::swap(this->mpoCurList, this->mpoUpdateList);
}




