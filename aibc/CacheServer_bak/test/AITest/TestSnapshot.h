
#ifndef AI_TESTSNAPSHOT_H_2008
#define AI_TESTSNAPSHOT_H_2008

#include "AITime.h"
#include "TestAction.h"

namespace AITest {
    
struct stStat
{
    stStat()
    {
        ciTotalCnt = 0;
        ciSuccessCnt = 0;
        ciRejectCnt = 0;
        ciTotalDelayTime = 0;
        ciMaxDelayTime = 0;
        ciMinDelayTime = 9999999;
    }
    void AddSuccess()
    {
        AIBC::AISmartLock loLock( coLock );
        ciSuccessCnt++;
        ciTotalCnt++;
    }
    void AddReject()
    {
        AIBC::AISmartLock loLock( coLock );
        ciRejectCnt++;
        ciTotalCnt++;
    }
    void AddDelayTime( int usec )
    {
        AIBC::AISmartLock loLock( coLock );
        ciTotalDelayTime += usec;
        if ( ciMaxDelayTime < usec ) ciMaxDelayTime = usec;
        if ( ciMinDelayTime > usec ) ciMinDelayTime = usec;
        if ( ciUnitMaxDelayTime < usec ) ciUnitMaxDelayTime = usec;
        if ( ciUnitMinDelayTime > usec ) ciUnitMinDelayTime = usec;
    }
    
    void GetStat( stStat& aoStat )
    {
        AIBC::AISmartLock loLock( coLock );
        aoStat.ciTotalCnt = ciTotalCnt; 
        aoStat.ciSuccessCnt = ciSuccessCnt;
        aoStat.ciRejectCnt = ciRejectCnt;
        aoStat.ciTotalDelayTime = ciTotalDelayTime;
        aoStat.ciMaxDelayTime = ciMaxDelayTime;
        aoStat.ciMinDelayTime = ciMinDelayTime;
        aoStat.ciUnitMaxDelayTime = ciUnitMaxDelayTime;
        aoStat.ciUnitMinDelayTime = ciUnitMinDelayTime;
        
        ciUnitMaxDelayTime = 0;
        ciUnitMinDelayTime = 9999999;
    }
    
    int ciTotalCnt;
    int ciSuccessCnt;
    int ciRejectCnt;
    long long ciTotalDelayTime;
    int ciMaxDelayTime;
    int ciMinDelayTime;
    int ciUnitMaxDelayTime;
    int ciUnitMinDelayTime;
    AIBC::AIMutexLock coLock;
};

class AITestSnapshot : public AITestAction
{
public:
    typedef AITestAction TBase;
    
public:
    AITestSnapshot( AITestAction* apoParentAction, int aiInterval );
    ~AITestSnapshot();

    virtual int DoIt( void* apParam );
    virtual int TestDo( stParam& aoParam );
    virtual int RunX( stParam& aoParam );
    
    int GetAvgDelayTime()                             { return coStat.ciTotalDelayTime / ( coStat.ciTotalCnt > 0 ? coStat.ciTotalCnt : 1 ); }
    int GetMaxDelayTime()                             { return coStat.ciMaxDelayTime; }
    int GetMinDelayTime()                             { return coStat.ciMinDelayTime; }
    
    TEST_ACTION_OPERATOR();
protected:
    static void* ThreadStatisticalFunc( void* apParam );
    
protected:
    AITestAction* cpoParentAction;
    
    int ciTimeInterval;
    bool cbIsRunning;
    
    stStat coStat;
};

AITestSnapshot::AITestSnapshot( AITestAction* apoParentAction, int aiInterval ) : 
    cpoParentAction(apoParentAction),
    ciTimeInterval(aiInterval),
    cbIsRunning(false)
{
    assert( cpoParentAction != NULL );
}

AITestSnapshot::~AITestSnapshot()
{
}

int AITestSnapshot::DoIt( void* apParam )
{
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif

    struct timeval  loStart, loEnd;
    gettimeofday(&loStart , NULL);
    
    int liRet = cpoParentAction->DoIt( apParam );
    if ( liRet != 0 )
    {
        coStat.AddReject();
    }
    else
    {
        coStat.AddSuccess();
    }
    
    gettimeofday(&loEnd , NULL);
    int liDelayTime = 1000000 * ( loEnd.tv_sec - loStart.tv_sec ) + ( loEnd.tv_usec - loStart.tv_usec );
    if ( liDelayTime >= 0 )
    {
        coStat.AddDelayTime( liDelayTime );
    }
    
    return liRet;
}

int AITestSnapshot::TestDo( stParam& aoParam )
{
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif

    assert( cpoCase != NULL && cpoParentAction != NULL );
    
    return cpoParentAction->TestDo( aoParam );
}

int AITestSnapshot::RunX( stParam& aoParam )
{  
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n ", __FILE__, __FUNCTION__ );
#endif
    stParam loParam = aoParam;
    loParam.cpoTestAction = this;
    
    cbIsRunning = true;
    
    pthread_t liThrd;
    if ( pthread_create( &liThrd, NULL, AITestSnapshot::ThreadStatisticalFunc, &loParam ) != 0 )
    {
        return -1;
    }
    
    int liRet = cpoParentAction->RunX( aoParam );
    cbIsRunning = false;
    pthread_join( liThrd, NULL );
    
    return liRet;
}

void* AITestSnapshot::ThreadStatisticalFunc( void* apParam )
{
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    
    assert( apParam != NULL );
    stParam* lpParam = (stParam*)(apParam);
    AITestSnapshot* lpTestSnapshot = (AITestSnapshot*)lpParam->cpoTestAction;
    assert( lpTestSnapshot != NULL );
    char lsTimeBuffer[100] = {0};
    
    FILE* lpFile = fopen( "statistical", "w" );
    if ( lpFile == NULL )
    {
        printf( "Open statistical file fail! - %s\n", strerror(errno) );
        return NULL;
    }
    stStat loStat;
    while( lpTestSnapshot->cbIsRunning )
    {
        sleep( lpTestSnapshot->ciTimeInterval );
        
        stStat loStatNext;
        lpTestSnapshot->coStat.GetStat( loStatNext );
        fprintf( lpFile, "------------------------- Time Point %s -------------------------\n", AIBC::GetTimeString( lsTimeBuffer, 0, 1 ) );
        fprintf( lpFile, " Complete record    = %d\n", loStatNext.ciTotalCnt - loStat.ciTotalCnt );
        fprintf( lpFile, " Success record     = %d\n", loStatNext.ciSuccessCnt - loStat.ciSuccessCnt );
        fprintf( lpFile, " Reject record      = %d\n", loStatNext.ciRejectCnt - loStat.ciRejectCnt );
        fprintf( lpFile, " Avg delay time(us) = %lld\n", ( loStatNext.ciTotalDelayTime - loStat.ciTotalDelayTime ) /
                  ( (loStatNext.ciTotalCnt - loStat.ciTotalCnt) <= 0 ? 1 : (loStatNext.ciTotalCnt - loStat.ciTotalCnt) ) );
        fprintf( lpFile, " Max delay time(us) = %d\n", loStatNext.ciUnitMaxDelayTime );
        fprintf( lpFile, " Min delay time(us) = %d\n", loStatNext.ciUnitMinDelayTime );
        fflush(lpFile);
        loStatNext.GetStat( loStat );
    }
    fclose(lpFile);
    
    return NULL;
}

};

#endif //AI_TESTSNAPSHOT_H_2008
