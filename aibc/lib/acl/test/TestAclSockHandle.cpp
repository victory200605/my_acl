#include "Test.h"
#include "acl/SockHandle.h"
#include "acl/SockAcceptor.h"

using namespace acl;

#define SERVPORT 31234
#define CLIPORT  31235

void TestSameAddr(CSockAddr& aoAddr, apl_uint16_t aiPort, 
        char const* apcIpAddr, apl_int_t aiFamily)
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GetFamily failed.", aiFamily, aoAddr.GetFamily() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GetPort failed.", aiPort, aoAddr.GetPort());
    char lsIpAddr[16];
    memset(lsIpAddr, 0, sizeof(lsIpAddr));
    aoAddr.GetIpAddr(lsIpAddr, sizeof(lsIpAddr));
    CPPUNIT_ASSERT_MESSAGE("GetIpAddr failed.", apl_strcmp(lsIpAddr, apcIpAddr)==0);
}   

/* ------------------------------------------------------------------ */

class CTestAclSockHandle:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSockHandle);
    CPPUNIT_TEST(testOpen);
    CPPUNIT_TEST(testGetLocalAddr);
    CPPUNIT_TEST(testGetRemoteAddr);
    CPPUNIT_TEST(testOptionLinger);
    CPPUNIT_TEST(testIGOOption);
    CPPUNIT_TEST(testIOption);
    //CPPUNIT_TEST(testIOptionRcvSnd);
    CPPUNIT_TEST_SUITE_END();

private:
    apl_handle_t ciTcp;
    apl_handle_t ciUdp;
    CSockHandle coSockHandleTcp;
    CSockHandle coSockHandleUdp;

