#include "Test.h"

/* ---------------------------------------------------------------------- */

#define CALC_SUM_1(struct_test)   ( sizeof(struct_test.member1) )

#define CALC_SUM_2(struct_test)   ( sizeof(struct_test.member1) + sizeof(struct_test.member2) )

#define CALC_SUM_3(struct_test)   ( CALC_SUM_2(struct_test) + sizeof(struct_test.member3) )

/* ---------------------------------------------------------------------- */

#define TEST_OFFSET_4(struct_test, srt_test, type1, type2, type3, type4) \
    do { \
        if  ( APL_OFFSETOF(struct_test, member1)!=0 || APL_OFFSETOF(struct_test, member1)%sizeof(type1)!=0) \
        { \
            CPPUNIT_FAIL("offsetof failed at 1:" ); \
        } \
        if  ( APL_OFFSETOF(struct_test, member2)<CALC_SUM_1(srt_test) || APL_OFFSETOF(struct_test, member2)%sizeof(type2)!=0) \
        { \
            CPPUNIT_FAIL("offsetof failed at 2:"); \
        } \
        if  ( APL_OFFSETOF(struct_test, member3)<CALC_SUM_2(srt_test) || APL_OFFSETOF(struct_test, member3)%sizeof(type3)!=0) \
        { \
            CPPUNIT_FAIL("offsetof failed at 3:"); \
        } \
        if  ( APL_OFFSETOF(struct_test, member4)<CALC_SUM_3(srt_test) || APL_OFFSETOF(struct_test, member4)%sizeof(type4)!=0) \
        { \
            CPPUNIT_FAIL("offsetof failed at 4:"); \
        } \
    } while(0)

/* ---------------------------------------------------------------------- */

#define TEST_OFFSET_2(struct_test, srt_test, type1, type2) \
    do { \
        struct struct_test srt_test; \
        if  ( APL_OFFSETOF(struct_test, member1)!=0 || APL_OFFSETOF(struct_test, member1)%sizeof(type1)!=0) \
        { \
            CPPUNIT_FAIL("offsetof failed at 1:" ); \
        } \
        if  ( APL_OFFSETOF(struct_test, member2)<CALC_SUM_1(srt_test) || APL_OFFSETOF(struct_test, member2)%sizeof(type2)!=0) \
        { \
            CPPUNIT_FAIL("offsetof failed at 2:"); \
        } \
    } while(0)

/* ---------------------------------------------------------------------- */

#define TEST_OFFSET_1(struct_test, srt_test, type1) \
    do { \
        if  ( APL_OFFSETOF(struct_test, member1)!=0 || APL_OFFSETOF(struct_test, member1)%sizeof(type1)!=0) \
        { \
            CPPUNIT_FAIL("offsetof failed at 1:" ); \
        } \
    } while(0)

/* ---------------------------------------------------------------------- */


/**
 * @brief Test suite SysNull.
 */
class CTestaplsysNull: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplsysNull);
    CPPUNIT_TEST(testNull);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp() {}

    void tearDown() {}


    /** 
     * Test case SysNull.
     */
    void testNull()
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int8_t li_apl1 = 0;
        apl_int16_t li_apl2 = 0;
        apl_int32_t li_apl3 = 0;
        apl_int64_t li_apl4 = 0;
        apl_int_t li_apl5 = 0;

        //case
        CPPUNIT_ASSERT_MESSAGE("APL_NULL doesn't equal 0", APL_NULL == li_apl1);
        CPPUNIT_ASSERT_MESSAGE("APL_NULL doesn't equal 0", APL_NULL == li_apl2);
        CPPUNIT_ASSERT_MESSAGE("APL_NULL doesn't equal 0", APL_NULL == li_apl3);
        CPPUNIT_ASSERT_MESSAGE("APL_NULL doesn't equal 0", APL_NULL == li_apl4);
        CPPUNIT_ASSERT_MESSAGE("APL_NULL doesn't equal 0", APL_NULL == li_apl5);

        //end environment
    }
};


/* ---------------------------------------------------------------------- */

/**
 * @brief Test suite SysOffset.
 */
class CTestaplsysOffset: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplsysOffset);
    CPPUNIT_TEST(testOffset);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp() {}
    void tearDown() {}

    /** 
     * Test case SysOffset.
     */
    void testOffset()
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        struct struct_test1
        {
            char member1;
            int  member2;
            long member3;
            short member4;
        };

        struct struct_test2
        {
            char member1;
            char member2;
            int  member3;
            long member4;
        };
        struct struct_test3
        {
            int member1;
            char member2;
            char member3[9];
            short member4;
        };
        struct struct_test4
        {
            int  member1;
            char member2;
        };
        struct struct_test5
        {
            char member1;
        };

        struct struct_test1 test_srt_1;
        struct struct_test2 test_srt_2;
        struct struct_test3 test_srt_3; 
        struct struct_test4 test_srt_4; 
        struct struct_test5 test_srt_5; 

        //case
        TEST_OFFSET_4(struct_test1, test_srt_1, char, int, long, short);
        TEST_OFFSET_4(struct_test2, test_srt_2, char, char, int, long);
        TEST_OFFSET_4(struct_test3, test_srt_3, int, char, char, short);
        TEST_OFFSET_2(struct_test4, test_srt_4, int, char);
        TEST_OFFSET_1(struct_test5, test_srt_5, char);

        //end environment
    }

};

/* ---------------------------------------------------------------------- */

CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplsysNull);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplsysOffset);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

