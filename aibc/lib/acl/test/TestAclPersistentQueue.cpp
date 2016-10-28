#include "Test.h"
#include "acl/PersistentQueue.h"
#include "acl/Timestamp.h"

using namespace acl;
        struct CNode
        {
            char macBuff[692];
        };

class CTestAclPersistentQueue : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclPersistentQueue);
    CPPUNIT_TEST(testInitialize1);
    CPPUNIT_TEST(testInitialize2);
    CPPUNIT_TEST(testInitialize3);
    CPPUNIT_TEST(testInitialize4);
    CPPUNIT_TEST(testPop1);
    CPPUNIT_TEST(testPop2);
    CPPUNIT_TEST(testPerformance);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) 
    {
    }

    void tearDown(void) 
    {
    }

    void testInitialize1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment 
        acl::CDir::Remove("PersistentQueue");

        //case 1
        {
            acl::TPersistentQueue<apl_int_t> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 100) == 0);

            for (apl_int_t liN = 0; liN < 1000; liN++)
            {
                ASSERT_MESSAGE(loQueue.Push(liN) == 0);
            }
        }

        //case 2
        {
            acl::TPersistentQueue<apl_int_t> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 100) == 0);
            ASSERT_MESSAGE(loQueue.GetSize() == 1000);
        }

        //end environment
    }
    
    void testInitialize2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment 
        acl::CDir::Remove("PersistentQueue");

        //case 1
        {
            acl::TPersistentQueue<apl_int_t> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 100, 50) == 0);

            for (apl_int_t liN = 0; liN < 1000; liN++)
            {
                ASSERT_MESSAGE(loQueue.Push(liN) == 0);
            }
        }

        //case 2
        {
            acl::TPersistentQueue<apl_int_t> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 100, 50) == 0);
            ASSERT_MESSAGE(loQueue.GetSize() == 1000);
        }

        //end environment
    }
    
    void testInitialize3(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment 
        acl::CDir::Remove("PersistentQueue");

        //case 1
        {
            acl::TPersistentQueue<apl_int_t> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 100, 50, ".", 1000) == 0);

            for (apl_int_t liN = 0; liN < 1000; liN++)
            {
                ASSERT_MESSAGE(loQueue.Push(liN) == 0);
            }
            
            ASSERT_MESSAGE(loQueue.Push(1000) != 0);
        }

        //end environment
    }
    
    void testInitialize4(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment 
        acl::CDir::Remove("PersistentQueue");

        //case 1
        {
            acl::TPersistentQueue<apl_int_t> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 100) == 0);

            for (apl_int_t liN = 0; liN < 1000; liN++)
            {
                ASSERT_MESSAGE(loQueue.Push(liN) == 0);
            }
        }

        acl::CFile loFile;

        ASSERT_MESSAGE(loFile.Open("PersistentQueue/1", APL_O_RDWR) == 0);
        apl_size_t luFileSize = loFile.GetFileSize();
        ASSERT_MESSAGE(loFile.Truncate(luFileSize + 2) == 0);
        acl::CFileInfo loFileInfo;
        acl::CFile::GetFileInfo("PersistentQueue/1", loFileInfo);
        ASSERT_MESSAGE(loFileInfo.GetSize() == luFileSize + 2);

        //case 2
        {
            acl::TPersistentQueue<apl_int_t> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 100) == 0);
            ASSERT_MESSAGE(loQueue.GetSize() == 1000);
        }

        //end environment
    }
    
    
    void testPop1(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment 
        acl::CDir::Remove("PersistentQueue");

        //case 1
        {
            acl::TPersistentQueue<apl_int_t> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 100, 50) == 0);

            for (apl_int_t liN = 0; liN < 1000; liN++)
            {
                ASSERT_MESSAGE(loQueue.Push(liN) == 0);
            }
        }

        //case 2
        {
            acl::TPersistentQueue<apl_int_t> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 100, 50) == 0);
            ASSERT_MESSAGE(loQueue.GetSize() == 1000);
            for (apl_int_t liN = 0; liN < 1000; liN++)
            {
                apl_int_t liTemp;
                ASSERT_MESSAGE(loQueue.Front(liTemp) == 0 && liN == liN);
                ASSERT_MESSAGE(loQueue.Pop() == 0);
            }
            
            apl_int_t liTemp;
            ASSERT_MESSAGE(loQueue.Front(liTemp) != 0);
        }

        //end environment
    }

    void testPop2(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment 

        acl::CDir::Remove("PersistentQueue");

        //case 1
        {
            acl::TPersistentQueue<CNode> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 20480, 1000) == 0);
            
            for (apl_int_t liN = 0; liN < 10000; liN++)
            {
                ASSERT_MESSAGE(loQueue.Push(CNode() ) == 0);
                ASSERT_MESSAGE(loQueue.Pop() == 0);
            }
            
            for (apl_int_t liN = 0; liN < 1000; liN++)
            {
                ASSERT_MESSAGE(loQueue.Push(CNode() ) == 0);
            }
            
            ASSERT_MESSAGE(loQueue.GetSize() == 1000);
        }
        
        //case 1
        {
            acl::TPersistentQueue<CNode> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 20480, 1000) == 0);
            ASSERT_MESSAGE(loQueue.GetSize() == 1000);
        }

        //end environment
    }
    void testPerformance(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start case environment 
        acl::CDir::Remove("PersistentQueue");

        //case 1
        {
            acl::TPersistentQueue<apl_int_t> loQueue;
            ASSERT_MESSAGE(loQueue.Initialize("PersistentQueue", 10000, 100) == 0);
            
            START_LOOP(100000)
            {
                assert(loQueue.Push(0) == 0);
            }
            END_LOOP();
            
            apl_int_t liTemp;
            START_LOOP(100000)
            {
                assert(loQueue.Pop(liTemp) == 0);
            }
            END_LOOP();
        }
    }
    
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclPersistentQueue);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

