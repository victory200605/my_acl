#include "Test.h"
#include "acl/MemCache.h"
#include "acl/Timestamp.h"

using namespace acl;

struct stReplacePolicy
{
    template<typename T>
    apl_int_t operator () ( T& atOprs )
    {
        return 0;
    }
};

class CTestAclMemCache:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclMemCache);
    CPPUNIT_TEST(testGetSize);
    CPPUNIT_TEST(testGetCapacity);
    CPPUNIT_TEST(testPutGet);
    CPPUNIT_TEST(testBeginEnd);
    CPPUNIT_TEST(testErase);
    CPPUNIT_TEST(testClear);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testGetSize(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TMemCache<std::string, std::string> loMemCache(10);

        loMemCache.Put(std::string("key1"), std::string("value1"));
        loMemCache.Put(std::string("key2"), std::string("value2"));
        loMemCache.Put(std::string("key3"), std::string("value3"));
        loMemCache.Put(std::string("key4"), std::string("value4"));
        loMemCache.Put(std::string("key5"), std::string("value5"));

        //case
        apl_size_t liSize = loMemCache.GetSize();                                    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Put & GetSize failed.", (apl_size_t)5, liSize);

        //end environment
    }

    void testGetCapacity(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TMemCache<std::string, std::string> loMemCache(10);

        //case
        apl_size_t liCapacity = loMemCache.GetCapacity();                               
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetCapacity failed.", (apl_size_t)10, liCapacity);

        //end environment
    }

    void testPutGet(void)
    {
        PRINT_TITLE_2(__func__); 

        //start case environment
        apl_int_t liRet = -1;
        TMemCache<std::string, std::string> loMemCache(10);

        //case
        liRet = loMemCache.Put(std::string("key1"), std::string("value1"));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Put(OPT_NOOVERWRITE) failed.", (apl_int_t)0, liRet);
        liRet = loMemCache.Put(std::string("key1"), std::string("new value1"));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Put(OPT_NOOVERWRITE) failed.", (apl_int_t)-1, liRet);
        liRet = loMemCache.Put(std::string("key1"), std::string("new value1"),
                TMemCache<std::string, std::string>::OPT_OVERWRITE);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Put(OPT_OVERWRITE) failed.", (apl_int_t)0, liRet);

        loMemCache.Put(std::string("key2"), std::string("value2"));
        loMemCache.Put(std::string("key3"), std::string("value3"));
        loMemCache.Put(std::string("key4"), std::string("value4"));
        loMemCache.Put(std::string("key5"), std::string("value5"));

        std::string loValue;
        CTimestamp loTimestamp;
        
        liRet = loMemCache.Get(std::string("key1"), &loValue, &loTimestamp);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get(OPT_NODELETE) failed.", (apl_int_t)0, liRet);
        CPPUNIT_ASSERT_MESSAGE("Get(OPT_NODELETE) failed.", 
                apl_strcmp("new value1", loValue.c_str()) == 0);
        apl_time_t liTime = time(NULL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get(OPT_NODELETE) timestamp failed.", liTime, loTimestamp.Sec());

        loMemCache.Get(std::string("key3"), &loValue, NULL, 
                TMemCache<std::string, std::string>::OPT_DELETE);
        CPPUNIT_ASSERT_MESSAGE("Get failed.", apl_strcmp("value3", loValue.c_str()) == 0);
        apl_size_t liSize = loMemCache.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get(OPT_DELETE) failed.", (apl_size_t)4, liSize);

        //end environment
    }

    void testBeginEnd(void)
    {
        PRINT_TITLE_2(__func__);   

        //start case environment
        TMemCache<std::string, std::string> loMemCache(10);

        loMemCache.Put(std::string("key1"), std::string("value1"));
        loMemCache.Put(std::string("key2"), std::string("value2"));

        //case
        apl_time_t liTime = time(NULL);
        for(TMemCache<std::string, std::string>::IteratorType loIter = loMemCache.Begin();
                loIter != loMemCache.End(); ++loIter)
        {
            CPPUNIT_ASSERT_MESSAGE("iterator failed.", 
                    loIter->Key().c_str()[strlen(loIter->Key().c_str())-1] == 
                    loIter->Value().c_str()[apl_strlen(loIter->Value().c_str())-1]);
            CPPUNIT_ASSERT_MESSAGE("iterator timestamp failed.", liTime == loIter->Timestamp().Sec());
        }

        //end environment
    }

    void testErase(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TMemCache<std::string, std::string> loMemCache(10);

        loMemCache.Put(std::string("key1"), std::string("value1"));
        loMemCache.Put(std::string("key2"), std::string("value2"));

        apl_time_t liTime = time(NULL);
        for(TMemCache<std::string, std::string>::IteratorType loIter = loMemCache.Begin();
                loIter != loMemCache.End(); )
        {
            CPPUNIT_ASSERT_MESSAGE("iterator failed.", 
                    loIter->Key().c_str()[strlen(loIter->Key().c_str())-1] == 
                    loIter->Value().c_str()[apl_strlen(loIter->Value().c_str())-1]);
            CPPUNIT_ASSERT_MESSAGE("iterator timestamp failed.", liTime == loIter->Timestamp().Sec());
            //case
            loIter = loMemCache.Erase(loIter);
        }
        apl_size_t liSize = loMemCache.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Erase failed.", (apl_size_t)0, liSize);

        //end environment
    }

    void testClear(void)
    {
        PRINT_TITLE_2(__func__);  

        //start case environment
        TMemCache<std::string, std::string> loMemCache(10);        
                                                                   
        loMemCache.Put(std::string("key1"), std::string("value1"));
        loMemCache.Put(std::string("key2"), std::string("value2"));

        //case
        apl_size_t liSize = loMemCache.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Put failed.", (apl_size_t)2, liSize);  
        loMemCache.Clear();
        liSize = loMemCache.GetSize();                                       
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Clear failed.", (apl_size_t)0, liSize);

        //end environment
    }

};  