public:
    void setUp(void) 
    {
        ciTcp = coSockHandleTcp.Open(APL_AF_INET,SOCK_STREAM,0);
        ciUdp = coSockHandleUdp.Open(APL_AF_INET,SOCK_DGRAM,0);
    }
    
    void tearDown(void) 
    {
        coSockHandleTcp.Close();
        coSockHandleUdp.Close();
    }

    void testOpen(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        
        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open socket handle failed.", (apl_handle_t)0, ciTcp);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open socket handle failed.", (apl_handle_t)0, ciUdp);

        //end environment
    }

    /* ------------------------------------------------------------------ */

    void testGetLocalAddr(void)
    {
        PRINT_TITLE_2(__func__); 
        
        //start case environment
        apl_int_t liRet = -1;
        CSockAddr loLocalAddr(CLIPORT, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;

        //case
        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Create socket failed.", (apl_int_t)0, liRet);
        liRet = apl_bind(loSockStream.GetHandle(), loLocalAddr.GetAddr(), 
                loLocalAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Bind address failed.", (apl_int_t)0, liRet);

        CSockAddr loGetLocalAddr;
        loSockStream.GetLocalAddr(loGetLocalAddr);

        TestSameAddr(loGetLocalAddr, CLIPORT, "127.0.0.1", APL_AF_INET);

        //end environment
        loSockStream.Close();
    }

    /* ------------------------------------------------------------------ */


    void testGetRemoteAddr(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        
        //case
        apl_int_t liTmp = 0;
        START_THREAD_BODY(myBody, apl_int_t, liTmp)
        {
            apl_int_t liRet = -1;
            CSockAddr loLocalAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAcceptor loSockAcceptor;
            CSockStream loSockStream;
            CTimeValue loTimeValue;

            liRet = loSockAcceptor.Open(loLocalAddr, APL_AF_INET, 10, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Open failed.", (apl_int_t)0, liRet);

            liRet = loSockAcceptor.Accept(loSockStream, NULL, CTimeValue::MAXTIME);
            apl_sleep(APL_TIME_SEC);
            loSockStream.Close();
        }
        END_THREAD_BODY(myBody)
        RUN_THREAD_BODY(myBody)

        apl_sleep(100*APL_TIME_MSEC);    

        apl_int_t liRet = -1;
        CSockAddr loRemoteAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
        CSockAddr loLocalAddr(CLIPORT, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;

        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Create socket failed.", (apl_int_t)0, liRet);

        liRet = apl_connect(loSockStream.GetHandle(), 
                loRemoteAddr.GetAddr(), loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Connect socket failed.", (apl_int_t)0, liRet);

        CSockAddr loGetRemoteAddr;
        loSockStream.GetRemoteAddr(loGetRemoteAddr);
        TestSameAddr(loGetRemoteAddr, SERVPORT, "127.0.0.1", APL_AF_INET);

        loSockStream.Close();

        //end environment
    }

    /* ------------------------------------------------------------------ */

    void testOptionLinger(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment

        //case
        TestOptionLinger(coSockHandleTcp);
        TestOptionLinger(coSockHandleUdp);

        //end environment
    }

    void TestOptionLinger(CSockHandle &aoSockHandle)
    {
        apl_int_t liOnoff = -1; 
        apl_int_t liLonger = -1;

        aoSockHandle.SetOption(CSockHandle::OPT_LINGER, 0, 0);
        aoSockHandle.GetOption(CSockHandle::OPT_LINGER, &liOnoff, &liLonger);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptionLinger failed.", 
                (apl_int_t)0, liOnoff);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testOptionLinger failed.",
                (apl_int_t)0, liLonger);

        aoSockHandle.SetOption(CSockHandle::OPT_LINGER, 1, 1);
        aoSockHandle.GetOption(CSockHandle::OPT_LINGER, &liOnoff, &liLonger);

        if(!liOnoff)
        {
            CPPUNIT_FAIL("testOptionLinger failed.");
        }
        if(!liLonger)
        {
            CPPUNIT_FAIL("testOptionLinger failed.");
        }
    }

    /* ------------------------------------------------------------------ */

    void testIGOOption(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        
        //case
        TestIGOOption(coSockHandleTcp, CSockHandle::OPT_ERROR);
        TestIGOOption(coSockHandleUdp, CSockHandle::OPT_ERROR);
        TestIGOOption(coSockHandleTcp, CSockHandle::OPT_TYPE);
        TestIGOOption(coSockHandleUdp, CSockHandle::OPT_TYPE);
        TestIGOOption(coSockHandleTcp, CSockHandle::OPT_ACCEPTCONN);
        TestIGOOption(coSockHandleUdp, CSockHandle::OPT_ACCEPTCONN);
        
        //end environment
    }

    void TestIGOOption(CSockHandle &aoSockHandle, CSockHandle::EIGOOption aeOpt)
    {
        apl_int_t liOptValue;
        apl_int_t liRet = aoSockHandle.GetOption(aeOpt, &liOptValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testIGOOption failed.", (apl_int_t)0, liRet);
    }

    /* ------------------------------------------------------------------ */

    void testIOption(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        
        //case
        TestIOption(coSockHandleTcp, CSockHandle::OPT_BROADCAST);
        TestIOption(coSockHandleUdp, CSockHandle::OPT_BROADCAST);
        //TestIOption(coSockHandleTcp, CSockHandle::OPT_DEBUG);
        //TestIOption(coSockHandleUdp, CSockHandle::OPT_DEBUG);
        TestIOption(coSockHandleTcp, CSockHandle::OPT_DONTROUTE);
        TestIOption(coSockHandleUdp, CSockHandle::OPT_DONTROUTE);
        TestIOption(coSockHandleTcp, CSockHandle::OPT_KEEPALIVE);

        TestIOption(coSockHandleTcp, CSockHandle::OPT_REUSEADDR);
        TestIOption(coSockHandleUdp, CSockHandle::OPT_REUSEADDR);

        TestIOption(coSockHandleTcp, CSockHandle::OPT_TCPNODELAY);
        //end environment
    }

    void TestIOption(CSockHandle &aoSockHandle, CSockHandle::EIOption aeOpt)
    {
        apl_int_t liOptValue;

        aoSockHandle.SetOption(aeOpt, 0);
        aoSockHandle.GetOption(aeOpt, &liOptValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("testIOption failed.", (apl_int_t)0, liOptValue);

        aoSockHandle.SetOption(aeOpt, 1);
        aoSockHandle.GetOption(aeOpt, &liOptValue); 
        if(!liOptValue)
        {
            CPPUNIT_FAIL("testIOption failed.");
        }
    }

    /* ------------------------------------------------------------------ */ 

    void testIOptionRcvSnd(void)
    {
        PRINT_TITLE_2(__func__); 
        //start case environment
        
        //case 
        TestIOptionRcvSnd(coSockHandleTcp, CSockHandle::OPT_RCVBUF);
        TestIOptionRcvSnd(coSockHandleUdp, CSockHandle::OPT_RCVBUF);
        TestIOptionRcvSnd(coSockHandleTcp, CSockHandle::OPT_SNDBUF);
        TestIOptionRcvSnd(coSockHandleUdp, CSockHandle::OPT_SNDBUF);
        //end environment
    }

    void TestIOptionRcvSnd(CSockHandle &aoSockHandle, CSockHandle::EIOption aeOpt)
    {
        apl_int_t liOptValue;
        aoSockHandle.SetOption(aeOpt, 100);
        aoSockHandle.GetOption(aeOpt, &liOptValue);
        if(liOptValue != 100)
        {
            CPPUNIT_FAIL("testIOptionRcvSnd failed.");
        }
    }

};  

class CTestAclSockHandlePair:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSockHandlePair);
    CPPUNIT_TEST(testOpenPair);
    CPPUNIT_TEST(testGetFirst);
    CPPUNIT_TEST(testGetSecond);
    CPPUNIT_TEST_SUITE_END();
    
public:
    void setUp(void) {}
    void tearDown(void) {}

    void testOpenPair(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CSockHandlePair loSockHandlePair;

        //case
        apl_int_t liRet = loSockHandlePair.Open(APL_AF_UNIX,SOCK_STREAM,0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Create socket handle pair failed.", (apl_int_t)0, liRet);
        
        //end environment
    }

    void testGetFirst(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CSockHandlePair loSockHandlePair;
        apl_int_t liRet = loSockHandlePair.Open(APL_AF_UNIX,SOCK_STREAM,0);

        //case
        if(liRet == 0)
        {
            apl_handle_t liFirst = loSockHandlePair.GetFirst().GetHandle();
            if(!liFirst)
            {
                CPPUNIT_FAIL("Get first pair failed.");
            }
        }

        //end environment

    }

    void testGetSecond(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment
        CSockHandlePair loSockHandlePair;
        apl_int_t liRet = loSockHandlePair.Open(APL_AF_UNIX,SOCK_STREAM,0);

        //case
        if(liRet == 0)
        {
            apl_handle_t liSecond = loSockHandlePair.GetSecond().GetHandle();
            if(!liSecond)
            {
                CPPUNIT_FAIL("Get second pair failed.");
            }
        }

        //end environment
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSockHandle);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSockHandlePair);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

