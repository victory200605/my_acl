
#ifndef ATF_TESTPARALLEL_H_2008
#define ATF_TESTPARALLEL_H_2008

#include "TestAction.h"
#include "acl/stl/string.h"

namespace atf {

//-------------------------------AITestSuit-------------------------------//
class CTestParallel : public CTestAction
{
public:
    typedef void* (*ThreadFunc)( void* apParam );
    
public:
    CTestParallel( apl_int_t aiParallel );
    virtual ~CTestParallel();

    virtual apl_int_t RunX( CParam& aoParam );

protected:
    void SetThreadDoFunc( ThreadFunc apThreadFunc );
    static void* ThreadDoFunc( void* apParam );
    
protected:
    apl_int_t miParallel;
    ThreadFunc mpfThreadDo;
    pthread_t* mpThreadHandle;
};

inline CTestParallel::CTestParallel( apl_int_t aiParallel ) :
    miParallel(aiParallel)
    
{
    ACL_ASSERT(miParallel > 0);
    
    SetThreadDoFunc(CTestParallel::ThreadDoFunc);
    mpThreadHandle = new pthread_t[miParallel];
    apl_memset(mpThreadHandle, 0, sizeof(pthread_t) * miParallel );
}

inline CTestParallel::~CTestParallel()
{
    delete[] mpThreadHandle;
}

inline apl_int_t CTestParallel::RunX( CParam& aoParam )
{   
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif

    acl::CTimestamp loStart, loEnd;
    CParam loParam = aoParam;
    
    loStart.Update();

    apl_int_t liIt = 0;
    for( liIt = 0; liIt < miParallel; liIt++ )
    {
        if ( pthread_create( &mpThreadHandle[liIt], NULL, mpfThreadDo, &loParam ) != 0 )
        {
            return -1;
        }
    }

    // wait
    for( liIt = 0; liIt < miParallel; liIt++ )
    {
        pthread_join( mpThreadHandle[liIt], NULL );
    }
    
    loEnd.Update();
    
    mpoCase->SetUseTime(loEnd.Sec() - loStart.Sec() );
    
    return 0;
}

inline void CTestParallel::SetThreadDoFunc( ThreadFunc apThreadFunc )
{
    mpfThreadDo = apThreadFunc;
}

inline void* CTestParallel::ThreadDoFunc( void* apParam )
{
#if defined(_ATF_DEBUG)
    printf( "%s::%s:do ...\n", __FILE__, __FUNCTION__ );
#endif
    CParam* lpoParam = (CParam*)apParam;
    ACL_ASSERT( apParam != NULL && lpoParam->mpoTestAction != NULL );
    
    lpoParam->mpoTestAction->TestDo( *lpoParam );
    
    return NULL;
}

};

#endif //AI_TESTPARALLEL_H_2008
