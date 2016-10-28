#include "Test.h"
#include "acl/SockAddr.h"

using namespace acl;

#define SERVPORT 10001


/* ----------------------------------------------------------------- */

class CTestAclSockAddr:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSockAddr);
    CPPUNIT_TEST(testSetAny);
    CPPUNIT_TEST(testIsAny);
    CPPUNIT_TEST(testSetLoopBack);
    CPPUNIT_TEST(testIsLoopBack);
    CPPUNIT_TEST(testSetPort);
    CPPUNIT_TEST(testSetObj);
    CPPUNIT_TEST(testCopyOperator);
    CPPUNIT_TEST(testCompareOperator);
    CPPUNIT_TEST(testSetAddr);
    CPPUNIT_TEST(testGetLength);
    CPPUNIT_TEST(testGetCapacity);
    CPPUNIT_TEST(testGetAddr);
    CPPUNIT_TEST(testGetAddrConst);
    CPPUNIT_TEST(testSetHostSvcName);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testSetAny(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CSockAddr loSockAddr(SERVPORT);

        //case
        apl_int_t liRet = loSockAddr.SetAny();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set loopback failed.", (apl_int_t)0, liRet);
        char lsIpAddr[16];
        loSockAddr.GetIpAddr(lsIpAddr, sizeof(lsIpAddr));
        CPPUNIT_ASSERT_MESSAGE("Isn't any addr", apl_strcmp(lsIpAddr, "0.0.0.0")==0);
        
        //end environment
    }
    void testIsAny(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CSockAddr loSockAddr(SERVPORT);

        //case
        loSockAddr.SetAny();
        CPPUNIT_ASSERT_MESSAGE("Check any addr failed", true == loSockAddr.IsAny());

        //end environment
    }

    /* ------------------------------------------------------------- */

    void testSetLoopBack(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CSockAddr loSockAddr(SERVPORT);

        //case
        apl_int_t liRet = loSockAddr.SetLoopBack();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set loopback failed.", 0, (int)liRet);
        char lsIpAddr[16];
        loSockAddr.GetIpAddr(lsIpAddr, sizeof(lsIpAddr));
        CPPUNIT_ASSERT_MESSAGE("Isn't loopback addr.", apl_strcmp(lsIpAddr, "127.0.0.1")==0 );

        //end environment
    }

    void testIsLoopBack(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CSockAddr loSockAddr(SERVPORT);

        //case
        loSockAddr.SetLoopBack();
        CPPUNIT_ASSERT_MESSAGE("Check loopback failed.", true == loSockAddr.IsLoopBack());

        //end environment
    }

    /* ------------------------------------------------------------- */

    void testSetPort(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment

        //case
        CSockAddr loSockAddrPort(SERVPORT, "4.3.2.1");                               
        TestSameAddr(loSockAddrPort, (apl_uint16_t)SERVPORT, "4.3.2.1", APL_AF_INET);

        //end environment
    }

    void testSetObj(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        //case
        CSockAddr loSockAddrPort(SERVPORT, "4.3.2.1");
        CSockAddr loSockAddrObj;
        loSockAddrObj.Set(loSockAddrPort);
        TestSameAddr(loSockAddrObj, (apl_uint16_t)SERVPORT, "4.3.2.1", APL_AF_INET);

        //end environment
    }
    
    void testCopyOperator(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        //case
        CSockAddr loSockAddr1(SERVPORT, "4.3.2.1");
        CSockAddr loSockAddr2;
        loSockAddr2 = loSockAddr1;
        TestSameAddr(loSockAddr2, (apl_uint16_t)SERVPORT, "4.3.2.1", APL_AF_INET);

        //end environment
    }

    void testCompareOperator(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        //case
        CSockAddr loSockAddr1(SERVPORT, "4.3.2.1");
        CSockAddr loSockAddr2;
        CSockAddr loSockAddr3(SERVPORT, "5.3.2.1");
        loSockAddr2 = loSockAddr1;
        
        ASSERT_MESSAGE(loSockAddr1 == loSockAddr2);
        
        ASSERT_MESSAGE(loSockAddr2 != loSockAddr3);

        //end environment
    }

    void testSetAddr(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        const char *lpcAddr = "1.2.3.4";
        apl_uint16_t liPort = SERVPORT;
        struct apl_sockaddr_storage_t loSas;
        struct apl_sockaddr_in_t* loSai= ( struct apl_sockaddr_in_t*)&loSas;
        apl_sockaddr_setfamily( (apl_sockaddr_t*)loSai,APL_AF_INET);
        apl_sockaddr_setaddr( (apl_sockaddr_t*)loSai, sizeof(*loSai), lpcAddr);
        apl_sockaddr_setport( (apl_sockaddr_t*)loSai, sizeof(*loSai), liPort);

        CSockAddr loSockAddr((apl_sockaddr_t*)loSai, sizeof(*loSai));
        
        //case                  
        TestSameAddr(loSockAddr, (apl_uint16_t)SERVPORT, "1.2.3.4", APL_AF_INET);

        //end environment
    }

    void TestSameAddr(CSockAddr& aoAddr, apl_uint16_t aiPort, char const* apcIpAddr, apl_int_t aiFamily)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set failed.", aiFamily, aoAddr.GetFamily() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set failed.", aiPort, aoAddr.GetPort());
        char lsIpAddr[16];
        memset(lsIpAddr, 0, sizeof(lsIpAddr));
        aoAddr.GetIpAddr(lsIpAddr, sizeof(lsIpAddr));
        CPPUNIT_ASSERT_MESSAGE("Set failed.", apl_strcmp(lsIpAddr, apcIpAddr)==0);
    }

    /* ------------------------------------------------------------------- */

    void testSetAndGet(void)
    {
       PRINT_TITLE_2(__func__);
       //start case environment
       CSockAddr loSockAddr(SERVPORT);
       //case
       loSockAddr.SetIpAddr("1.1.1.1");
       loSockAddr.SetPort(1111);
       loSockAddr.SetFamily(APL_AF_INET);

       CPPUNIT_ASSERT_EQUAL_MESSAGE("Get family name failed.",
               (apl_int_t)APL_AF_INET, loSockAddr.GetFamily());
       CPPUNIT_ASSERT_EQUAL_MESSAGE("Get port failed.", 
               (apl_uint16_t)1111, (apl_uint16_t)loSockAddr.GetPort());
       char lsIpAddr[16];
       memset(lsIpAddr, 0x0, sizeof(lsIpAddr));
       loSockAddr.GetIpAddr(lsIpAddr, sizeof(lsIpAddr));
       CPPUNIT_ASSERT_MESSAGE("GetIpAddr failed.", apl_strcmp(lsIpAddr, "1.1.1.1")==0);
       //end environment
    }

    void testGetLength(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        const char *lpcAddr = "1.2.3.4";                                             
        apl_uint16_t liPort = SERVPORT;                                        
        CSockAddr loSockAddr(liPort, lpcAddr, APL_AF_INET);

        //case
        struct apl_sockaddr_storage_t loSas;                                   
        struct apl_sockaddr_in_t* loSai= ( struct apl_sockaddr_in_t*)&loSas;   
        apl_sockaddr_setfamily( (apl_sockaddr_t*)loSai,APL_AF_INET);           
        apl_sockaddr_setaddr( (apl_sockaddr_t*)loSai, sizeof(*loSai), lpcAddr);
        apl_sockaddr_setport( (apl_sockaddr_t*)loSai, sizeof(*loSai), liPort); 

        apl_size_t liSize = apl_sockaddr_getlen((apl_sockaddr_t*)loSai);

        CPPUNIT_ASSERT_EQUAL_MESSAGE ("Get Length failed.", liSize,
                loSockAddr.GetLength());

        //end environment
    }

    void testGetCapacity(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CSockAddr loSockAddr(SERVPORT);

        //case
        struct apl_sockaddr_storage_t loSas;
        apl_size_t liSize = sizeof(loSas);
        CPPUNIT_ASSERT_EQUAL_MESSAGE ("Get Max Length failed.", liSize,
                loSockAddr.GetCapacity());

        //end environment
    }

    void testGetAddr(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CSockAddr loSockAddr(SERVPORT, "1.2.3.4");

        //case
        char lsIpAddr[16];
        memset(lsIpAddr, 0x0, sizeof(lsIpAddr));

        apl_sockaddr_t *loSai = loSockAddr.GetAddr();
        apl_sockaddr_getaddr(loSai, lsIpAddr, sizeof(lsIpAddr));
        CPPUNIT_ASSERT_MESSAGE("GetAddr failed.", apl_strcmp(lsIpAddr, "1.2.3.4")==0);

        //end environment
    }

    void testGetAddrConst(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment 
        CSockAddr loSockAddr(SERVPORT, "1.2.3.4");

        //case
        char lsIpAddr[16];
        memset(lsIpAddr, 0x0, sizeof(lsIpAddr));

        apl_sockaddr_t const* loSai = loSockAddr.GetAddr();
        apl_sockaddr_getaddr(loSai, lsIpAddr, sizeof(lsIpAddr));
        CPPUNIT_ASSERT_MESSAGE("GetAddr failed.", apl_strcmp(lsIpAddr, "1.2.3.4")==0);  

        //end environment
    }

    /* ------------------------------------------------------------------- */

    void testSetHostSvcName(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CSockAddr loSockAddr(SERVPORT);
        //case
        apl_int_t liRet = loSockAddr.Set("localhost", "ftp");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set hostname failed.", (apl_int_t)0, liRet);
        char lsIpAddr[16];
        loSockAddr.GetIpAddr(lsIpAddr, sizeof(lsIpAddr));
        CPPUNIT_ASSERT_MESSAGE("testSetHostName failed.", apl_strcmp(lsIpAddr, "127.0.0.1")==0 );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("testSetSvcName failed.", 
                (apl_uint16_t)21, loSockAddr.GetPort());
        //end environment
    }

    /* ------------------------------------------------------------------- */
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSockAddr);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

