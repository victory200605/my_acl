#include "Test.h"
#include "acl/Shlib.h"

using namespace acl;


typedef int (*dlSym)(int);

class CTestAclShlib: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclShlib);
    CPPUNIT_TEST( testLoad );
    CPPUNIT_TEST( testSymbol );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) 
    {
    }
    void tearDown(void) 
    {
    }
    void testLoad(void)
    {
        PRINT_TITLE_2(__func__);
        loadlib("./.libs/libtest_dll.so.0", APL_RTLD_LAZY);
        loadlib("./.libs/libtest_dll.so.0", APL_RTLD_LAZY | APL_RTLD_GLOBAL);
        loadlib("./.libs/libtest_dll.so.0", APL_RTLD_LAZY | APL_RTLD_LOCAL);
        loadlib("./.libs/libtest_dll.so.0", APL_RTLD_NOW);
        loadlib("./.libs/libtest_dll.so.0", APL_RTLD_NOW | APL_RTLD_GLOBAL);
        loadlib("./.libs/libtest_dll.so.0", APL_RTLD_NOW | APL_RTLD_LOCAL);
    }
    void loadlib( char const* apcLibName, apl_int_t aiMode )
    {
        //start environment

        //case
        CShlib lolib;
        apl_int_t lir= -1;
        lir= lolib.Load( apcLibName, aiMode);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("shlib load fail", (apl_int_t)0, lir);
        CPPUNIT_ASSERT_MESSAGE("IsLoad fail", lolib.IsLoad() );
        lolib.Unload();
        CPPUNIT_ASSERT_MESSAGE("IsLoad fail", !lolib.IsLoad() );

        lir= 0;
        lir= lolib.Load( "testLib", aiMode);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("shlib load fail", (apl_int_t)-1, lir);
        //end environment
    }

    void testSymbol(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment

        //case
        CShlib lolib;
        lolib.Load( "./.libs/libtest_dll.so.0" );
        lolib.GetDlError();
        dlSym lpvSym = lolib.GetSymbol<dlSym>( "GetA" );
        if ( lpvSym == APL_NULL )
        {
            if ( lolib.GetDlError() == APL_NULL )
            {
                CPPUNIT_FAIL("GetDlError() fail");
            }
        }
        lpvSym = lolib.GetSymbol<dlSym>( "test_func" );
        if ( lpvSym == APL_NULL )
        {
            CPPUNIT_FAIL("GetSymbol fail");
        }
        if ( lpvSym(-1) < 0 || lpvSym(1) >0 )
        {
            CPPUNIT_FAIL("GetSymbol fail");
        }

        //end environment
    }

};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclShlib);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
