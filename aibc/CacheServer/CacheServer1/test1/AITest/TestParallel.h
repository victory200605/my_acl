
#ifndef AI_TESTPARALLEL_H_2008
#define AI_TESTPARALLEL_H_2008

#include "TestAction.h"
#include <string.h>
#include <errno.h>

namespace AITest {

//-------------------------------AITestSuit-------------------------------//
class AITestParallel : public AITestAction
{
public:
    typedef void* (*ThreadFunc)( void* apParam );
    
public:
    AITestParallel( int aiParallel );
    virtual ~AITestParallel();

    virtual int RunX( stParam& aoParam );

protected:
    void SetThreadDoFunc( ThreadFunc apThreadFunc );
    static void* ThreadDoFunc( void* apParam );
    
protected:
    int ciParallel;
    pthread_t* cpThreadHandle;
    ThreadFunc cpfThreadDo;
};

inline AITestParallel::AITestParallel( int aiParallel ) :
    ciParallel(aiParallel)
    
{
    assert( ciParallel > 0 );
    
    SetThreadDoFunc( AITestParallel::ThreadDoFunc );
    
    cpThreadHandle = new pthread_t[ciParallel];
    memset( cpThreadHandle, 0, sizeof(pthread_t) * ciParallel );
}

inline AITestParallel::~AITestParallel()
{
    delete[] cpThreadHandle;
}

inline int AITestParallel::RunX( stParam& aoParam )
{   
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif

    stParam loParam = aoParam;
    
    time_t liStart;
    time( &liStart );
    int liIt = 0;
    for( liIt = 0; liIt < ciParallel; liIt++ )
    {
        if ( pthread_create( &cpThreadHandle[liIt], NULL, cpfThreadDo, &loParam ) != 0 )
        {
            return -1;
        }
    }
    // wait
    for( liIt = 0; liIt < ciParallel; liIt++ )
    {
        pthread_join( cpThreadHandle[liIt], NULL );
    }
    
    cpoCase->SetUseTime( time(NULL) - liStart );
    
    return 0;
}

inline void AITestParallel::SetThreadDoFunc( ThreadFunc apThreadFunc )
{
    cpfThreadDo = apThreadFunc;
}

inline void* AITestParallel::ThreadDoFunc( void* apParam )
{
#if defined(AI_TEST_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    stParam* lpoParam = (stParam*)apParam;
    assert( apParam != NULL && lpoParam->cpoTestAction != NULL );
    
    lpoParam->cpoTestAction->TestDo( *lpoParam );
    
    return NULL;
}

};

#endif //AI_TESTPARALLEL_H_2008
