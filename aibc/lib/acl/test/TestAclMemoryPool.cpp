#include "Test.h"
#include "acl/MemoryPool.h"

using namespace acl;

class CTestAclMemoryPool:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclMemoryPool);
    CPPUNIT_TEST(testMemoryPool);
    CPPUNIT_TEST(testGetSize);
    CPPUNIT_TEST(testGetSpace);
    CPPUNIT_TEST(testInitialize);
    CPPUNIT_TEST(testAllocate);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testDeallocate);
    CPPUNIT_TEST(testRelease);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testGetSize(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start case environment
        CMemoryPool loMemoryPool;
        loMemoryPool.Initialize(64);

        loMemoryPool.Allocate(8);

        //case
        apl_size_t liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_MESSAGE("testGetSize failed.", liSize >= 8);

        //end environment
    }

    void testGetSpace(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CMemoryPool loMemoryPool;
        loMemoryPool.Initialize(64);

        //case
        apl_size_t liSpace = loMemoryPool.GetSpace(); 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testGetSpace failed.", (apl_size_t)ACL_DOALIGN(64, 8), liSpace);

        //end environment
    }

    void testInitialize(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CMemoryPool loMemoryPool;

        //case
        loMemoryPool.Initialize(64);
        apl_size_t liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testInitialize failed.", (apl_size_t)0, liSize);
        apl_size_t liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testInitialize failed.", (apl_size_t)ACL_DOALIGN(64, 8), liSpace);

        //end environment
    }

    void testAllocate(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CMemoryPool loMemoryPool;
        apl_size_t liPoolSize = 64;
        apl_size_t liAllocateSize = 8;
        loMemoryPool.Initialize(liPoolSize);
        apl_size_t liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);
        apl_size_t liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)ACL_DOALIGN(liPoolSize, 8), liSpace);

        //case
        apl_int_t* lpiN = (apl_int_t*)loMemoryPool.Allocate(liAllocateSize);
        CPPUNIT_ASSERT_MESSAGE("Allocate failed.", lpiN != NULL);
        liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_MESSAGE("Allocate failed.", liSize >= ACL_DOALIGN(liAllocateSize, 8) );
        liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_MESSAGE("Allocate failed.", (ACL_DOALIGN(liPoolSize,8) - ACL_DOALIGN(liAllocateSize,8)) >= liSpace);

        //end environment
    }

    void testReset(void)
    {
        PRINT_TITLE_2(__func__); 

        //start case environment
        CMemoryPool loMemoryPool;
        apl_size_t liPoolSize = 100;
        apl_size_t liAllocateSize = 10;
        loMemoryPool.Initialize(liPoolSize);
        apl_size_t liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);
        apl_size_t liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)ACL_DOALIGN(liPoolSize, 8), liSpace);

        apl_int_t* lpiN1 = (apl_int_t*)loMemoryPool.Allocate(liAllocateSize);
        if( lpiN1 == NULL)
        {
            CPPUNIT_FAIL("Allocate failed.");
        }

        //case
        loMemoryPool.Reset();
        liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testReset failed.", (apl_size_t)0, liSize);
        liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testReset failed.", (apl_size_t)ACL_DOALIGN(liPoolSize, 8), liSpace);

        apl_int_t* lpiN2 = (apl_int_t*)loMemoryPool.Allocate(liAllocateSize);
        if( lpiN2 == NULL)
        {
            CPPUNIT_FAIL("Allocate failed.");
        }

        //end environment
    }

    void testDeallocate(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        CMemoryPool loMemoryPool;
        apl_size_t liPoolSize = 64;
        loMemoryPool.Initialize(liPoolSize);
        apl_size_t liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);
        apl_size_t liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)ACL_DOALIGN(liPoolSize, 8), liSpace);

        apl_int_t* lpiN1 = (apl_int_t*)loMemoryPool.Allocate(8);
        if( lpiN1 == NULL)
        {
            CPPUNIT_FAIL("Allocate failed.");
        }

        apl_int_t* lpiN2 = (apl_int_t*)loMemoryPool.Allocate(10);
        if( lpiN2 == NULL)
        {
            CPPUNIT_FAIL("Allocate failed.");
        }

        liSize = loMemoryPool.GetSize();                                                         
        CPPUNIT_ASSERT_MESSAGE("Allocate failed.", liSize >= 18);              
        liSpace = loMemoryPool.GetSpace();                                                       
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Allocate failed.", (apl_size_t)ACL_DOALIGN(liPoolSize, 8)-liSize, liSpace);

        //case
        loMemoryPool.Deallocate(lpiN1);

        liSize = loMemoryPool.GetSize();                                                         
        CPPUNIT_ASSERT_MESSAGE("testDeallocate failed.", liSize >= 10 );              
        liSpace = loMemoryPool.GetSpace();                                                       
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testDeallocate failed.", (apl_size_t)ACL_DOALIGN(liPoolSize, 8)-liSize, liSpace);

        //end environment
    }

    void testRelease(void)
    {
        PRINT_TITLE_2(__func__); 

        //start case environment
        CMemoryPool loMemoryPool;
        apl_size_t liPoolSize = 100;
        apl_size_t liAllocateSize = 10;
        loMemoryPool.Initialize(liPoolSize);
        apl_size_t liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);
        apl_size_t liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)ACL_DOALIGN(liPoolSize, 8), liSpace);

        apl_int_t* lpiN = (apl_int_t*)loMemoryPool.Allocate(liAllocateSize);
        if( lpiN == NULL)
        {
            CPPUNIT_FAIL("Allocate failed.");
        }

        //case
        loMemoryPool.Release();
        liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRelease failed.", (apl_size_t)0, liSize);
        liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testRelease failed.", (apl_size_t)0, liSpace);

        //end environment
    }

    void testMemoryPool(void)
    {
        PRINT_TITLE_2(__func__);
        TestMemoryPool(32, 8);
        TestMemoryPool(1024, 992);
        TestMemoryPool(40, 10);
        TestMemoryPool(1024, 1017);
    }

