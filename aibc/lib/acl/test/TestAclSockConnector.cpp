#include "Test.h"
#include "acl/SockConnector.h"
#include "acl/SockAcceptor.h"

using namespace acl;

#define SERVPORT 12341

class CTestAclSockConnector:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSockConnector);
    CPPUNIT_TEST(testSockConnector);
    CPPUNIT_TEST(testSockConnectorBindAddr);
    CPPUNIT_TEST(testSockConnectorTimeout);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) 
    {
        this->miStat = 0;
        
        START_THREAD_BODY(mybody, apl_int_t, miStat)
        {
            CSockAddr     loLocalAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockStream   loPeer;
            CSockAcceptor loAcceptor;
            CTimeValue    loTimeout(5);
            CPPUNIT_ASSERT(loAcceptor.Open(loLocalAddr) == 0);
  
            miStat = 1;
            
            char lacBuffer[4];
            CPPUNIT_ASSERT(loAcceptor.Accept(loPeer) == 0);
            CPPUNIT_ASSERT(loPeer.Recv(lacBuffer, sizeof(lacBuffer)) == sizeof(lacBuffer) );
            
            CPPUNIT_ASSERT(strncmp(lacBuffer, "OK\r\n", 4) == 0);
            
            loPeer.Close();
            loAcceptor.Close();
            
            miStat = 2;
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY_DETACH(mybody)    
    }
    
    void tearDown(void)
    {
        WAIT_EXP(this->miStat == 2);
    }

    void testSockConnector(void)
    {
        PRINT_TITLE_2(__func__);
        
        WAIT_EXP(this->miStat == 1);
        
        //case
        START_THREAD_BODY(mybody, apl_int_t, miStat)
        {
            CSockAddr      loRemoteAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockConnector loSockConnector;
            CSockStream    loPeer;
            CTimeValue     loTimeValue(10);

            CPPUNIT_ASSERT(loSockConnector.Connect(loPeer, loRemoteAddr, loTimeValue, NULL, 0) == 0);
           
            CPPUNIT_ASSERT(loPeer.Send("OK\r\n", 4) == 4);
            
            loPeer.Close();
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)
        
        //end environment
    }

    void testSockConnectorBindAddr(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        WAIT_EXP(this->miStat == 1);
        
        //case
        START_THREAD_BODY(mybody, apl_int_t, miStat)
        {
            CSockAddr      loRemoteAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockAddr      loLocalAddr(14321, "127.0.0.1", APL_AF_INET);
            CSockConnector loSockConnector;
            CSockStream    loPeer;
            CTimeValue     loTimeValue(10);

            CPPUNIT_ASSERT(loSockConnector.Connect(loPeer, loRemoteAddr, 
                    loTimeValue, &loLocalAddr, 0) == 0);
            
            CPPUNIT_ASSERT(loPeer.Send("OK\r\n", 4) == 4);
            CPPUNIT_ASSERT(loPeer.GetLocalAddr(loLocalAddr) == 0);
            CPPUNIT_ASSERT(loLocalAddr.GetPort() == 14321);
            
            loPeer.Close();
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        //end environment
    }

    void testSockConnectorTimeout(void)
    {
        PRINT_TITLE_2(__func__);
        WAIT_EXP(this->miStat == 1);
        
        //case
        START_THREAD_BODY(mybody, apl_int_t, miStat)
        {
            CSockAddr      loRemoteAddr(SERVPORT, "127.0.0.1", APL_AF_INET);
            CSockConnector loSockConnector;
            CSockStream    loPeer;
            CTimeValue     loTimeValue(3);

            CPPUNIT_ASSERT(loSockConnector.Connect(loPeer, loRemoteAddr, loTimeValue, NULL, 0) == 0);
           
            CPPUNIT_ASSERT(loPeer.Send("OK\r\n", 4) == 4);
            
            loPeer.Close();
            
            WAIT_EXP(this->miStat == 2);
            
            CPPUNIT_ASSERT(loSockConnector.Connect(loPeer, loRemoteAddr, loTimeValue, NULL, 0) == -1);
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)
        
        //end environment
    }
    
private:
    apl_int_t miStat;
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSockConnector);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

