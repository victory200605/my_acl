#include "Test.h"
#include "acl/GetOpt.h"

using namespace acl;

const char *gpacStrL[16]={ "test", "-b123", "-drtv", "-e", "--ms=564", "-h",
                           "--config=asia", "--log=123", "-w456", "-z789" };

const char *gpacStr[16]={ "test", "-b123", "-drtv", "-e", "-ms564", "-h",
                          "-configasia", "-log123", "-w456", "-y789" };

char gacOpt[]= "ab:c:e:m:uy:";
char gacOptL[]= "eh:f:z:";

class CTestAclGetOpt: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclGetOpt);
    CPPUNIT_TEST( testOptString );
    CPPUNIT_TEST( testArgc );
    CPPUNIT_TEST( testArgv );
    CPPUNIT_TEST( testGetOpt );
    CPPUNIT_TEST( testGetOptLong );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void)
    {
    }
    void tearDown(void) {}
    void testOptString(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CGetOpt   loGetOpt1( 10, (char**)gpacStr, gacOpt );
        CGetOpt   loGetOpt2( 10, (char**)gpacStrL, gacOptL );
        apl_int_t lir= 0;
        //case
        const char *lpcChr1= loGetOpt1.OptString();
        lir = apl_memcmp(lpcChr1, gacOpt, apl_strlen(gacOpt));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("OptString fail", (apl_int_t)0, lir);

        const char *lpcChr2= loGetOpt2.OptString();
        lir = apl_memcmp(lpcChr2, gacOptL, apl_strlen(gacOptL));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("OptString fail", (apl_int_t)0, lir);

        //end environment
    }
    void testArgc(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CGetOpt   loGetOpt1( 10, (char**)gpacStr, gacOpt );
        CGetOpt   loGetOpt2( 10, (char**)gpacStrL, gacOptL );
        apl_int_t liN= 0;
        //case
        liN= loGetOpt1.Argc();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Argc fail", (apl_int_t)10, liN);
        liN= loGetOpt2.Argc();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Argc fail", (apl_int_t)10, liN);
        
        //end environment
    }
    void testArgv(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment
        CGetOpt   loGetOpt1( 10, (char**)gpacStr, gacOpt );
        CGetOpt   loGetOpt2( 10, (char**)gpacStrL, gacOptL );
        apl_int_t lir= 0;
        apl_int_t liN= 0;
        apl_int_t liM= 0;
        char **lppcChr1= loGetOpt1.Argv();
        char **lppcAgrv1= (char**)gpacStr;
        char **lppcChr2= loGetOpt2.Argv();
        char **lppcAgrv2= (char**)gpacStrL;
        //case
        liN= loGetOpt1.Argc();
        for(liM=0; liM<liN; ++liM)
        {
             lir= apl_memcmp(*lppcAgrv1, *lppcChr1, apl_strlen(*lppcAgrv1) );
             CPPUNIT_ASSERT_EQUAL_MESSAGE("Argv fail", (apl_int_t)0, lir);
             ++lppcAgrv1;
             ++lppcChr1;
        }
 
        liN= loGetOpt2.Argc();
        for(liM=0; liM<liN; ++liM)
        {
             lir= apl_memcmp(*lppcAgrv2, *lppcChr2, apl_strlen(*lppcAgrv2) );
             CPPUNIT_ASSERT_EQUAL_MESSAGE("Argv fail", (apl_int_t)0, lir);
             ++lppcAgrv2;
             ++lppcChr2;
        }

        //end environment
    }
    void testGetOpt(void)
    {
        PRINT_TITLE_2(__func__);
        getOpt(10, (char**)gpacStr, gacOpt);
        getOpt(10, (char**)gpacStr, gacOpt, 1);
    }
    void testGetOptLong(void)
    {
        PRINT_TITLE_2(__func__);
        getOptLong(10, (char**)gpacStrL, gacOptL);
        getOptLong(10, (char**)gpacStrL, gacOptL, 1);
    }
    void getOptLong(apl_int_t aiAgrc, char* apcAgrv[], char* apcOptString, apl_int_t aiSkipArg = 0)
    {
        //start environment
        CGetOpt   loGetOpt( aiAgrc, apcAgrv, apcOptString );
        apl_int_t liOpt;
        apl_int_t lir= 0;

        //case
        loGetOpt.LongOption( "config", 'f', CGetOpt::ARG_REQUIRED ); //LongOption: config
        loGetOpt.LongOption( "ms", 'k', CGetOpt::ARG_REQUIRED );     //LongOption: ms
        loGetOpt.LongOption( "log", CGetOpt::ARG_REQUIRED );         //LongOption: log

        while( (liOpt= loGetOpt()) != -1 ) //test: operator() OptArg OptInd OptOpt 
        {
            switch( liOpt )
            {
                case 0:
                {
                    lir= apl_memcmp(loGetOpt.LongOption(), "log", 3);
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("LongOption fail", (apl_int_t)0, lir);
                    lir= apl_memcmp(loGetOpt.OptArg(), "123", apl_strlen(loGetOpt.OptArg()) );
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("OptArg fail", (apl_int_t)0, lir);
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case 'w':
                {
                    lir= apl_memcmp(loGetOpt.OptArg(), "456", apl_strlen(loGetOpt.OptArg()) );
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("OptArg fail", (apl_int_t)0, lir);
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case 'f':
                {
                    lir= apl_memcmp(loGetOpt.LongOption(), "config", 6);
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("LongOption fail", (apl_int_t)0, lir);
                    lir= apl_memcmp(loGetOpt.OptArg(), "asia", apl_strlen(loGetOpt.OptArg()) );
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("OptArg fail", (apl_int_t)0, lir);
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case 'b':
                {
                    lir= apl_memcmp(loGetOpt.OptArg(), "123", apl_strlen(loGetOpt.OptArg()) );
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("OptArg fail", (apl_int_t)0, lir);
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case 'k':
                {
                    lir= apl_memcmp(loGetOpt.LongOption(), "ms", 2);
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("LongOption fail", (apl_int_t)0, lir);
                    lir= apl_memcmp(loGetOpt.OptArg(), "564", apl_strlen(loGetOpt.OptArg()) );
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("OptArg fail", (apl_int_t)0, lir);
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case ':':
                {
                    lir= loGetOpt.OptOpt();
                    if ( lir != 'h' )
                    {
                        CPPUNIT_FAIL("OptOpt fail");
                    }
                    break;
                }
                case '?':
                {
                    break;
                }
                default: break;
            }
        }
        lir= apl_memcmp( loGetOpt.LastOption(), "z", apl_strlen(loGetOpt.LastOption() ) ); //LastOption
        CPPUNIT_ASSERT_EQUAL_MESSAGE("LastOption fail", (apl_int_t)0, lir);

        //end environment
    }
    void getOpt(apl_int_t aiAgrc, char* apcAgrv[], char* apcOptString, apl_int_t aiSkipArg = 0)
    {
        //start environment
        CGetOpt   loGetOpt( aiAgrc, apcAgrv, apcOptString );
        apl_int_t liOpt;
        apl_int_t lir= 0;

        //case
        while( (liOpt= loGetOpt()) != -1 ) //test  operator() OptArg OptInd OptOpt
        {
            switch( liOpt )
            {
                case 'l':
                {
                    lir= apl_memcmp(loGetOpt.OptArg(), "123", apl_strlen(loGetOpt.OptArg()) );
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("OptArg fail", (apl_int_t)0, lir);
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case 'w':
                {
                    lir= apl_memcmp(loGetOpt.OptArg(), "456", apl_strlen(loGetOpt.OptArg()) );
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("getopt fail", (apl_int_t)0, lir);
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case 'a':
                {
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case 'b':
                {
                    lir= apl_memcmp(loGetOpt.OptArg(), "123", apl_strlen(loGetOpt.OptArg()) );
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("OptArg fail", (apl_int_t)0, lir);
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case 'c':
                {
                    lir= apl_memcmp(loGetOpt.OptArg(), "onfigasia", apl_strlen(loGetOpt.OptArg()) );
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("OptArg fail", (apl_int_t)0, lir); 
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case 'd':
                {
                    lir= apl_memcmp(loGetOpt.OptArg(), "rtv", apl_strlen(loGetOpt.OptArg()) );
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("OptArg fail", (apl_int_t)0, lir); 
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case 'm':
                {
                    lir= apl_memcmp(loGetOpt.OptArg(), "s564", apl_strlen(loGetOpt.OptArg()) );
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("OptArg fail", (apl_int_t)0, lir); 
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                case ':':
                {
                    lir= loGetOpt.OptOpt();
                    if ( lir != 'e' )
                    {
                        CPPUNIT_FAIL("OptOpt fail");
                    }
                    break;
                }
                case '?':
                {
                    lir= loGetOpt.OptInd();
                    CPPUNIT_ASSERT_MESSAGE("OptInd fail", lir>0 );
                    break;
                }
                default:break; 
            }
        }
        lir= apl_memcmp( loGetOpt.LastOption(), "y", 1); //test LastOption 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("LastOption fail", (apl_int_t)0, lir);

        //end environment
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclGetOpt);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
