
#ifndef COUNTSERVER_TABLE_H
#define COUNTSERVER_TABLE_H

#include "apl/apl.h"
#include "acl/Timestamp.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"
#include "acl/stl/map.h"

////////////////////////////////////////////////////////////////////////////////////
class CCounter
{
public:
    enum EOption
    {
        //If current tokens more than bucket and acquire will be rejected
        OPT_UNFORCE = 1,
        
        //Force acquire, do not care the number of token
        OPT_FORCE
    };

public:
    CCounter(void);

    CCounter(apl_size_t auBucketSize);

    void SetBucketSize( apl_size_t auBucketSize );
    
    apl_size_t GetBucketSize(void);

    apl_size_t GetCurTokens(void);

    apl_int_t Acquire(apl_size_t auTokens, EOption aeOpt = OPT_UNFORCE);
  
    apl_int_t Release(apl_size_t auTokens);
    
private:
    apl_size_t muBucketSize;

    apl_size_t muTokens;
};

/////////////////////////////////////////////////////////////////////////////////////////
class CSpeeder
{
public:
    CSpeeder(void);

    CSpeeder(apl_size_t auSpeed, apl_size_t auBucketSize);

    void SetBucketSize(apl_size_t auBucketSize);

    apl_size_t GetBucketSize(void);

    apl_size_t GetSpeed(void);

    double GetCurTokens(void);

    void SetSpeed(apl_size_t auTokensPerSec);

    apl_int_t Acquire(apl_size_t auToken, acl::CTimestamp& aoTimestamp);
    
    acl::CTimestamp& GetLastTime(void);

private:
    double mdTokens;

    apl_size_t muSpeed;

    apl_size_t muBucketSize;

    acl::CTimestamp moLastTime;
};

///////////////////////////////////////////////////////////////////////////////////////////
class CTableImpl
{
public:
    struct CNode
    {
        CNode(void);
        
        CCounter* mpoCounter;
        CSpeeder* mpoSpeeder;
    };

public:
    CTableImpl(void);

    ~CTableImpl(void);

    CCounter* GetCounter(char const* apcKey, apl_int_t* apiN );

    CCounter* GetCounter(apl_int_t aiN);

    CSpeeder* GetSpeeder(char const* apcKey, apl_int_t* apiN );

    CSpeeder* GetSpeeder(apl_int_t aiN);

    apl_int_t UpdateCounter(char const* apcKey, apl_size_t auMaxConNum);

    apl_int_t UpdateSpeeder(char const* apcKey, apl_size_t auTokens, apl_size_t auMaxBucketSize);

    void Clear(void);

    void Copy(CTableImpl& aoTable);

protected:
    apl_int_t FindItem(char const* apcKey);

private:
    std::map<std::string, apl_int_t> moMap;

    std::vector<CNode> moList;
};

///////////////////////////////////////////////////////////////////////////////////////////
class CTable
{
public:
    static CTable* Instance(void);

    static void Release(void);

    CCounter* GetCounter(char const* apcKey, apl_int_t* aiN);

    CCounter* GetCounter(apl_int_t aiN);

    CSpeeder* GetSpeeder(char const* apcKey, apl_int_t* aiN);

    CSpeeder* GetSpeeder(apl_int_t aiN);

    apl_int_t UpdateCounter(char const* apcKey, apl_size_t auMaxConNum);

    apl_int_t UpdateSpeeder(char const* apcKey, apl_size_t auTokens, apl_size_t auMaxBucketSize);

    void StartUpdate(void);

    void Commit(void);

protected:
    CTable(void);
    ~CTable(void);

private:
    static CTable* cpoInstance;

    CTableImpl* mpoCurList;

    CTableImpl* mpoUpdateList;
}; 

#endif//COUNTSERVER_TABLE_H
