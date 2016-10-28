#include "Test.h"



#define TEST_LIMITS(apllmt) \
    do{ \
        apl_intmax_t al = apllmt; \
        if ( al <= 0 ) \
        { \
            CPPUNIT_FAIL(#apllmt"< 0"); \
        } \
    } while(0) 

#define TEST_TYPES(apltyp, typ) \
    { \
        CPPUNIT_ASSERT_EQUAL_MESSAGE(#typ" failed", sizeof(apltyp), sizeof(typ)); \
    } while(0)


/**
 * @brief Test suite Limit and Types.
 * @brief Including two test cases: Limit and Types.
 */
class CTestapllmtype: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestapllmtype);
    CPPUNIT_TEST(testLmt);
    CPPUNIT_TEST(testTypes);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp() {}
    void tearDown() {}

    /** 
     * Test case Limit.
     */
    void testLmt()
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        TEST_LIMITS(APL_PATH_MAX);
        TEST_LIMITS(APL_HOST_NAME_MAX);
        TEST_LIMITS(APL_IOV_MAX);
        TEST_LIMITS(APL_PAGE_SIZE);
        TEST_LIMITS(APL_SEM_VALUE_MAX);
        TEST_LIMITS(APL_THREAD_STACK_MIN);
        TEST_LIMITS(APL_OPEN_MAX);
        TEST_LIMITS(APL_ARG_MAX);
        TEST_LIMITS(APL_LINE_MAX);
        TEST_LIMITS(APL_CHILD_MAX);

        //end environment
    }

    /** 
     * Test case Types.
     */
    void testTypes()
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        TEST_TYPES(apl_uint_t, apl_size_t);
        TEST_TYPES(apl_int_t , apl_ssize_t);
        TEST_TYPES(ptrdiff_t , apl_ptrdiff_t);
        TEST_TYPES(apl_int64_t, apl_time_t);
        TEST_TYPES(apl_int64_t, apl_off_t);

        //end environment
    }

};



CPPUNIT_TEST_SUITE_REGISTRATION(CTestapllmtype);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

