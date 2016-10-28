#include "Test.h"
#include "acl/Any.h"

using namespace acl;

class CTestAclAny:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclAny);
    CPPUNIT_TEST(testAnyCast);
    CPPUNIT_TEST(testSwap);
    CPPUNIT_TEST(testOptAssignment1);
    CPPUNIT_TEST(testOptAssignment2);
    CPPUNIT_TEST(testIsEmpty);
    CPPUNIT_TEST(testCastTo1);
    CPPUNIT_TEST(testCastTo2);
    CPPUNIT_TEST(testCastTo3);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testAnyCast(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CAny loAny((apl_int_t)5);
        CAny loAny2(loAny);

        //case
        if(AnyCast<apl_int_t>(&loAny) != NULL)
        {
            apl_int_t *lpiN = AnyCast<apl_int_t>(&loAny);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testAnyCast failed.", (apl_int_t)5, *lpiN);
        }

        if(AnyCast<apl_int_t>(&loAny2) != NULL)
        {
            apl_int_t *lpiN = AnyCast<apl_int_t>(&loAny2);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testAnyCast failed.", (apl_int_t)5, *lpiN);
        }

        //end environment
    }

    void testSwap(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CAny loAny1;
        CAny loAny2((apl_int_t)10);
        CAny loAnyStr(std::string("abc"));

        //case
        loAny1.Swap(loAny2);

        if(AnyCast<apl_int_t>(&loAny1) != NULL)
        {
            apl_int_t *lpiN = AnyCast<apl_int_t>(&loAny1);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testSwap failed.", (apl_int_t)10, *lpiN);
        }

        loAny1.Swap(loAnyStr);
        CPPUNIT_ASSERT_MESSAGE("testSwap failed.", 
                apl_strcmp("abc", AnyCast<std::string>(&loAny1)->c_str())==0);

        //end environment
        
    }

    void testOptAssignment1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CAny loAny1;

        //case
        loAny1 = 10;
        if(AnyCast<apl_int_t>(&loAny1) != NULL)
        {   
            apl_int_t *lpiN = AnyCast<apl_int_t>(&loAny1);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptAssignment failed.", (apl_int_t)10, *lpiN);
        }

        CAny loAny2;
        loAny2 = std::string("abc");
        if(AnyCast<std::string>(&loAny2) != NULL)
        {
            CPPUNIT_ASSERT_MESSAGE("testOptAssignment failed.", 
                    apl_strcmp("abc", AnyCast<std::string>(&loAny2)->c_str())==0);
        }

        //end environment
    }

    void testOptAssignment2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CAny loAny1(5);
        CAny loAny2(std::string("xyz"));

        //case
        CAny loAny3 = loAny1;
        if(AnyCast<apl_int_t>(&loAny3) != NULL)
        {   
            apl_int_t *lpiN = AnyCast<apl_int_t>(&loAny3);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptAssignment failed.", (apl_int_t)5, *lpiN);
        }

        CAny loAny4 = loAny2;
        if(AnyCast<std::string>(&loAny4) != NULL)
        {
            CPPUNIT_ASSERT_MESSAGE("testOptAssignment failed.",
                    apl_strcmp("xyz", AnyCast<std::string>(&loAny4)->c_str())==0);
        }   

        //end environment
    }

    void testIsEmpty(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start case environment
        CAny loAny;

        //case
        CPPUNIT_ASSERT_MESSAGE("testIsEmpty failed.", true == loAny.IsEmpty());

        loAny = 10;
        CPPUNIT_ASSERT_MESSAGE("testIsEmpty failed.", false == loAny.IsEmpty());

        //end environment
    }

    void testCastTo1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CAny loAny((apl_int_t)5);
        
        //case
        apl_int_t *lpiTmp = loAny.CastTo<apl_int_t>();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testCastTo failed.", (apl_int_t)5, *lpiTmp);

        //end environment
    }
    
    void testCastTo2(void)
    {
        PRINT_TITLE_2(__func__);
        
        apl_int_t liN = 0;
        float     lfN = 0.0;
        
        //start case environment
        CAny loAny((apl_int_t)5);
        
        //case 1
        ASSERT_MESSAGE(loAny.CastTo<apl_int_t>(liN) );
        ASSERT_MESSAGE(liN == 5);
        
        //case 2
        ASSERT_MESSAGE(!loAny.CastTo<float>(lfN) );

        //end environment
    }
    
    void testCastTo3(void)
    {
        PRINT_TITLE_2(__func__);
        
        apl_int_t  liN  = 0;
        apl_int_t* lpiN = NULL;
        float*     lpfN = NULL;
        
        //start case environment
        CAny loAny( new apl_int_t(5) );
        
        //case 1
        ASSERT_MESSAGE(loAny.CastTo<apl_int_t*>(lpiN) );
        ASSERT_MESSAGE(lpiN != NULL);
        ASSERT_MESSAGE(*lpiN == 5);
        
        //case 2
        ASSERT_MESSAGE(!loAny.CastTo<float*>(lpfN) );
        
        //case 2
        ASSERT_MESSAGE(!loAny.CastTo<apl_int_t>(liN) );
        
        //end environment
    }

};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclAny);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

