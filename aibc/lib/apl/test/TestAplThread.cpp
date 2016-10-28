#include "Test.h"


void* threadFunc(void* apv_arg)
{
   apl_thread_yield();
   int* lpi_arg = (int*)apv_arg;
   apl_errprintf("threadid %"APL_PRIuINT"\n", apl_thread_self() );
   apl_thread_exit( (void*)lpi_arg);
   return NULL;
}

/**
 * @brief Test suite Thread.
 */
class CTestapltrd : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestapltrd);
    CPPUNIT_TEST(testThread);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}

    void tearDown(void) {}

    /** 
     * Test case testThread.
     */
    void testThread(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_r;
        apl_thread_t lt_tid;
        apl_int_t li_i =9;

        //case
        li_r = apl_thread_create(&lt_tid, threadFunc, (void*)&li_i, 0, APL_THREAD_USER);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("create thread failed\n", 0, (int)li_r);


        li_r = apl_thread_create(&lt_tid, threadFunc, (void*)&li_i, 0, APL_THREAD_KERNEL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("create thread failed\n", 0, (int)li_r);

        li_r = apl_thread_create(&lt_tid, threadFunc, (void*)&li_i, 0, APL_THREAD_USER);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("create thread failed\n", 0, (int)li_r);

        sleep(4);

        //end environment
    } 
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestapltrd);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
