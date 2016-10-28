
#ifndef ATF_TESTCONTROL_H_2008
#define ATF_TESTCONTROL_H_2008

#include "TestAction.h"
#include "acl/Thread.h"

namespace atf {

class CTestControl : public CTestAction
{
public:
    CTestControl( CTestAction* apoParentAction, apl_int_t aiUnitTime, apl_int_t aiUnitRepeat );
    virtual ~CTestControl();

    virtual apl_int_t DoIt( void* apParam );
    virtual apl_int_t TestDo( CParam& aoParam );
    virtual apl_int_t RunX( CParam& aoParam );
    
    TEST_ACTION_OPERATOR();
    
protected:
    static void* ThreadControlFunc( void* apParam );
    
protected:
    CTestAction* mpoParentAction;
    
    apl_int_t miUnitTime;
    acl::TNumber<apl_int_t, acl::CLock> miUnitRepeat;
    apl_int_t miUnitRepeatConst;

    acl::CLock moControlLock;
    acl::CCondition moControlCond;
};

CTestControl::CTestControl( CTestAction* apoParentAction, apl_int_t aiUnitTime, apl_int_t aiUnitRepeat ) :
    mpoParentAction(apoParentAction),
    miUnitTime(aiUnitTime),
    miUnitRepeat(aiUnitRepeat),
    miUnitRepeatConst(aiUnitRepeat)
{
    ACL_ASSERT( mpoParentAction != NULL );
}

CTestControl::~CTestControl()
{
}

apl_int_t CTestControl::DoIt( void* apParam )
{
    return mpoParentAction->DoIt( apParam );
}

apl_int_t CTestControl::TestDo( CParam& aoParam )
{
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif

    ACL_ASSERT( mpoCase != NULL );
    
    moControlLock.Lock();
    moControlCond.Wait(moControlLock);
    moControlLock.Unlock();

    while( mpoCase->GetRepeatCount() > 0 )
    {
        moControlLock.Lock();
         
        while( miUnitRepeat-- >= 0 && mpoCase->DecreaseRepeatCount() >= 0 )
        {
            moControlLock.Unlock();
            aoParam.mpoTestAction->DoIt( aoParam.mpParam );
            moControlLock.Lock();
        }
        
        moControlCond.Wait(moControlLock, acl::CTimeValue(miUnitTime) );
        moControlLock.Unlock();
    }
    
    return 0;
}

apl_int_t CTestControl::RunX( CParam& aoParam )
{   
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    
    mpParent = aoParam.mpoTestAction;
    
    CParam loParam = aoParam;
    loParam.mpoTestAction = this;
    if ( acl::CThread::Spawn(CTestControl::ThreadControlFunc, &loParam ) != 0 )
    {
        return -1;
    }
    
    return mpoParentAction->RunX( aoParam );
}

void* CTestControl::ThreadControlFunc( void* apParam )
{
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    ACL_ASSERT(apParam != NULL);
    CParam* lpParam = (CParam*)(apParam);
    CTestControl* lpTestControl = (CTestControl*)(lpParam->mpoTestAction);
    ACL_ASSERT(lpTestControl != NULL);
    
    while( true )
    {
        lpTestControl->miUnitRepeat = lpTestControl->miUnitRepeatConst;
        lpTestControl->moControlCond.Broadcast();
        
        apl_sleep(lpTestControl->miUnitTime * APL_TIME_SEC);
    }

    return NULL;
}

};

#endif //ATF_TESTCONTROL_H_2008
