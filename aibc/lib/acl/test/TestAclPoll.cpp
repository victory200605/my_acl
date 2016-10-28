#include "Test.h"
#include "acl/Poll.h"
#include <deque>
#include <algorithm>
#include <vector>
#include "apl/apl.h"
using namespace acl;

#define RUN_AND_CHECK_RET_INT(func, args) \
    do { \
           apl_int_t ret; \
           ret =  func args; \
           if(ret == APL_INT_C(-1)) \
           { \
               apl_perror(#func #args); \
               CPPUNIT_FAIL(#func #args "fail"); \
           } \
       }while(0)

void testPollCheckResult(CPollee& cpoll, apl_handle_t reArray[], apl_int_t reEvent[], apl_ssize_t arlen)
{
    apl_int_t stateArray[10] = {0};
    apl_ssize_t j;
    for(apl_size_t i = 0;i < arlen;i++)
    {
        for(j = 0;j < arlen;j++)
        {
            if(reArray[j] == cpoll.GetHandle(i))
            {
                CPPUNIT_ASSERT_MESSAGE("poll fail", stateArray[i] == 0);
                CPPUNIT_ASSERT_MESSAGE("poll fail", reEvent[j] == cpoll.GetEvent(i));
                stateArray[i] = 1;
                break;
            }
        }
        CPPUNIT_ASSERT_MESSAGE("poll fail", j < arlen);
    }
}

void setNonblocking(apl_handle_t aiSocket)
{
    apl_int_t aiflag;
    RUN_AND_CHECK_RET_INT(apl_fcntl_getfl, (aiSocket, &aiflag));
    aiflag = aiflag | APL_O_NONBLOCK;
    RUN_AND_CHECK_RET_INT(apl_fcntl_setfl, (aiSocket, aiflag));
}

apl_ssize_t sendN(apl_handle_t aiSocket, void* apcbuf, apl_size_t aulen)
{
    apl_size_t luSend = 0;
    char* lubuf = (char*)apcbuf;
    apl_size_t liRetSz;

    while(luSend < aulen)
    {
        liRetSz = apl_send(aiSocket, lubuf + luSend, aulen - luSend, 0);
        if(liRetSz < 0)
        {
            apl_int_t liErrno = apl_get_errno();
            if(liErrno == APL_EINTR)
            {
                continue;
            }
            else if(liErrno == (APL_EAGAIN || APL_EWOULDBLOCK))    
            {
                break;
            }
            return APL_INT_C(-1);
        }
        if(liRetSz == 0)
        {
            break;
        }
        luSend += liRetSz;
    }
    assert(luSend >= 0 && luSend <= aulen);   
    return luSend;
}

apl_ssize_t revN(apl_handle_t aiSocket, void* apcbuf, apl_size_t aulen)
{
    apl_size_t luRev = 0;
    char* lubuf = (char*)apcbuf;
    apl_size_t liRetSz;

    while(luRev < aulen)
    {
        liRetSz = apl_recv(aiSocket, lubuf + luRev, aulen - luRev, 0);
        if(liRetSz < 0)
        {
            apl_int_t liErrno = apl_get_errno();
            if(liErrno == APL_EINTR)
            {
                continue;
            }
            else if(liErrno == (APL_EAGAIN || APL_EWOULDBLOCK))
            {
                break;
            }
            return APL_INT_C(-1);
        }
        if(liRetSz == 0)
        {
            break;
        }
        luRev += liRetSz;
    }
    assert(luRev >= 0 && luRev <= aulen);
    return luRev;
}

struct CQueue
{
    typedef std::deque<char> QueueT;
    struct apl_mutex_t csMtx;
    struct apl_cond_t csCondPush;

    QueueT coQueue;
 
    CQueue()
    {
        RUN_AND_CHECK_RET_INT(apl_mutex_init, (&csMtx));
        RUN_AND_CHECK_RET_INT(apl_cond_init, (&csCondPush));
    }
    
    ~CQueue()
    {
        RUN_AND_CHECK_RET_INT(apl_mutex_destroy, (&csMtx));
        RUN_AND_CHECK_RET_INT(apl_cond_destroy, (&csCondPush));  
    }

    void Push(void const* apcData, apl_size_t aiSize)
    {
        char* luData = (char*)apcData;
        apl_mutex_lock(&csMtx);

        coQueue.insert(coQueue.end(), luData, luData + aiSize);
        apl_cond_signal(&csCondPush);
        apl_mutex_unlock(&csMtx);
    }

    void CheckAndPop(void const* apcData, apl_size_t aiSize)
    {
        char* luData = (char*)apcData;
        apl_mutex_lock(&csMtx);
        while(coQueue.size() < aiSize)
        {
            apl_cond_wait(&csCondPush, &csMtx, APL_INT64_C(-1));
        }
        CPPUNIT_ASSERT_MESSAGE("check fail", std::equal(luData, luData + aiSize, coQueue.begin()));
        coQueue.erase(coQueue.begin(), coQueue.begin() + aiSize);
        apl_mutex_unlock(&csMtx);
    }

};

struct CThread
{
    std::string moName;
    volatile bool mbStarted;
    apl_thread_t tid;
    struct apl_mutex_t msMtx; 
    struct apl_cond_t csCondStart;
    
    CThread(char const* apcName)
        : moName(apcName)
        , mbStarted(false)
    {
        RUN_AND_CHECK_RET_INT(apl_mutex_init, (&msMtx));
        RUN_AND_CHECK_RET_INT(apl_cond_init, (&csCondStart));
    }

    virtual ~CThread()
    {
        RUN_AND_CHECK_RET_INT(apl_mutex_destroy, (&msMtx));
        RUN_AND_CHECK_RET_INT(apl_cond_destroy, (&csCondStart));
    }

    void Start()
    {
        mbStarted = true;
        apl_thread_create(&tid, &CThread::_ThreadEntry, this, 0, 0);
    }
    
    void Join()
    {
        apl_mutex_lock(&msMtx);
        while(mbStarted)
        {
            apl_cond_wait(&csCondStart, &msMtx, APL_TIME_SEC);
        }
        apl_mutex_unlock(&msMtx);
    }

    void OnExit()
    {
        apl_mutex_lock(&msMtx);
        mbStarted = false;
        apl_cond_signal(&csCondStart);
        apl_mutex_unlock(&msMtx);
    }

    virtual void Run() = 0;

    char const* GetName()
    {
        return moName.c_str();
    } 

    static void* _ThreadEntry(void* arg)
    {
        CThread* loThread = (CThread*)arg;
        loThread->Run();
        loThread->OnExit();
        
        return APL_NULL;
    }
}; 

struct CProducer
    : public CThread
{
    apl_handle_t muSocket;
    apl_size_t muBufferSize;
    apl_size_t muSendSize;
    CQueue* mpoQueue;

    CProducer(
        char const* mpcName,
        apl_handle_t aiSocket,
        apl_size_t aiBufferSize,
        apl_size_t aiSendSize,
        CQueue* aiQueue)
        : CThread(mpcName)
        , muSocket(aiSocket)
        , muBufferSize(aiBufferSize)
        , muSendSize(aiSendSize)
        , mpoQueue(aiQueue)
    {
    }

    virtual ~CProducer()
    {
    }

    virtual void Run()
    {
        apl_size_t liTotal = 0;
        char* lubuf = (char*)apl_malloc(muBufferSize);     
        if(lubuf == NULL)
        {
             CPPUNIT_FAIL("malloc fail");
        }

        apl_size_t liRetSz; 

        while(liTotal < muSendSize)
        {
            std::generate_n(lubuf, muBufferSize, rand);
            liRetSz = sendN(muSocket, lubuf, muBufferSize);
            CPPUNIT_ASSERT_MESSAGE("send data fail", liRetSz > 0);
            mpoQueue->Push(lubuf, liRetSz);
            liTotal += liRetSz;
        }

        apl_free(lubuf);
    }
};

struct CCustomer
    : public CThread
{
    apl_handle_t muSocket;
    apl_size_t muBufferSize;
    apl_size_t muRevSize;
    CQueue* mpoQueue;             

    CCustomer(
        char const* mpcName,
        apl_handle_t aiSocket,
        apl_size_t aiBufferSize,
        apl_size_t aiRevSize,
        CQueue* aiQueue)
        : CThread(mpcName)
        , muSocket(aiSocket)
        , muBufferSize(aiBufferSize)
        , muRevSize(aiRevSize)
        , mpoQueue(aiQueue)
    {
    }

    virtual ~CCustomer()
    {
    }

    virtual void Run()
    {
        apl_size_t liRetSz;
        apl_size_t liTotal = 0;
        char* lubuf = (char*)apl_malloc(muBufferSize);
        if(lubuf == NULL)
        {
            CPPUNIT_FAIL("malloc fail");
        }
       
        while(liTotal < muRevSize)
        { 
            liRetSz = revN(muSocket, lubuf, (muRevSize - liTotal) > muBufferSize ? muBufferSize : muRevSize - liTotal);
	    CPPUNIT_ASSERT_MESSAGE("receive data fail", liRetSz > 0);
            mpoQueue->CheckAndPop(lubuf, liRetSz);
            liTotal += liRetSz;
        }
    
        apl_free(lubuf);
    }
};

struct CTranfer
    : public CThread
{
    apl_handle_t sdSocket;
    apl_handle_t rvSocket;
    apl_size_t muBufferSize;
    apl_size_t muTranSize;
    CTimeValue const& muTimeout; 
    CPollee cpoll;

    CTranfer(
        char const* mpcName,
        apl_handle_t aisSocket,
        apl_handle_t airSocket, 
        apl_size_t aiBufferSize,
        apl_size_t aiTranSize,
        CTimeValue const& aiTimeout)
        : CThread(mpcName)
        , sdSocket(aisSocket)
        , rvSocket(airSocket)
        , muBufferSize(aiBufferSize)
        , muTranSize(aiTranSize)
        , muTimeout(aiTimeout) 
    {
    } 
    
    virtual ~CTranfer()
    {
    }

    virtual void Run()
    {
        apl_size_t liRetSz;
        apl_size_t lisTotal = 0;
        apl_size_t lirTotal = 0;
        char* libuf = (char*)apl_malloc(muBufferSize);
        apl_size_t libufCur = 0; 
        apl_int_t liRet;
        
        cpoll.Initialize(APL_INT_C(2));
        liRet = cpoll.AddHandle(sdSocket, APL_POLLOUT);
        CPPUNIT_ASSERT_MESSAGE("Add Handle fail", liRet == 0);
        liRet = cpoll.AddHandle(rvSocket, APL_POLLIN);
        CPPUNIT_ASSERT_MESSAGE("Add Handle fail", liRet == 0);
        
        while(lisTotal < muTranSize || lirTotal < muTranSize)
        {
            if(libufCur == muBufferSize || lirTotal == muTranSize)
            {
                cpoll.DelHandle(rvSocket);
            }
            else
            {
                liRet = cpoll.AddHandle(rvSocket, APL_POLLIN);
                //CPPUNIT_ASSERT_MESSAGE("Add Handle fail", liRet == 0);
            }

            if(libufCur == 0 || lisTotal == muTranSize)
            {
                cpoll.DelHandle(sdSocket);
            }
            else
            {
                liRet = cpoll.AddHandle(sdSocket, APL_POLLOUT);
                //CPPUNIT_ASSERT_MESSAGE("Add Handle fail", liRet == 0);
            }
            while(true)
            { 
                liRet = cpoll.Poll(muTimeout);
                if(liRet < 0)
                {
                    apl_int_t aiErrno = apl_get_errno();
                    if(aiErrno == EINTR)
                    {
                        continue;
                    }
                    CPPUNIT_FAIL("Poll fail");
                }
                break;
            }
            
            for(CPolleeIterator iter = cpoll.Begin();iter != cpoll.End();iter++)
            {
                if(iter.Event() & APL_POLLIN)
                {
                    liRetSz = revN(iter.Handle(), libuf + libufCur, muBufferSize - libufCur);
		    if(liRetSz < 0)
                    {
                        apl_int_t aiErrno = apl_get_errno();
                        if(aiErrno == EAGAIN || aiErrno == EWOULDBLOCK)
                        {
                            continue;
                        }
                        CPPUNIT_FAIL("revN fail");
                    }
                    else
                    {
                        libufCur += liRetSz;
                        lirTotal += liRetSz;
                    }
                }

                if(iter.Event() & APL_POLLOUT)
                {
                    liRetSz = sendN(iter.Handle(), libuf, libufCur);      
                    if(liRetSz < 0)
                    {
                        apl_int_t aiErrno = apl_get_errno();
                        if(aiErrno == EAGAIN || aiErrno == EWOULDBLOCK)
                        {
                            continue;
                        }
                        CPPUNIT_FAIL("send fail");
                    }
                    else
                    {
                        apl_memmove(libuf, libuf + liRetSz, libufCur);
                        libufCur -= liRetSz;
                        lisTotal += liRetSz;
                    }
                }
            }
        }
        apl_free(libuf);
    }
};
 
class CTestAclPoll : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclPoll);
    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testDel);
    CPPUNIT_TEST(testGetEvent);
    CPPUNIT_TEST(testPC);
    CPPUNIT_TEST(testPTC);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}
    void testAdd(void)
    {
        CPollee cpoll;
        cpoll.Initialize(10);
        apl_ssize_t ret;
        apl_handle_t sockArray[10];
        apl_int_t eArray[10];

        for(apl_int_t i = 0;i < 10;i++)
        {
            eArray[i] = APL_POLLOUT;
        }
        for(apl_int_t i = 0;i < 5;i++)
        {
            RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockArray[2 * i], &sockArray[2 * i + 1]));
            cpoll.AddHandle(sockArray[2 * i], APL_POLLOUT | APL_POLLIN);
            cpoll.AddHandle(sockArray[2 * i + 1], APL_POLLOUT | APL_POLLIN); 
            ret = cpoll.Poll();
            CPPUNIT_ASSERT_MESSAGE("poll fail", ret == 2 * i + 2);
            testPollCheckResult(cpoll, sockArray, eArray, 2 * i + 2);
        }
        
        // test multiple add
        ret = cpoll.AddHandle(sockArray[0], APL_POLLOUT);
        //CPPUNIT_ASSERT_MESSAGE("multiple add fail", ret == -1);
        ret = cpoll.AddHandle(sockArray[0], APL_POLLIN);
        //CPPUNIT_ASSERT_MESSAGE("multiple add fail", ret == -1);
        ret = cpoll.Poll();
        CPPUNIT_ASSERT_MESSAGE("poll fail", ret == 10);
        testPollCheckResult(cpoll, sockArray, eArray, 10);
        
        for(apl_int_t i = 0;i < 10;i++)
        {
            RUN_AND_CHECK_RET_INT(apl_close, (sockArray[i])); 
        }
    }
    
    void testDel(void)
    {
        CPollee cpoll;
        cpoll.Initialize(10);
        apl_ssize_t ret;
        apl_handle_t sockArray[10];
        apl_handle_t eArray[10];

        for(apl_int_t i = 0;i < 10;i++)
        {
            eArray[i] = APL_POLLOUT;
        }

        for(apl_int_t i = 0;i < 5;i++)
        {
            RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockArray[2 * i], &sockArray[2 * i + 1]));
            ret = cpoll.AddHandle(sockArray[2 * i], APL_POLLIN | APL_POLLOUT);
            CPPUNIT_ASSERT_MESSAGE("poll AddHandle fail", ret == 0);
            ret = cpoll.AddHandle(sockArray[2 * i + 1], APL_POLLIN | APL_POLLOUT);
            CPPUNIT_ASSERT_MESSAGE("poll AddHandle fail", ret == 0);
        }
        ret = cpoll.Poll();
        for(apl_int_t i = 9;i >= 0;i--)
        {
            CPPUNIT_ASSERT_MESSAGE("poll fail", ret == i + 1);
            testPollCheckResult(cpoll, sockArray, eArray, i + 1);
            cpoll.DelHandle(sockArray[i]);
            if(i != 0)
                ret = cpoll.Poll();
        }
        
        // test multiple delete
        cpoll.AddHandle(sockArray[0], APL_POLLIN | APL_POLLOUT);
        cpoll.AddHandle(sockArray[1], APL_POLLIN | APL_POLLOUT);
        cpoll.DelHandle(sockArray[1]);
        ret = cpoll.DelHandle(sockArray[1]); 
        //CPPUNIT_ASSERT_MESSAGE("multiple delete fail", ret == -1);
        ret = cpoll.Poll();
        CPPUNIT_ASSERT_MESSAGE("poll fail", ret == 1); 
        testPollCheckResult(cpoll, sockArray, eArray, 1);

        for(apl_int_t i = 0;i < 10;i++)
        {
            RUN_AND_CHECK_RET_INT(apl_close, (sockArray[i]));
        }
    }
  
    void testGetEvent(void)
    {
        //test error event
        apl_handle_t sockA;
        apl_int_t testEvent;
        char hostaddr[] = "127.0.0.0";
        struct apl_sockaddr_t testaddr;
        
        sockA = apl_socket(APL_AF_UNIX, APL_SOCK_STREAM, 0);
        setNonblocking(sockA); 
        apl_sockaddr_setfamily(&testaddr, APL_AF_UNIX);
        apl_sockaddr_setaddr(&testaddr, sizeof(testaddr), hostaddr);
        apl_sockaddr_setport(&testaddr, sizeof(testaddr), 10006);
        
        apl_handle_t sockB;
        apl_handle_t sockC;
        apl_socketpair(APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockB, &sockC);
        apl_close(sockC);
        CPollee cpoll;
        cpoll.Initialize(1);
        cpoll.AddHandle(sockB, APL_POLLIN | APL_POLLOUT);
        cpoll.Poll();
        testEvent = cpoll.GetEvent(0);
        CPPUNIT_ASSERT_MESSAGE("event return fail", (testEvent == APL_POLLOUT | APL_POLLIN));
    }
 
    void testPC(void)
    {
        apl_handle_t sockA, sockB;
        apl_size_t BUF_SIZE = 1024;
        apl_size_t TEST_SIZE = 10 * 1024 * 1024;
        
        RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockA, &sockB));

        CQueue loQ;
        CProducer loP("Producer", sockA, BUF_SIZE, TEST_SIZE, &loQ);
        CCustomer loC("Customer", sockB, BUF_SIZE, TEST_SIZE, &loQ);
        
        loP.Start();
        loC.Start();

        loP.Join();
        loC.Join(); 
        RUN_AND_CHECK_RET_INT(apl_close, (sockA));
        RUN_AND_CHECK_RET_INT(apl_close, (sockB));
    }

    void testPTC(void)
    {
        apl_handle_t sockA;
        apl_handle_t sockB;
        apl_handle_t sockC;
        apl_handle_t sockD;
   
        apl_size_t BUF_SIZE = 1024;
        apl_size_t TEST_SIZE = 10 * 1024 * 1024;
        RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockA, &sockB));
        RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockC, &sockD));

        setNonblocking(sockB);
        setNonblocking(sockC);

        CQueue loQ;
        CProducer loP("Producer", sockA, BUF_SIZE, TEST_SIZE, &loQ);
        CCustomer loC("Customer", sockD, BUF_SIZE, TEST_SIZE, &loQ);
        CTranfer  loT1("Tranfer-1", sockC, sockB, BUF_SIZE, TEST_SIZE, CTimeValue::MAXTIME);

        loP.Start();
        loT1.Start();
        loC.Start();

        loP.Join();
        loT1.Join();
        loC.Join();

        RUN_AND_CHECK_RET_INT(apl_close, (sockA));
        RUN_AND_CHECK_RET_INT(apl_close, (sockB));
        RUN_AND_CHECK_RET_INT(apl_close, (sockC));
        RUN_AND_CHECK_RET_INT(apl_close, (sockD));
    } 
};