class CTestAclMemCacheReplacePolicy:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclMemCacheReplacePolicy);
    CPPUNIT_TEST(testGetSize);
    CPPUNIT_TEST(testGetCapacity);
    CPPUNIT_TEST(testPutGet);
    CPPUNIT_TEST(testBeginEnd);
    CPPUNIT_TEST(testErase);
    CPPUNIT_TEST(testClear);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testGetSize(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TMemCache<std::string, std::string, stReplacePolicy> loMemCache(10);

        loMemCache.Put(std::string("key1"), std::string("value1"));
        loMemCache.Put(std::string("key2"), std::string("value2"));
        loMemCache.Put(std::string("key3"), std::string("value3"));
        loMemCache.Put(std::string("key4"), std::string("value4"));
        loMemCache.Put(std::string("key5"), std::string("value5"));

        //case
        apl_size_t liSize = loMemCache.GetSize();                                    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Put & GetSize failed.", (apl_size_t)5, liSize);

        //end environment
    }

    void testGetCapacity(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TMemCache<std::string, std::string, stReplacePolicy> loMemCache(10);

        //case
        apl_size_t liCapacity = loMemCache.GetCapacity();                               
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetCapacity failed.", (apl_size_t)10, liCapacity);

        //end environment
    }

    void testPutGet(void)
    {
        PRINT_TITLE_2(__func__); 

        //start case environment
        apl_int_t liRet = -1;
        TMemCache<std::string, std::string, stReplacePolicy> loMemCache(10);

        //case
        liRet = loMemCache.Put(std::string("key1"), std::string("value1"));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Put(OPT_NOOVERWRITE) failed.", (apl_int_t)0, liRet);
        liRet = loMemCache.Put(std::string("key1"), std::string("new value1"));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Put(OPT_NOOVERWRITE) failed.", (apl_int_t)-1, liRet);
        liRet = loMemCache.Put(std::string("key1"), std::string("new value1"),
                TMemCache<std::string, std::string, stReplacePolicy>::OPT_OVERWRITE);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Put(OPT_OVERWRITE) failed.", (apl_int_t)0, liRet);

        loMemCache.Put(std::string("key2"), std::string("value2"));
        loMemCache.Put(std::string("key3"), std::string("value3"));
        loMemCache.Put(std::string("key4"), std::string("value4"));
        loMemCache.Put(std::string("key5"), std::string("value5"));

        std::string loValue;
        CTimestamp loTimestamp;
        
        liRet = loMemCache.Get(std::string("key1"), &loValue, &loTimestamp);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get(OPT_NODELETE) failed.", (apl_int_t)0, liRet);
        CPPUNIT_ASSERT_MESSAGE("Get(OPT_NODELETE) failed.", 
                apl_strcmp("new value1", loValue.c_str()) == 0);
        apl_time_t liTime = time(NULL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get(OPT_NODELETE) timestamp failed.", liTime, loTimestamp.Sec());

        loMemCache.Get(std::string("key3"), &loValue, NULL, 
                TMemCache<std::string, std::string, stReplacePolicy>::OPT_DELETE);
        CPPUNIT_ASSERT_MESSAGE("Get failed.", apl_strcmp("value3", loValue.c_str()) == 0);
        apl_size_t liSize = loMemCache.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get(OPT_DELETE) failed.", (apl_size_t)4, liSize);

        //end environment
    }

    void testBeginEnd(void)
    {
        PRINT_TITLE_2(__func__);   

        //start case environment
        TMemCache<std::string, std::string, stReplacePolicy> loMemCache(10);

        loMemCache.Put(std::string("key1"), std::string("value1"));
        loMemCache.Put(std::string("key2"), std::string("value2"));

        //case
        apl_time_t liTime = time(NULL);
        for(TMemCache<std::string, std::string, stReplacePolicy>::IteratorType loIter = loMemCache.Begin();
                loIter != loMemCache.End(); ++loIter)
        {
            CPPUNIT_ASSERT_MESSAGE("iterator failed.", 
                    loIter->Key().c_str()[strlen(loIter->Key().c_str())-1] == 
                    loIter->Value().c_str()[apl_strlen(loIter->Value().c_str())-1]);
            CPPUNIT_ASSERT_MESSAGE("iterator timestamp failed.", liTime == loIter->Timestamp().Sec());
        }

        //end environment
    }

    void testErase(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        TMemCache<std::string, std::string, stReplacePolicy> loMemCache(10);

        loMemCache.Put(std::string("key1"), std::string("value1"));
        loMemCache.Put(std::string("key2"), std::string("value2"));

        apl_time_t liTime = time(NULL);
        for(TMemCache<std::string, std::string, stReplacePolicy>::IteratorType loIter = loMemCache.Begin();
                loIter != loMemCache.End(); )
        {
            CPPUNIT_ASSERT_MESSAGE("iterator failed.", 
                    loIter->Key().c_str()[strlen(loIter->Key().c_str())-1] == 
                    loIter->Value().c_str()[apl_strlen(loIter->Value().c_str())-1]);
            CPPUNIT_ASSERT_MESSAGE("iterator timestamp failed.", liTime == loIter->Timestamp().Sec());
            //case
            loIter = loMemCache.Erase(loIter);
        }
        apl_size_t liSize = loMemCache.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Erase failed.", (apl_size_t)0, liSize);

        //end environment
    }

    void testClear(void)
    {
        PRINT_TITLE_2(__func__);  

        //start case environment
        TMemCache<std::string, std::string, stReplacePolicy> loMemCache(10);        
                                                                   
        loMemCache.Put(std::string("key1"), std::string("value1"));
        loMemCache.Put(std::string("key2"), std::string("value2"));

        //case
        apl_size_t liSize = loMemCache.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Put failed.", (apl_size_t)2, liSize);  
        loMemCache.Clear();
        liSize = loMemCache.GetSize();                                       
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Clear failed.", (apl_size_t)0, liSize);

        //end environment
    }

};  
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclMemCache);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclMemCacheReplacePolicy);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

