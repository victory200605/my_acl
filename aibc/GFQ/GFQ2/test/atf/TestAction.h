
#ifndef ATF_TESTACTION_H
#define ATF_TESTACTION_H

#include "TestCase.h"

namespace atf {

class CTestAction;

// common thread param
struct CParam
{
    CTestAction* mpoTestAction;
    void* mpParam;
};

#define TEST_ACTION_OPERATOR( ) \
    virtual void SetTestCase( CTestCase* apoCase ) \
    { \
        mpoCase = apoCase; \
        if ( mpoParentAction != NULL ) mpoParentAction->SetTestCase( apoCase ); \
    } \
    CTestCase* GetTestCase() \
    { \
        return mpoCase; \
    } 

//-------------------------------CTestSuit-------------------------------//
class CTestAction
{
public:
    CTestAction();
    virtual ~CTestAction();
    virtual void SetTestCase( CTestCase* apoCase );
    
    virtual apl_int_t Run( void* apParam );
    virtual apl_int_t DoIt( void* apParam );
    
    virtual apl_int_t TestDo( CParam& aoParam );
    virtual apl_int_t RunX( CParam& aoParam );
    
    CTestAction* GetParent()                         { return mpParent; }
    
protected:
    CTestCase* mpoCase;
    
    CTestAction* mpParent;
};

inline CTestAction::CTestAction()
{
}

inline CTestAction::~CTestAction()
{}

void CTestAction::SetTestCase( CTestCase* apoCase )
{
    mpoCase = apoCase;
}

apl_int_t CTestAction::DoIt( void* apParam )
{
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    apl_int_t liRet = 0;
    if ( ( liRet = mpoCase->DoIt( apParam ) ) == 0 )
    {
        mpoCase->AddSuccess();
    }
    else
    {
        mpoCase->AddReject();
    }
    
    return liRet;
}

inline apl_int_t CTestAction::TestDo( CParam& aoParam )
{
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    ACL_ASSERT(mpoCase != NULL);
    
    while(mpoCase->DecreaseRepeatCount() > 0)
    {
        aoParam.mpoTestAction->DoIt( aoParam.mpParam );
    }
    
    return 0;
}

inline apl_int_t CTestAction::Run( void* apParam )
{
    acl::CTimestamp loStart, loEnd;

    loStart.Update(acl::CTimestamp::PRC_USEC);
    
    CParam loParam;
    loParam.mpoTestAction = this;
    loParam.mpParam = apParam;

    RunX(loParam);
    
    loEnd.Update(acl::CTimestamp::PRC_USEC);

    mpoCase->SetUseTime(float(loEnd.Nsec() - loStart.Nsec() ) / APL_TIME_SEC);
    
    return 0;
}

inline apl_int_t CTestAction::RunX( CParam& aoParam )
{
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    aoParam.mpoTestAction->TestDo( aoParam );
    return 0;
}

};

#endif //ATF_TESTACTION_H_2008
