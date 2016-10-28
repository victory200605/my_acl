#include "apl/apl.h"
#include "acl/Synch.h"
#include "cppunit/TestCase.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/ui/text/TestRunner.h"
#include "cppunit/TestCaller.h"
#include "cppunit/extensions/HelperMacros.h"

/* ---------------------------------------------------------------------- */

void PrintTitle(char const* apc_title, char ac_sep, apl_size_t au_repeat)
{
    apl_size_t  lu_itr;

    for (lu_itr = 0; lu_itr < au_repeat; ++lu_itr)
    {
        apl_errprintf("%c", ac_sep);
    }

    apl_errprintf(" %s ", apc_title);

    for (lu_itr = 0; lu_itr < au_repeat; ++lu_itr)
    {
        apl_errprintf("%c", ac_sep);
    }

    apl_errprintf("\n");
}

#define PRINT_TITLE_1(title) PrintTitle(title, '*', 20)
#define PRINT_TITLE_2(title) PrintTitle(title, '=', 20)
#define PRINT_TITLE_3(title) PrintTitle(title, '-', 20)

/* ---------------------------------------------------------------------- */

void PrintErrMsg(char const* apc_msg, apl_int_t ai_num)
{
    if ( APL_INT_C(-1) == ai_num )
    {
        apl_perror(apc_msg);
        CPPUNIT_FAIL(apc_msg);
    }
}
/* ---------------------------------------------------------------------- */

#define RUN_AND_CHECK_RET_INT(func, args) \
    do { \
        apl_int_t li_ret = func args; \
        if (li_ret == APL_INT_C(-1)) \
        { \
            apl_perror(#func #args); \
            CPPUNIT_FAIL(#func #args "failed"); \
        } \
    } while(0)

/* ---------------------------------------------------------------------- */

#define RUN_AND_CHECK_RET_ERRNO(func, args) \
    do { \
        apl_int_t li_ret = func args; \
        if (li_ret != 0) \
        { \
            apl_errprintf(#func #args "failed, %s", apl_strerror(li_ret)); \
            CPPUNIT_FAIL(#func #args "failed"); \
        } \
    } while(0)

/* ---------------------------------------------------------------------- */

#define RUN_ALL_TEST(title) \
    do { \
        CppUnit::TextUi::TestRunner runner; \
        CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry(); \
        runner.addTest( registry.makeTest() ); \
        \
        PRINT_TITLE_1(title); \
        \
        if (runner.run()) \
        { \
            return 0; \
        } \
        else \
        { \
            return 1; \
        } \
    } while(0)

/* ---------------------------------------------------------------------- */

struct IThreadBody
{    
    virtual void DoIt() = 0;
};

struct CAutoJoinThread
{
    CAutoJoinThread(void) 
        : miThrd(0)
    { 
    }
    
    ~CAutoJoinThread(void)
    {
        if (miThrd != 0)
            pthread_join(miThrd, NULL);
    }
    
    pthread_t miThrd;
};

void* ThreadFunc(void* apvParam)
{
    IThreadBody* lpoBody = (IThreadBody*)(apvParam);
    
    lpoBody->DoIt();
    
    //delete lpoBody;
    
    return NULL;
}

void* THR_NULL = NULL;

#define START_THREAD_BODY( name, paramtype, param ) \
    IThreadBody* name = NULL; \
    CAutoJoinThread name##auto; \
    { \
        paramtype& _param = param; \
        struct _threadbody : public IThreadBody \
        { \
            paramtype& param; \
            _threadbody(paramtype& apvParam) \
            : param(apvParam) \
            { \
            } \
            virtual void DoIt() \
            {

#define END_THREAD_BODY( name ) \
            } \
        }; \
        name = new _threadbody(_param); \
    }

#define RUN_THREAD_BODY( name ) \
    { \
        pthread_create(&name##auto.miThrd, NULL, ThreadFunc, name); \
    }

#define RUN_THREAD_BODY_DETACH( name ) \
    { \
        apl_thread_t liThrd; \
        apl_thread_create(&liThrd, ThreadFunc, name, 0, APL_THREAD_USER); \
    }

#define WAIT_EXP( exp ) \
    {apl_int_t liNum= 0; \
    while( !(exp) && liNum<1000 ) { apl_sleep(10*APL_TIME_MSEC); ++liNum; };}
    
#define START_FUNC_BODY( rettype, ... ) \
    struct { \
        rettype operator () ( __VA_ARGS__ ) \
        {

#define END_FUNC_BODY(name) \
        } \
    } name;

#define ASSERT_MESSAGE( exp ) \
{ \
    bool lbAssert = exp; \
    char lacMessage[1024]; \
    apl_snprintf(lacMessage, sizeof(lacMessage), "%s:Line %d", #exp, __LINE__); \
    CPPUNIT_ASSERT_MESSAGE(lacMessage, lbAssert); \
}

#define START_LOOP( count ) \
{ \
    acl::CTimestamp __loStart, __loEnd; \
    __loStart.Update(acl::CTimestamp::PRC_USEC); \
    apl_int_t __liCount = count; \
    for( apl_int_t __liN = 0; __liN < __liCount; __liN++ )

#define END_LOOP() \
    __loEnd.Update(acl::CTimestamp::PRC_USEC); \
    double __ldUseTime = double( __loEnd.Msec() - __loStart.Msec() ) / 1000; \
    printf("Use time=%lf, %lf/pre\n", __ldUseTime, double(__liCount) / __ldUseTime); \
}
/* ---------------------------------------------------------------------- */
