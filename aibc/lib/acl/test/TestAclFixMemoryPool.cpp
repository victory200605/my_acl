#include "Test.h"
#include "acl/FixMemoryPool.h"

using namespace acl;

class CTestAclFixMemoryPool:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclFixMemoryPool);
    CPPUNIT_TEST(testGetSize);
    CPPUNIT_TEST(testGetSpace);
    CPPUNIT_TEST(testInitialize);
    CPPUNIT_TEST(testInitializeDoDel);
    CPPUNIT_TEST(testAllocate);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testDeallocate);
    CPPUNIT_TEST(testRelease);
    CPPUNIT_TEST(testFixMemoryPool);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testGetSize(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_size_t liBlockSize = sizeof(apl_int_t);
        apl_size_t liBlockCount = 10;

        CFixMemoryPool loMemoryPool;                       
        loMemoryPool.Initialize(liBlockSize, liBlockCount);

        apl_int_t *lpiN = (apl_int_t*)loMemoryPool.Allocate(liBlockSize);
        if(lpiN == NULL)
        {
            CPPUNIT_FAIL("Allocate failed.");
        }

        //case
        apl_size_t liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSize failed.", sizeof(apl_int_t), liSize);

        //end environment
    }

    void testGetSpace(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_size_t liBlockSize = sizeof(apl_int_t);
        apl_size_t liBlockCount = 10;

        CFixMemoryPool loMemoryPool;                                                          
        loMemoryPool.Initialize(liBlockSize, liBlockCount);                                   

        //case
        apl_size_t liSpace = loMemoryPool.GetSpace();                  
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSpace failed.", 
                ACL_DOALIGN(liBlockSize, 4)*liBlockCount, liSpace);

        //end environment
    }

    void testInitialize(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_size_t liBlockSize = sizeof(apl_int_t);
        apl_size_t liBlockCount = 10;

        apl_size_t liSize = 0;
        apl_size_t liSpace = 0;

        CFixMemoryPool loMemoryPool;                                                          

        //case
        loMemoryPool.Initialize(liBlockSize, liBlockCount);                                   
        liSize = loMemoryPool.GetSize();                                                      
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);            
        liSpace = loMemoryPool.GetSpace();                                                    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", 
                ACL_DOALIGN(liBlockSize, 4)*liBlockCount, liSpace);

        //end environment
    }

    void testInitializeDoDel(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_size_t liBlockSize = sizeof(apl_int_t);
        apl_size_t liBlockCount = 10;

        apl_size_t liSize = 0;
        apl_size_t liSpace = 0;

        unsigned char* lpcPtr = NULL;
        ACL_NEW_N_ASSERT( lpcPtr, unsigned char, liBlockCount * liBlockSize );

        CFixMemoryPool loMemoryPool;                                                          

        //case
        loMemoryPool.Initialize(lpcPtr, liBlockSize*liBlockCount, liBlockSize, liBlockCount);                                   
        liSize = loMemoryPool.GetSize();                                                      
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);            
        liSpace = loMemoryPool.GetSpace();                                                    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", 
                ACL_DOALIGN(liBlockSize, 4)*liBlockCount, liSpace);

        //end environment
        ACL_DELETE_N(lpcPtr);
    }

    void testAllocate(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start case environment
        apl_size_t liBlockSize = sizeof(apl_int_t);
        apl_size_t liBlockCount = 10;
        apl_size_t liAllocateNum = 7;
        apl_size_t liSize = 0;
        apl_size_t liSpace = 0;
        apl_int_t *lpiN[liAllocateNum];

        CFixMemoryPool loMemoryPool;                                                          
        loMemoryPool.Initialize(liBlockSize, liBlockCount);                                   
        liSize = loMemoryPool.GetSize();                                                      
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);            
        liSpace = loMemoryPool.GetSpace();                                                    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", 
                ACL_DOALIGN(liBlockSize, 4)*liBlockCount, liSpace);

        //case
        for(apl_size_t i=1; i<=liAllocateNum; ++i)
        {
            lpiN[i-1] = (apl_int_t*)loMemoryPool.Allocate(liBlockSize);
            if(lpiN[i-1] == NULL)
            {
                CPPUNIT_FAIL("Allocate failed.");
            }

            liSize = loMemoryPool.GetSize();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Allocate failed.", i*ACL_DOALIGN(liBlockSize, 4), liSize);
            liSpace = loMemoryPool.GetSpace();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Allocate failed.", 
                    ACL_DOALIGN(liBlockSize, 4)*liBlockCount - i*liBlockSize, liSpace);
        }

        //end environment
    }

    void testReset(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_size_t liBlockSize = sizeof(apl_int_t);
        apl_size_t liBlockCount = 10;
        apl_size_t liAllocateNum = 7;
        apl_size_t liSize = 0;
        apl_size_t liSpace = 0;
        apl_int_t *lpiN[liAllocateNum];

        CFixMemoryPool loMemoryPool;                                                          
        loMemoryPool.Initialize(liBlockSize, liBlockCount);                                   
        liSize = loMemoryPool.GetSize();                                                      
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);            
        liSpace = loMemoryPool.GetSpace();                                                    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", 
                ACL_DOALIGN(liBlockSize, 4)*liBlockCount, liSpace);

        for(apl_size_t i=1; i<=liAllocateNum; ++i)
        {
            lpiN[i-1] = (apl_int_t*)loMemoryPool.Allocate(liBlockSize);
            if(lpiN[i-1] == NULL)
            {
                CPPUNIT_FAIL("Allocate failed.");
            }
        }

        //case
        loMemoryPool.Reset();
        liSize = loMemoryPool.GetSize();                                                      
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Reset failed.", (apl_size_t)0, liSize);            
        liSpace = loMemoryPool.GetSpace();                                                    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Reset failed.", 
                ACL_DOALIGN(liBlockSize, 4)*liBlockCount, liSpace);

        //end environment
    }

    void testDeallocate(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_size_t liBlockSize = sizeof(apl_int_t);
        apl_size_t liBlockCount = 10;
        apl_size_t liAllocateNum = 7;
        apl_size_t liSize = 0;
        apl_size_t liSpace = 0;
        apl_int_t *lpiN[liAllocateNum];

        CFixMemoryPool loMemoryPool;                                                          
        loMemoryPool.Initialize(liBlockSize, liBlockCount);                                   
        liSize = loMemoryPool.GetSize();                                                      
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);            
        liSpace = loMemoryPool.GetSpace();                                                    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", 
                ACL_DOALIGN(liBlockSize, 4)*liBlockCount, liSpace);

        for(apl_size_t i=1; i<=liAllocateNum; ++i)
        {
            lpiN[i-1] = (apl_int_t*)loMemoryPool.Allocate(liBlockSize);
            if(lpiN[i-1] == NULL)
            {
                CPPUNIT_FAIL("Allocate failed.");
            }
        }

        apl_size_t liTotalAllocateSize = loMemoryPool.GetSize();
        apl_size_t liTotalSpace = loMemoryPool.GetSpace();

        //case
        for(apl_size_t i=1; i<=liAllocateNum; ++i)
        {
            loMemoryPool.Deallocate((void*)lpiN[i-1]);
            liSize = loMemoryPool.GetSize();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Deallocate failed.", 
                    liTotalAllocateSize - i*ACL_DOALIGN(liBlockSize, 4), liSize);
            liSpace = loMemoryPool.GetSpace();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Deallocate failed.", 
                    liTotalSpace + i*ACL_DOALIGN(liBlockSize, 4), liSpace);
        }

        //end environment
    }

    void testRelease(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_size_t liBlockSize = sizeof(apl_int_t);
        apl_size_t liBlockCount = 10;
        apl_size_t liAllocateNum = 7;
        apl_size_t liSize = 0;
        apl_size_t liSpace = 0;
        apl_int_t *lpiN[liAllocateNum];

        CFixMemoryPool loMemoryPool;                                                          
        loMemoryPool.Initialize(liBlockSize, liBlockCount);                                   

        for(apl_size_t i=1; i<=liAllocateNum; ++i)
        {
            lpiN[i-1] = (apl_int_t*)loMemoryPool.Allocate(liBlockSize);
            if(lpiN[i-1] == NULL)
            {
                CPPUNIT_FAIL("Allocate failed.");
            }
        }

        //case
        loMemoryPool.Release();
        liSize = loMemoryPool.GetSize();                                                      
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Release failed.", (apl_size_t)0, liSize);            
        liSpace = loMemoryPool.GetSpace();                                                    
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Release failed.", (apl_size_t)0, liSpace);

        //end environment
    }

    void testFixMemoryPool(void)
    {
        PRINT_TITLE_2(__func__);
        TestFixMemoryPool(sizeof(apl_int_t), 10, 5);
        TestFixMemoryPool(sizeof(apl_int_t), 1024, 1);
        TestFixMemoryPool(sizeof(apl_int_t), 1024, 1024);
        TestFixMemoryPool(3, 10, 10);
    }

