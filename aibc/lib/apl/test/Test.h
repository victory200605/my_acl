#include "apl/apl.h"
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

void* ThreadFunc(void* apvParam)
{
    IThreadBody* lpoBody = (IThreadBody*)(apvParam);

    lpoBody->DoIt();

    delete lpoBody;

    return NULL;
}

void* THR_NULL = NULL;
 
#define START_THREAD_BODY( name, paramtype, param ) \
    IThreadBody* name = NULL; \
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
        apl_thread_t _id; \
        apl_thread_create(&_id, ThreadFunc, name, 0, 0); \
    }

#define WAIT_EXP( exp ) \
    while( !(exp) ) { apl_sleep(10*APL_TIME_MSEC); };

    
#define START_FUNC_BODY( rettype, ... ) \
  struct { \
    rettype operator () ( __VA_ARGS__ ) \
    {

#define END_FUNC_BODY(name) \
    } \
  } name;
