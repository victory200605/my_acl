#include "Test.h"



#define TEST_JNDU(message, stime, num) \
    do { \
        CPPUNIT_ASSERT_EQUAL_MESSAGE(#message" precision failed\n", 0, (int)(stime%num) ); \
    } while(0) 

/* if (oper < 0)  today - days
 * if (oper >=0)  today + days       
*/
#define TEST_TIME(days, oper)  \
    do { \
        char const *lpc_chr = "Today is %a, day %d of %b %Y, time %H:%M:%S"; \
        char lac_chr[128]; \
        struct apl_tm_t ls_srt; \
        apl_time_t lt_time1 = days*APL_TIME_DAY; \
        apl_time_t lt_time2 = apl_gettimeofday(); \
        apl_time_t lt_time = 0 ; \
        if ( oper < 0 ) \
        { \
           lt_time = lt_time2 - lt_time1; \
        }else \
        { \
            lt_time = lt_time2 + lt_time1; \
        } \
        if ( APL_NULL == apl_strftime(lac_chr, sizeof(lac_chr), lpc_chr,  \
                                       apl_localtime(lt_time, &ls_srt) \
                                     )  ) \
        { \
            CPPUNIT_FAIL("aplstrftime failed"); \
        } \
        printf("apl_strftime:"); \
        printf(lac_chr); \
        printf("\n"); \
        struct apl_tm_t ls_srt1; \
        if ( APL_NULL == apl_strptime(lac_chr, lpc_chr, &ls_srt1) ) \
        { \
            CPPUNIT_FAIL("aplstrptime failed "); \
        } \
        char lac_chr1[128]; \
        if ( APL_NULL == apl_asctime(&ls_srt1, lac_chr1, sizeof(lac_chr1) )  ) \
        { \
           CPPUNIT_FAIL("apl_asctime failed"); \
        } \
        printf("apl_asctime after apl_strptime :"); \
        printf(lac_chr1); \
        printf("\n");     \
        if ( (lt_time/APL_TIME_SEC) != (apl_mktime(&ls_srt1)/APL_TIME_SEC) )  \
        { \
           CPPUNIT_FAIL("apl_mktime failed");   \
        } \
        if ( APL_NULL == apl_gmtime(lt_time, &ls_srt1) ) \
        { \
           CPPUNIT_FAIL("apl_gmtime failed"); \
        } \
    } while(0) 



class CTestapltime: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestapltime);
    CPPUNIT_TEST(testTime);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}
    void testTime()
    {

        TEST_JNDU(apl_time, apl_time(), APL_TIME_SEC);
        TEST_JNDU(apl_gettimeofday, apl_gettimeofday(), APL_TIME_NSEC);

        /* TEST_TIME(days, oper)
         * oper< 0;today - days 
         * oper>=0;today + days
         *
        */
        TEST_TIME(2, -1);
        TEST_TIME(1, -1);
        TEST_TIME(0, -1);
        TEST_TIME(1, 1);
        
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestapltime);

int main(int argc, char *argv[])
{
    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    if (runner.run())
       return 0;
    else
       return 1;
}