/* ----------------------------------------------------------------- */

    void TestFixMemoryPool(apl_size_t aiBlockSize, apl_int_t aiBlockCount, apl_int_t aiAllocateNum )
    {
        apl_int_t *lpiN[aiAllocateNum];
        apl_size_t liSize = 0;
        apl_size_t liSpace = 0;

        apl_size_t liBlockSize = ACL_DOALIGN(aiBlockSize, 4);

        CFixMemoryPool loMemoryPool;
        loMemoryPool.Initialize(aiBlockSize, aiBlockCount);
        liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);
        liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", liBlockSize*aiBlockCount, liSpace);
    
        //Allocate
        for(apl_int_t i=1; i<=aiAllocateNum; ++i)
        {
            lpiN[i-1] = (apl_int_t*)loMemoryPool.Allocate(aiBlockSize);
            if(lpiN[i-1] == NULL)
            {
                CPPUNIT_FAIL("Allocate failed.");
            }

            liSize = loMemoryPool.GetSize();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSize failed.", i*liBlockSize, liSize);
            liSpace = loMemoryPool.GetSpace();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSpace failed.", liBlockSize*aiBlockCount - i*liBlockSize, liSpace);
        }

        //Reset
        loMemoryPool.Reset();
        liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);
        liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", liBlockSize*aiBlockCount, liSpace);

        //Allocate again
        for(apl_int_t i=1; i<=aiAllocateNum; ++i)
        {   
            lpiN[i] = (apl_int_t*)loMemoryPool.Allocate(aiBlockSize);
            if(lpiN[i] == NULL)
            {   
                CPPUNIT_FAIL("Allocate failed.");
            }
                
            liSize = loMemoryPool.GetSize();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSize failed.", i*liBlockSize, liSize);
            liSpace = loMemoryPool.GetSpace();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSpace failed.", liBlockSize*aiBlockCount - i*liBlockSize, liSpace);
        }

        //Deallocate
        apl_size_t liTotalAllocateSize = liSize;
        apl_size_t liTotalSpace = liSpace;

        for(apl_int_t i=1; i<=aiAllocateNum; ++i)
        {
            loMemoryPool.Deallocate(lpiN[i]);
            liSize = loMemoryPool.GetSize();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Deallocate failed.", liTotalAllocateSize - i*liBlockSize, liSize);
            liSpace = loMemoryPool.GetSpace();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Deallocate failed.", liTotalSpace + i*liBlockSize, liSpace);
        }
    }

};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclFixMemoryPool);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

