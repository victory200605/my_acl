
#ifndef ATF_TESTSNAPSHOT_H_2008
#define ATF_TESTSNAPSHOT_H_2008

#include "TestAction.h"

namespace atf {
    
struct CStat
{
    CStat()
    {
        miTotalCnt = 0;
        miSuccessCnt = 0;
        miRejectCnt = 0;
        miTotalDelayTime = 0;
        miMaxDelayTime = 0;
        miMinDelayTime = 9999999;
    }
    void AddSuccess()
    {
        acl::TSmartLock<acl::CLock> loLock( coLock );
        miSuccessCnt++;
        miTotalCnt++;
    }
    void AddReject()
    {
        acl::TSmartLock<acl::CLock> loLock( coLock );
        miRejectCnt++;
        miTotalCnt++;
    }
    void AddDelayTime( apl_int_t usec )
    {
        acl::TSmartLock<acl::CLock> loLock( coLock );
        miTotalDelayTime += usec;
        if ( miMaxDelayTime < usec ) miMaxDelayTime = usec;
        if ( miMinDelayTime > usec ) miMinDelayTime = usec;
        if ( miUnitMaxDelayTime < usec ) miUnitMaxDelayTime = usec;
        if ( miUnitMinDelayTime > usec ) miUnitMinDelayTime = usec;
    }
    
    void GetStat( CStat& aoStat )
    {
        acl::TSmartLock<acl::CLock> loLock( coLock );
        aoStat.miTotalCnt = miTotalCnt; 
        aoStat.miSuccessCnt = miSuccessCnt;
        aoStat.miRejectCnt = miRejectCnt;
        aoStat.miTotalDelayTime = miTotalDelayTime;
        aoStat.miMaxDelayTime = miMaxDelayTime;
        aoStat.miMinDelayTime = miMinDelayTime;
        aoStat.miUnitMaxDelayTime = miUnitMaxDelayTime;
        aoStat.miUnitMinDelayTime = miUnitMinDelayTime;
        
        miUnitMaxDelayTime = 0;
        miUnitMinDelayTime = 9999999;
    }
    
    apl_int_t miTotalCnt;
    apl_int_t miSuccessCnt;
    apl_int_t miRejectCnt;
    apl_int64_t miTotalDelayTime;
    apl_int_t  miMaxDelayTime;
    apl_int_t  miMinDelayTime;
    apl_int_t  miUnitMaxDelayTime;
    apl_int_t  miUnitMinDelayTime;
    acl::CLock coLock;
};

class CTestSnapshot : public CTestAction
{
public:
    typedef CTestAction TBase;
    
public:
    CTestSnapshot( CTestAction* apoParentAction, apl_int_t aiInterval );
    ~CTestSnapshot();

    virtual apl_int_t DoIt( void* apParam );
    virtual apl_int_t TestDo( CParam& aoParam );
    virtual apl_int_t RunX( CParam& aoParam );
    
    apl_int_t GetAvgDelayTime()                             { return moStat.miTotalDelayTime / ( moStat.miTotalCnt > 0 ? moStat.miTotalCnt : 1 ); }
    apl_int_t GetMaxDelayTime()                             { return moStat.miMaxDelayTime; }
    apl_int_t GetMinDelayTime()                             { return moStat.miMinDelayTime; }
    
    TEST_ACTION_OPERATOR();
protected:
    static void* ThreadStatisticalFunc( void* apParam );
    
protected:
    CTestAction* mpoParentAction;
    
    apl_int_t miTimeInterval;
    bool mbIsRunning;
    