class CTestAclPollIterator : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclPollIterator);
    CPPUNIT_TEST(testDoubleAdd);
    CPPUNIT_TEST(testEqual);
    CPPUNIT_TEST(testUnequal);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp() {}
    void tearDown() {}

    void testDoubleAdd(void)
    {
        std::vector<apl_handle_t> sockArray(10);
        apl_handle_t tmpArray[10];
        apl_int_t liRetCode = 0;
        CPollee loPoll;
        
        liRetCode = loPoll.Initialize(10);
        CPPUNIT_ASSERT_MESSAGE("Poll Initialize fail", liRetCode == 0);

        for(apl_int_t i = 0;i < 5;i++)
        {
            RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &tmpArray[2 * i], &tmpArray[2 * i + 1]));
            sockArray[2 * i] = tmpArray[2 * i];
            sockArray[2 * i + 1] = tmpArray[2 * i + 1];
            liRetCode = loPoll.AddHandle(sockArray[2 * i], APL_POLLIN | APL_POLLOUT);
            CPPUNIT_ASSERT_MESSAGE("test DoubleAdd fail", liRetCode == 0);
            liRetCode = loPoll.AddHandle(sockArray[2 * i + 1], APL_POLLIN | APL_POLLOUT);
            CPPUNIT_ASSERT_MESSAGE("test DoubleAdd fail", liRetCode == 0);
        }
        loPoll.Poll();
        for(CPollee::IteratorType iter = loPoll.Begin();iter != loPoll.End();iter++)
        {
            std::vector<apl_handle_t>::iterator iterA;
            for(iterA = sockArray.begin();iterA != sockArray.end();iterA++)
            {
               if(*iterA == iter.Handle())
               {
                   break;
               }
            }
            CPPUNIT_ASSERT_MESSAGE("Event() fail", iter.Event() == APL_POLLOUT);
            CPPUNIT_ASSERT_MESSAGE("operator++ fail", iterA != sockArray.end());
            sockArray.erase(iterA);
        }
    }
    
    void testEqual(void)
    {
        apl_handle_t sockA, sockB;
        CPollee cpoll1, cpoll2;
        RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockA, &sockB));
        cpoll1.AddHandle(sockA, APL_POLLIN | APL_POLLOUT);
        cpoll1.AddHandle(sockB, APL_POLLIN | APL_POLLOUT);
        cpoll2.AddHandle(sockA, APL_POLLIN | APL_POLLOUT);
        cpoll2.AddHandle(sockB, APL_POLLIN | APL_POLLOUT);
        
        cpoll1.Poll();
        cpoll2.Poll();
        CPolleeIterator iter1;
        for(iter1 = cpoll1.Begin();iter1 != cpoll1.End();iter1++)
        {
            for(CPolleeIterator iter2 = cpoll2.Begin();iter2 != cpoll2.End();iter2++)
            {
                CPPUNIT_ASSERT_MESSAGE("operator== fail", (iter1 == iter2) == false);
            }
        }
        CPolleeIterator iter3 = cpoll1.Begin();
        for(iter1 = cpoll1.Begin();iter1 != cpoll1.End();iter1++, iter3++)
        {
            CPPUNIT_ASSERT_MESSAGE("operator== fail", iter1 == iter3);
        }
    }

    void testUnequal(void)
    {
        apl_handle_t sockA, sockB;
        CPollee cpoll1, cpoll2;
        RUN_AND_CHECK_RET_INT(apl_socketpair, (APL_AF_UNIX, APL_SOCK_STREAM, 0, &sockA, &sockB)); 
        cpoll1.AddHandle(sockA, APL_POLLIN | APL_POLLOUT);
        cpoll1.AddHandle(sockB, APL_POLLIN | APL_POLLOUT);
        cpoll2.AddHandle(sockA, APL_POLLIN | APL_POLLOUT);
        cpoll2.AddHandle(sockB, APL_POLLIN | APL_POLLOUT);
        
        cpoll1.Poll();
        cpoll2.Poll();
        CPolleeIterator iter1;
        for(iter1 = cpoll1.Begin();iter1 != cpoll1.End();iter1++)
        {
            for(CPolleeIterator iter2 = cpoll2.Begin();iter2 != cpoll2.End();iter2++)
            {
                CPPUNIT_ASSERT_MESSAGE("operator== fail", iter1 != iter2);
            }
        }
        CPolleeIterator iter3 = cpoll1.Begin();
        for(iter1 = cpoll1.Begin();iter1 != cpoll1.End();iter1++, iter3++)
        {
            CPPUNIT_ASSERT_MESSAGE("operator== fail", (iter1 != iter3) == false);
        }
         
    }  
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclPoll);
CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclPollIterator);
int main()
{
    RUN_ALL_TEST(__FILE__);
}
