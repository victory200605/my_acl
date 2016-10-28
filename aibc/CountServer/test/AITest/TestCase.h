
#ifndef AI_TESTCASE_H_2008
#define AI_TESTCASE_H_2008

#include "acl/Synch.h"
#include "acl/stl/string.h"

// macro define

#define AI_MUTEX_LOCK( mutex ) mutex.Lock();
#define AI_MUTEX_UNLOCK( mutex ) mutex.Unlock();
#define AI_CONDITION_WAIT( cond, lock ) cond.Wait(lock);
#define AI_CONDITION_WAIT_UNTIL( cond, lock, time ) cond.Wait(lock, time);
#define AI_SLEEP_US( us )  apl_sleep( APL_TIME_MSEC );

namespace AITest {
    
// multi-thread counter
class clsCount
{
public:
    // constructor
    clsCount( int aiCount = 0 ) : ciCount(aiCount) {}
    
    // operator = over write
    clsCount& operator = ( int aiN ) 
    { 
        AI_MUTEX_LOCK(coLock)
        ciCount = aiN; 
        AI_MUTEX_UNLOCK(coLock)
        return *this;
    }
    
    int operator ++ ( int )
    {
        AI_MUTEX_LOCK(coLock)
        int liTmp = ++ciCount;
        AI_MUTEX_UNLOCK(coLock)
        return liTmp;
    }
    
    int operator -- ( int )
    {
        AI_MUTEX_LOCK(coLock)
        int liTmp = --ciCount;
        AI_MUTEX_UNLOCK(coLock)
        return liTmp;
    }
    
    operator int ()
    {
        AI_MUTEX_LOCK(coLock)
        int liTmp = ciCount;
        AI_MUTEX_UNLOCK(coLock)
        return liTmp;
    }
    
protected:
    int ciCount;
    acl::CLock coLock;
};

//-------------------------------AITestSuit-------------------------------//
class AITestCase
{
public:
    struct stStat
    {
        stStat() : ciUseTime(0) {};
        
        clsCount ciSuccess;
        clsCount ciReject;
        double ciUseTime;
    };
    
public:
    AITestCase();
    AITestCase( int aiCount );
    virtual ~AITestCase();
    
    virtual void SetRepeatCount( int aiCount )       { ciRepeatCount = aiCount; }

    virtual int DoIt( void* apParam ) = 0;
    
    void AddSuccess()                    { coStat.ciSuccess++; }
    void AddReject()                     { coStat.ciReject++; }
    void SetUseTime( double aiUseTime )  { coStat.ciUseTime = aiUseTime; }
    
    int IncreaseRepeatCount()            { return ciRepeatCount++; }
    int DecreaseRepeatCount()            { return ciRepeatCount--; }
    int GetRepeatCount()                 { return ciRepeatCount; }
    
    int GetSuccess()                     { return coStat.ciSuccess; }
    int GetReject()                      { return coStat.ciReject; }
    double GetUseTime()                  { return coStat.ciUseTime; }
    double GetFrequency()                { return ( (int)coStat.ciSuccess + (int)coStat.ciReject ) / ( coStat.ciUseTime > 0 ? coStat.ciUseTime : 1 ); }

protected:
    clsCount ciRepeatCount;
    stStat coStat;
};

inline AITestCase::AITestCase()
{
    ciRepeatCount = 1;
}

inline AITestCase::AITestCase( int aiCount )
{
    ciRepeatCount = aiCount;
}

inline AITestCase::~AITestCase()
{}

};

#endif //AI_TESTSUIT_H_2008