/* ----------------------------------------------------------------- */    

    void TestMemoryPool(apl_size_t aiPoolSize, apl_size_t aiAllocateSize)
    {
        apl_size_t liSize = 0;
        apl_size_t liSpace = 0;

        apl_size_t liPoolSize = ACL_DOALIGN(aiPoolSize, 8);
        apl_size_t liAllocateSize = ACL_DOALIGN(aiAllocateSize, 8);
        CMemoryPool loMemoryPool;

        //Initialize
        loMemoryPool.Initialize(aiPoolSize);
        liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", (apl_size_t)0, liSize);
        liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initialize failed.", liPoolSize, liSpace);

        //Allocate
        apl_int_t* lpiN = (apl_int_t*)loMemoryPool.Allocate(aiAllocateSize);
        if(lpiN == NULL && (liPoolSize - liAllocateSize)>8)
        {
            CPPUNIT_FAIL("Allocate failed.");
        }
        else if(lpiN == NULL && (liPoolSize - liAllocateSize)<8)
        {
            CPPUNIT_ASSERT_MESSAGE("Allocate failed.", lpiN == NULL);
            return;
        }
        liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_MESSAGE("Allocate failed.", liAllocateSize <= liSize);
        liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_MESSAGE("Allocate failed.", liPoolSize-liAllocateSize >= liSpace);

        //Reset
        loMemoryPool.Reset();
        liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Reset failed.", (apl_size_t)0, liSize);
        liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Reset failed.", liPoolSize, liSpace);

        //Allocate again
        lpiN = (apl_int_t*)loMemoryPool.Allocate(aiAllocateSize);
        if(lpiN == NULL && (liPoolSize - liAllocateSize)>8)
        {   
            CPPUNIT_FAIL("Allocate failed.");
        }
        else if(lpiN == NULL && (liPoolSize - liAllocateSize)<8)
        {   
            CPPUNIT_ASSERT_MESSAGE("Allocate failed.", lpiN == NULL);
            return;
        }
        liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_MESSAGE("Allocate failed.", liAllocateSize <= liSize);
        liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_MESSAGE("Allocate failed.", liPoolSize-liSize >= liSpace);

        //Deallocate
        loMemoryPool.Deallocate(lpiN);
        liSize = loMemoryPool.GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Deallocate failed.", (apl_size_t)0, liSize);
        liSpace = loMemoryPool.GetSpace();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Deallocate failed.", liPoolSize, liSpace);
    }
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclMemoryPool);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

