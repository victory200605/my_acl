
#ifndef ATF_TESTCASE_H
#define ATF_TESTCASE_H

#include "acl/stl/string.h"
#include "acl/Synch.h"
#include "acl/Number.h"
#include "acl/Timestamp.h"

namespace atf {
    
//-------------------------------CTestCase-------------------------------//
class CTestCase
{
public:
    struct CStat
    {
        CStat() : mdUseTime(0) {};
        
        acl::TNumber<apl_int_t, acl::CLock> miSuccess;
        acl::TNumber<apl_int_t, acl::CLock> miReject;
        double mdUseTime;
    };
    
public:
    CTestCase();
    CTestCase( apl_int_t aiCount );
    virtual ~CTestCase();
    
    virtual void SetRepeatCount( apl_int_t aiCount )       { miRepeatCount = aiCount; }

    virtual apl_int_t DoIt( void* apParam ) = 0;
    
    void AddSuccess()                    { moStat.miSuccess++; }
    void AddReject()                     { moStat.miReject++; }
    void SetUseTime( double adUseTime )  { moStat.mdUseTime = adUseTime; }
    
    apl_int_t IncreaseRepeatCount()            { return ++miRepeatCount; }
    apl_int_t DecreaseRepeatCount()            { return --miRepeatCount; }
    apl_int_t GetRepeatCount()                 { return miRepeatCount; }
    
    apl_int_t GetSuccess()                     { return moStat.miSuccess; }
    apl_int_t GetReject()                      { return moStat.miReject; }
    double GetUseTime()                  { return moStat.mdUseTime; }
    double GetFrequency()                { return (moStat.miSuccess + moStat.miReject) / ( moStat.mdUseTime > 0 ? moStat.mdUseTime : 1 ); }

protected:
    acl::TNumber<apl_int_t, acl::CLock> miRepeatCount;
    CStat moStat;
};

inline CTestCase::CTestCase()
{
    miRepeatCount = 1;
}

inline CTestCase::CTestCase( apl_int_t aiCount )
{
    miRepeatCount = aiCount;
}

inline CTestCase::~CTestCase()
{}

};

#endif //AI_TESTSUIT_H_2008
