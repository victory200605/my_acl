
#ifndef AI_TESTCONTROL_H_2008
#define AI_TESTCONTROL_H_2008

#include "TestAction.h"

namespace AITest {

class AITestControl : public AITestAction
{
public:
    AITestControl( AITestAction* apoParentAction, int aiUnitTime, int aiUnitRepeat );
    virtual ~AITestControl();

    virtual int DoIt( void* apParam );
    virtual int TestDo( stParam& aoParam );
    virtual int RunX( stParam& aoParam );
    
    TEST_ACTION_OPERATOR();
    
protected:
    static void* ThreadControlFunc( void* apParam );
    
protected:
    AITestAction* cpoParentAction;
    
    int ciUnitTime;
    clsCount ciUnitRepeat;
    int ciUnitRepeatConst;

    AIBC::AIMutexLock coControlLock;
    AIBC::AICondition coControlCond;
};

AITestControl::AITestControl( AITestAction* apoParentAction, int aiUnitTime, int aiUnitRepeat ) :
    cpoParentAction(apoParentAction),
    ciUnitTime(aiUnitTime),
    ciUnitRepeat(aiUnitRepeat),
    ciUnitRepeatConst(aiUnitRepeat)
{
    assert( cpoParentAction != NULL );
}

AITestControl::~AITestControl()
{
}

int AITestControl::DoIt( void* apParam )
{
    return cpoParentAction->DoIt( apParam );
}

int AITestControl::TestDo( stParam& aoParam )
{
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif

    assert( cpoCase != NULL );
    
    AI_MUTEX_LOCK( coControlLock );
    AI_CONDITION_WAIT( coControlCond, coControlLock );
    AI_MUTEX_UNLOCK( coControlLock );

    while( cpoCase->GetRepeatCount() > 0 )
    {
        AI_MUTEX_LOCK( coControlLock );
         
        while( ciUnitRepeat-- >= 0 && cpoCase->DecreaseRepeatCount() >= 0 )
        {
            AI_MUTEX_UNLOCK( coControlLock );
            aoParam.cpoTestAction->DoIt( aoParam.cpParam );
            AI_MUTEX_LOCK( coControlLock );
        }
        
        AI_CONDITION_WAIT_UNTIL( coControlCond, coControlLock, ciUnitTime );
        AI_MUTEX_UNLOCK( coControlLock );
    }
    
    return 0;
}

int AITestControl::RunX( stParam& aoParam )
{   
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    
    cpParent = aoParam.cpoTestAction;
    
    stParam loParam = aoParam;
    loParam.cpoTestAction = this;
    pthread_t liThrd;
    if ( pthread_create( &liThrd, NULL, AITestControl::ThreadControlFunc, &loParam ) != 0 )
    {
            return -1;
    }
    
    return cpoParentAction->RunX( aoParam );
}

void* AITestControl::ThreadControlFunc( void* apParam )
{
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif

    pthread_detach( pthread_self() );
    
    assert( apParam != NULL );
    stParam* lpParam = (stParam*)(apParam);
    AITestControl* lpTestControl = (AITestControl*)(lpParam->cpoTestAction);
    assert( lpTestControl != NULL );
    
    while( true )
    {
        lpTestControl->ciUnitRepeat = lpTestControl->ciUnitRepeatConst;
        lpTestControl->coControlCond.Broadcast();
        
        sleep(lpTestControl->ciUnitTime);
    }
    return NULL;
}

};

#endif //AI_TESTCONTROL_H_2008