    CStat moStat;
};

CTestSnapshot::CTestSnapshot( CTestAction* apoParentAction, apl_int_t aiInterval ) : 
    mpoParentAction(apoParentAction),
    miTimeInterval(aiInterval),
    mbIsRunning(false)
{
    ACL_ASSERT(mpoParentAction != NULL);
}

CTestSnapshot::~CTestSnapshot()
{
}

apl_int_t CTestSnapshot::DoIt( void* apParam )
{
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif

    acl::CTimestamp loStart, loEnd;

    loStart.Update(acl::CTimestamp::PRC_USEC);
    
    apl_int_t liRet = mpoParentAction->DoIt( apParam );
    if ( liRet != 0 )
    {
        moStat.AddReject();
    }
    else
    {
        moStat.AddSuccess();
    }
    
    loEnd.Update(acl::CTimestamp::PRC_USEC);
    
    apl_int_t liDelayTime = loEnd.Usec() - loStart.Usec();
    if ( liDelayTime >= 0 )
    {
        moStat.AddDelayTime( liDelayTime );
    }
    
    return liRet;
}

apl_int_t CTestSnapshot::TestDo( CParam& aoParam )
{
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif

    ACL_ASSERT( mpoCase != NULL && mpoParentAction != NULL );
    
    return mpoParentAction->TestDo( aoParam );
}

apl_int_t CTestSnapshot::RunX( CParam& aoParam )
{  
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n ", __FILE__, __FUNCTION__ );
#endif
    CParam loParam = aoParam;
    loParam.mpoTestAction = this;
    
    mbIsRunning = true;
    
    pthread_t liThrd;
    if ( pthread_create( &liThrd, NULL, CTestSnapshot::ThreadStatisticalFunc, &loParam ) != 0 )
    {
        return -1;
    }
    
    apl_int_t liRet = mpoParentAction->RunX( aoParam );
    mbIsRunning = false;
    pthread_join( liThrd, NULL );
    
    return liRet;
}

void* CTestSnapshot::ThreadStatisticalFunc( void* apParam )
{
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    
    assert( apParam != NULL );
    CParam* lpParam = (CParam*)(apParam);
    CTestSnapshot* lpTestSnapshot = (CTestSnapshot*)lpParam->mpoTestAction;
    ACL_ASSERT(lpTestSnapshot != NULL);
    acl::CDateTime loTime;
    
    FILE* lpFile = fopen( "statistical", "w" );
    if ( lpFile == NULL )
    {
        printf( "Open statistical file fail! - %s\n", strerror(errno) );
        return NULL;
    }
    CStat loStat;
    while( lpTestSnapshot->mbIsRunning )
    {
        sleep( lpTestSnapshot->miTimeInterval );
        
        loTime.Update();
        
        CStat loStatNext;
        lpTestSnapshot->moStat.GetStat( loStatNext );
        fprintf( lpFile, "------------------------- Time Point %s -------------------------\n", loTime.Format("%Y-%m-%d %H:%M:%S") );
        fprintf( lpFile, " Complete record    = %"APL_PRIdINT"\n", (apl_int_t)(loStatNext.miTotalCnt - loStat.miTotalCnt) );
        fprintf( lpFile, " Success record     = %"APL_PRIdINT"\n", (apl_int_t)(loStatNext.miSuccessCnt - loStat.miSuccessCnt) );
        fprintf( lpFile, " Reject record      = %"APL_PRIdINT"\n", (apl_int_t)(loStatNext.miRejectCnt - loStat.miRejectCnt) );
        fprintf( lpFile, " Avg delay time(us) = %"APL_PRId64"\n", (apl_int64_t)(( loStatNext.miTotalDelayTime - loStat.miTotalDelayTime ) /
                  ( (loStatNext.miTotalCnt - loStat.miTotalCnt) <= 0 ? 1 : (loStatNext.miTotalCnt - loStat.miTotalCnt) ) ) );
        fprintf( lpFile, " Max delay time(us) = %"APL_PRIdINT"\n", (apl_int_t)loStatNext.miUnitMaxDelayTime );
        fprintf( lpFile, " Min delay time(us) = %"APL_PRIdINT"\n", (apl_int_t)loStatNext.miUnitMinDelayTime );
        fflush(lpFile);
        loStatNext.GetStat( loStat );
    }
    fclose(lpFile);
    
    return NULL;
}

};

#endif //AI_TESTSNAPSHOT_H_2008
