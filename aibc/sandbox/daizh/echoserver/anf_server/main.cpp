
//#include "Test.h"
#include "anf/util/SyncClient.h"
#include "anf/protocol/SimpleAsyncMessage.h"
#include "anf/filter/SimpleAsyncMessageFilter.h"
#include "anf/IoSockAcceptor.h"
#include "anf/IoPollingProcessor.h"
#include "anf/IoHandler.h"
#include "anf/IoSession.h"
#include "acl/SockAddr.h"
#include "acl/GetOpt.h"

acl::TNumber<apl_int64_t, acl::CLock> goTraffic(0); 
    
class MyHandler : public anf::IoHandler
{
public:
    virtual void SessionOpened( anf::SessionPtrType& aoSession )
    {
        printf("Open session %d\n", aoSession->GetHandle() );
        //aoSession->ResumeWrite(aoSession);
    }
    
    virtual void SessionClosed( anf::SessionPtrType& aoSession )
    {
        printf("Close session %d\n", aoSession->GetHandle() );
    }
        
    virtual void MessageReceived( anf::SessionPtrType& aoSession, const acl::CAny& aoMessage )
    {
        anf::CSimpleAsyncMessage* lpoMessage = NULL;
        
        ACL_ASSERT(aoMessage.CastTo<anf::CSimpleAsyncMessage*>(lpoMessage) );
            
        aoSession->Write(aoSession, lpoMessage);
        
        goTraffic += lpoMessage->GetBody().GetLength();
        
        ACL_DELETE(lpoMessage);
    }
    
    virtual void MessageSent( anf::SessionPtrType& aoSession, acl::CAny& aoMessage )
    {
    }
};

void* Stat(void*)
{
    apl_int64_t li64Curr = 0;
    apl_int64_t li64Prev = 0;
    
    while(true)
    {
        apl_sleep(APL_TIME_SEC);
        
        li64Curr = goTraffic;
         
        printf("Curr traffic = %lfKB\n", float(li64Curr - li64Prev) / (1024 * 1024) );
        
        li64Prev = li64Curr;
    }
    
    return NULL;
}
/*
class CTestAnfAcceptor : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAnfAcceptor);
    CPPUNIT_TEST(testAccept);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testAccept(void)
    {
        PRINT_TITLE_2(__func__);
        acl::CSockAddr loAddr(5555, "127.0.0.1");
        anf::CIoAcceptor loAcceptor;
        MyHandler* lpoHandler = new MyHandler;
        
        loAcceptor.SetHandler(lpoHandler);
        loAcceptor.Bind(loAddr);
        
        acl::CThreadManager loThreadManager;
    
        loThreadManager.Spawn(Stat, NULL);
    
//        apl_sleep(APL_TIME_SEC * 30);
//        exit(0);
        loThreadManager.WaitAll();
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAnfAcceptor);
*/
int main( int argc, char* argv[] )
{
    apl_int_t liOptChar;
    apl_int16_t li16Port = 5555;
    char lacIpAddress[12] = "127.0.0.1";
    apl_int_t liSleepTime = 10;
    acl::CGetOpt loOpt(argc, argv, "i:p:s:");
    while( (liOptChar = loOpt() ) != EOF )
	{
		switch (liOptChar)
		{
		    case 'i':
			    apl_strncpy(lacIpAddress, loOpt.OptArg(), sizeof(lacIpAddress) );
			    break;
			case 'p':
			    li16Port = apl_strtoi32(loOpt.OptArg(), NULL, 10);
				break;
		    case 's':
		        liSleepTime = apl_strtoi32(loOpt.OptArg(), NULL, 10);
		        break;
        }
    }
    
    {
        acl::CSockAddr loAddr(li16Port, lacIpAddress);
        anf::CIoSockAcceptor loAcceptor;
        MyHandler* lpoHandler = new MyHandler;
        
        loAcceptor.GetFilterChain()->AddFirst("simple-async-message", new anf::CSimpleAsyncMessageFilter);
        loAcceptor.GetConfiguration()->SetProcessorPoolSize(10);
        loAcceptor.GetConfiguration()->SetReadBufferSize(2048);
        loAcceptor.GetConfiguration()->SetWriteBufferSize(2048);
        loAcceptor.SetHandler(lpoHandler);
        loAcceptor.Bind(loAddr);
        
        acl::CThreadManager loThreadManager;
    
        loThreadManager.Spawn(Stat, NULL);
    
        //apl_sleep(APL_TIME_SEC* liSleepTime);
        //apl_exit(0);
        loThreadManager.WaitAll();
    }
    //RUN_ALL_TEST(__FILE__);
}
