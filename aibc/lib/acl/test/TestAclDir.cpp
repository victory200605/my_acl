#include "Test.h"
#include "acl/Dir.h"

using namespace acl;


class CTestAclDir: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclDir);
    CPPUNIT_TEST( testOperDir );
    CPPUNIT_TEST( testMkDir );
    CPPUNIT_TEST( testRmDir );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void)
    {
        if ( coDir.Open( "." ) != 0 )
        {
            CPPUNIT_FAIL("open dir fail");
        }
        cpcDirPath= "testDir";
    }
    void tearDown(void) 
    {
        coDir.Close();
    }
    void testOperDir(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        apl_int_t liIi =0;
        apl_int_t liIi1=0;
        apl_int_t liIj =0;
        apl_int_t liIj1=0;

        //case
        while( true )
        {
            if ( coDir.Read() != 0 )
            {
                break;
            }
            if ( !coDir.IsDots() && coDir.IsDir() )
            {
//                printf("\nDir  absname:%s", coDir.GetName() );
//                printf("\nDir fullname:%s", coDir.GetFullName() );
                ++liIi;
            }
            if ( !coDir.IsDots() && coDir.IsFile() )
            {
//                printf("\nFile  absname:%s", coDir.GetName() );
//                printf("\nFile fullname:%s", coDir.GetFullName() );
                ++liIj;
            }
        }
        coDir.Rewind();
        while ( true )
        {
            if ( coDir.Read() != 0 )
            {
                break;
            }
            if ( !coDir.IsDots() && coDir.IsDir() )
            {
                ++liIi1;
            }
            if ( !coDir.IsDots() && coDir.IsFile() )
            {
                ++liIj1;
            }
        }
        CPPUNIT_ASSERT_MESSAGE("dir rewind fail", liIi == liIi1 );
        CPPUNIT_ASSERT_MESSAGE("dir rewind fail", liIj == liIj1 );
 //       printf("\nDir:%s has files:%d, subdirs:%d\n", coDir.GetPath(), (int)liIj, (int)liIi );
        coDir.Seek( coDir.Tell() );

        //end environment
    }

    void testMkDir(void)
    {
        PRINT_TITLE_2(__func__);
        //set environment
        
        //case
        CDir::Remove( cpcDirPath );
        if ( CDir::Make( cpcDirPath ) != 0 )
        {
            CPPUNIT_FAIL("dir mkdir fail");
        }

        //end environment
    }
    void testRmDir(void)
    {
        PRINT_TITLE_2(__func__);
        //set environment
        
        //case
        if ( CDir::Remove( cpcDirPath ) != 0 ) 
        {
            CPPUNIT_FAIL("dir remove fail");
        }

        //end environment
    }
private:
    char const* cpcDirPath;
    CDir coDir;
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclDir);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
