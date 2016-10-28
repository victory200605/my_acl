#include "Test.h"

/**
 * @brief Test suite dlopen, dlclose, dlsym and dlerror.
 */
class CTestapldlfcn : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestapldlfcn);
    CPPUNIT_TEST(testDlfcn);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void)
    {
    }

    void tearDown(void)
    {
    }

    /**
     * Test case testDlfcn. Load the test_func with different flag.
     */
    void testDlfcn(void) 
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        dlfcn("./.libs/libtest_dll.so.0", APL_RTLD_LAZY, "test_func");
        dlfcn("./.libs/libtest_dll.so.0", APL_RTLD_LAZY | APL_RTLD_GLOBAL, "test_func");
        dlfcn("./.libs/libtest_dll.so.0", APL_RTLD_LAZY | APL_RTLD_LOCAL, "test_func");
        dlfcn("./.libs/libtest_dll.so.0", APL_RTLD_NOW, "test_func");
        dlfcn("./.libs/libtest_dll.so.0", APL_RTLD_NOW | APL_RTLD_GLOBAL, "test_func");
        dlfcn("./.libs/libtest_dll.so.0", APL_RTLD_NOW | APL_RTLD_LOCAL , "test_func");

        //end environment
    }

    void dlfcn(const char* apc_dlname, apl_int_t ai_flag, const char* apc_sym)
    {
        if ( apc_dlname == NULL )
        {
            return ;
        }
        apl_dlerror(); 
        void* lpv_dl = apl_dlopen(apc_dlname, ai_flag);
        if ( lpv_dl == NULL )
        {
            CPPUNIT_FAIL("apl_dlopen failed:");
            return ;
        }
        if ( apc_sym != NULL )
        {
            typedef int (*dl_sym_t)(int);
            dl_sym_t lpv_sym = (dl_sym_t)apl_dlsym(lpv_dl, apc_sym);
            if ( apl_dlerror() != NULL )
            {
                CPPUNIT_FAIL("apl_dlsym failed: ");
            }
            if ( lpv_sym(-1) < 0 )
            {
                printf("%d",lpv_sym(-1) );
                CPPUNIT_FAIL("dlsym fail");
            }
            if ( lpv_sym(1) > 0 )
            {
                CPPUNIT_FAIL("dlsym fail");
            }
        }
        apl_dlclose( lpv_dl );
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestapldlfcn);
int main()
{
    RUN_ALL_TEST(__FILE__);
}
