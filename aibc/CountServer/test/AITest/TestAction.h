
#ifndef AI_TESTACTION_H_2008
#define AI_TESTACTION_H_2008

#include "acl/Timestamp.h"
#include "TestCase.h"

namespace AITest {

class AITestAction;

// common thread param
struct stParam
{
    AITestAction* cpoTestAction;
    void* cpParam;
};

#define TEST_ACTION_OPERATOR( ) \
    virtual void SetTestCase( AITestCase* apoCase ) \
    { \
        cpoCase = apoCase; \
        if ( cpoParentAction != NULL ) cpoParentAction->SetTestCase( apoCase ); \
    } \
    AITestCase* GetTestCase() \
    { \
        return cpoCase; \
    } 

//-------------------------------AITestSuit-------------------------------//
class AITestAction
{
public:
    AITestAction();
    virtual ~AITestAction();
    virtual void SetTestCase( AITestCase* apoCase );
    
    virtual int Run( void* apParam );
    virtual int DoIt( void* apParam );
    
    virtual int TestDo( stParam& aoParam );
    virtual int RunX( stParam& aoParam );
    
    AITestAction* GetParent()                         { return cpParent; }
    
protected:
    AITestCase* cpoCase;
    
    AITestAction* cpParent;
};

inline AITestAction::AITestAction()
{
}

inline AITestAction::~AITestAction()
{}

void AITestAction::SetTestCase( AITestCase* apoCase )
{
    cpoCase = apoCase;
}

int AITestAction::DoIt( void* apParam )
{
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    int liRet = 0;
    if ( ( liRet = cpoCase->DoIt( apParam ) ) == 0 )
    {
        cpoCase->AddSuccess();
    }
    else
    {
        cpoCase->AddReject();
    }
    
    return liRet;
}

inline int AITestAction::TestDo( stParam& aoParam )
{
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    assert( cpoCase != NULL );
    
    while( cpoCase->DecreaseRepeatCount() >= 0 )
    {
        aoParam.cpoTestAction->DoIt( aoParam.cpParam );
        //cpoCase->DecreaseRepeatCount();
    }
    
    return 0;
}

inline int AITestAction::Run( void* apParam )
{
    acl::CTimestamp loStart;
    acl::CTimestamp loEnd;
    
    loStart.Update(acl::CTimestamp::PRC_USEC);
        
    stParam loParam;
    loParam.cpoTestAction = this;
    loParam.cpParam = apParam;

    RunX( loParam );
    
    loEnd.Update(acl::CTimestamp::PRC_USEC);

    cpoCase->SetUseTime( float( loEnd.Usec() - loStart.Usec() ) / 1000000 );
    
    return 0;
}

inline int AITestAction::RunX( stParam& aoParam )
{
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    aoParam.cpoTestAction->TestDo( aoParam );
    return 0;
}

};

#endif //AI_TESTACTION_H_2008
