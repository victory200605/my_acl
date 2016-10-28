#include "Test.h"
#include "acl/Number.h"

using namespace acl;

class CTestAclNumber:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclNumber);
    CPPUNIT_TEST(testIsEmpty);
    CPPUNIT_TEST(testOptT1);
    CPPUNIT_TEST(testOptT2);
    CPPUNIT_TEST(testOptPlusPlus1);
    CPPUNIT_TEST(testOptPlusPlus2);
    CPPUNIT_TEST(testOptSubtSubt1);
    CPPUNIT_TEST(testOptSubtSubt2);
    CPPUNIT_TEST(testOptAssignment1);
    CPPUNIT_TEST(testOptAssignment2);
    CPPUNIT_TEST(testOptPlusAndAssignment1);
    CPPUNIT_TEST(testOptPlusAndAssignment2);
    CPPUNIT_TEST(testOptSubAndAssignment1);
    CPPUNIT_TEST(testOptSubAndAssignment2);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testIsEmpty(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        TNumber<apl_int_t> loNum1;
        TNumber<apl_int_t> loNum2(5);
        TNumber<apl_int_t> loNum3(loNum2);

        //case
        CPPUNIT_ASSERT_MESSAGE("testIsEmpty.", loNum1.IsEmpty() == true);
        CPPUNIT_ASSERT_MESSAGE("testIsEmpty failed.", loNum2.IsEmpty() == false);
        CPPUNIT_ASSERT_MESSAGE("testIsEmpty failed.", loNum3.IsEmpty() == false);

        //end environment
    }

    void testOptT1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TNumber<apl_int_t> loNum1;
        TNumber<apl_int_t> loNum2(5);
        TNumber<apl_int_t> loNum3(loNum2);

        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptT failed.", (apl_int_t)0, (apl_int_t)loNum1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptT failed.", (apl_int_t)5, (apl_int_t)loNum2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptT failed.", (apl_int_t)5, (apl_int_t)loNum3);

        //end environment
    }
    
    void testOptT2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        const TNumber<apl_int_t> loNum1;
        const TNumber<apl_int_t> loNum2(5);
        const TNumber<apl_int_t> loNum3(loNum2);

        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptT failed.", (apl_int_t)0, (apl_int_t)loNum1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptT failed.", (apl_int_t)5, (apl_int_t)loNum2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptT failed.", (apl_int_t)5, (apl_int_t)loNum3);

        //end environment
    }

    void testOptPlusPlus1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TNumber<apl_int_t, CLock> loNum(5);

        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE("++ () failed.", (apl_int_t)6, (apl_int_t)++loNum);

        //end environment
    }

    void testOptPlusPlus2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TNumber<apl_int_t, CLock> loNum(5);

        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE("++ (int) failed.", (apl_int_t)5, (apl_int_t)loNum++);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("++ (int) failed.", (apl_int_t)6, (apl_int_t)loNum);

        //end environment
    }

    void testOptSubtSubt1(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        TNumber<apl_int_t, CLock> loNum(5);

        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE("-- () failed.", (apl_int_t)4, (apl_int_t)--loNum);

        //end environment
    }

    void testOptSubtSubt2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TNumber<apl_int_t, CLock> loNum(5);

        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE("-- (int) failed.", (apl_int_t)5, (apl_int_t)loNum--);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("-- (int) failed.", (apl_int_t)4, (apl_int_t)loNum);

        //end environment
    }

    void testOptAssignment1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TNumber<apl_int_t> loNum1;
        
        //case
        loNum1 = 10;
        CPPUNIT_ASSERT_MESSAGE("Set value failed.", loNum1.IsEmpty() == false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptAssignment failed.", 
                (apl_int_t)10, (apl_int_t)loNum1);

        //end environment
    }

    void testOptAssignment2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TNumber<apl_int_t> loNum1;
        TNumber<apl_int_t> loNum2;
        loNum1 = 10;

        //case
        loNum2 = loNum1;
        CPPUNIT_ASSERT_MESSAGE("Set value failed.", loNum2.IsEmpty() == false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptAssignment failed.", 
                (apl_int_t)10, (apl_int_t)loNum2);

        //end environment
    }
    
    void testOptPlusAndAssignment1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TNumber<apl_int_t> loNum(0);
        
        //case
        loNum += 10;
        
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptPlusAndAssignment failed.", 
                (apl_int_t)10, (apl_int_t)loNum);
                
        loNum += 10;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptPlusAndAssignment failed.", 
                (apl_int_t)20, (apl_int_t)loNum);

        //end environment
    }
    
    void testOptPlusAndAssignment2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TNumber<apl_int_t> loNum1(0);
        TNumber<apl_int_t> loNum2(10);
        
        //case
        loNum1 += loNum2;
        
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptPlusAndAssignment failed.", 
                (apl_int_t)10, (apl_int_t)loNum1);
                
        loNum1 += loNum2;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptPlusAndAssignment failed.", 
                (apl_int_t)20, (apl_int_t)loNum1);

        //end environment
    }
    
    void testOptSubAndAssignment1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TNumber<apl_int_t> loNum1(0);
        TNumber<apl_int_t> loNum2(10);
        
        loNum1 -= loNum2;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptSubAndAssignment failed.", 
                (apl_int_t)-10, (apl_int_t)loNum1);
                
        //case
        loNum1 -= loNum2;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptSubAndAssignment failed.", 
                (apl_int_t)-20, (apl_int_t)loNum1);

        //end environment
    }
    
    void testOptSubAndAssignment2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TNumber<apl_int_t> loNum(0);
        loNum -= 10;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptSubAndAssignment failed.", 
                (apl_int_t)-10, (apl_int_t)loNum);
                
        //case
        loNum -= 10;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptSubAndAssignment failed.", 
                (apl_int_t)-20, (apl_int_t)loNum);

        //end environment
    }
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclNumber);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

