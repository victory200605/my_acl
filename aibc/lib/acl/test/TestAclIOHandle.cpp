#include "Test.h"
#include "acl/IOHandle.h"

using namespace acl;

class CTestAclIOHandle:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclIOHandle);
    CPPUNIT_TEST(testHandle);
    CPPUNIT_TEST(testEnDisable);
    CPPUNIT_TEST(testClose);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testHandle(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CIOHandle loIOHandle;

        //case
        loIOHandle.SetHandle(9);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set handle failed.", (apl_int_t)9, loIOHandle.GetHandle()) ;
        //end environment
        loIOHandle.Close();
    }

    void testEnDisable(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CIOHandle loIOHandle;
        apl_int_t liRet = -1;
        apl_int_t liFlag = -1;
    
        //case
        loIOHandle.SetHandle(0);
        /* test Enable and Disable OPT_NONBLOCK */
        liRet = apl_fcntl_getfl(loIOHandle.GetHandle(), &liFlag);
        if(liFlag & APL_O_NONBLOCK)
        {
            CPPUNIT_FAIL("Get file status flags failed.");
        }
        
        liRet = loIOHandle.Disable(CIOHandle::OPT_NONBLOCK);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Disable failed.", (apl_int_t)0, liRet);
        liRet = apl_fcntl_getfl(loIOHandle.GetHandle(), &liFlag);
        if(liFlag & APL_O_NONBLOCK)
        {
            CPPUNIT_FAIL("Get file status flags failed.");
        }

        liRet = loIOHandle.Enable(CIOHandle::OPT_NONBLOCK);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Enable failed.", (apl_int_t)0, liRet);

        liRet = apl_fcntl_getfl(loIOHandle.GetHandle(), &liFlag);
        if(!(liFlag & APL_O_NONBLOCK))
        {
            CPPUNIT_FAIL("Get file status flags failed.");
        }

        liRet = loIOHandle.Disable(CIOHandle::OPT_NONBLOCK);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Disable failed.", (apl_int_t)0, liRet);
        liRet = apl_fcntl_getfl(loIOHandle.GetHandle(), &liFlag);
        if(liFlag & APL_O_NONBLOCK)
        {
            CPPUNIT_FAIL("Get file status flags failed.");
        }

        /* test Enable and Disable OPT_CLOEXEC */
        liRet = apl_fcntl_getfd(loIOHandle.GetHandle(), &liFlag);
        if(liFlag & APL_FD_CLOEXEC)
        {
            CPPUNIT_FAIL("Get file descriptor failed.");
        }
        
        liRet = loIOHandle.Disable(CIOHandle::OPT_CLOEXEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Disable failed.", (apl_int_t)0, liRet);
        liRet = apl_fcntl_getfd(loIOHandle.GetHandle(), &liFlag);
        if(liFlag & APL_FD_CLOEXEC)
        {
            CPPUNIT_FAIL("Get file descriptor failed.");
        }

        liRet = loIOHandle.Enable(CIOHandle::OPT_CLOEXEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Enable failed.", (apl_int_t)0, liRet);

        liRet = apl_fcntl_getfd(loIOHandle.GetHandle(), &liFlag);
        if(!(liFlag & APL_FD_CLOEXEC))
        {
            CPPUNIT_FAIL("Get file descriptor failed.");
        }

        liRet = loIOHandle.Disable(CIOHandle::OPT_CLOEXEC);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Disable failed.", (apl_int_t)0, liRet);
        liRet = apl_fcntl_getfd(loIOHandle.GetHandle(), &liFlag);
        if(liFlag & APL_FD_CLOEXEC)
        {
            CPPUNIT_FAIL("Get file descriptor failed.");
        }

        //end environment
        loIOHandle.Close();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Close handle failed.", (apl_int_t)-1, (loIOHandle.GetHandle()) );
    }

    void testClose(void)
    {
        PRINT_TITLE_2(__func__); 
        //start case environment
        CIOHandle loIOHandle;
        
        //case
        loIOHandle.SetHandle(7);
        loIOHandle.Close();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Close failed.", (apl_int_t)-1, loIOHandle.GetHandle());
        //end environment
    }

};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclIOHandle);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

