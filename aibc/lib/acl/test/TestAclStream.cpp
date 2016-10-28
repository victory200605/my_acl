#include "Test.h"
#include "acl/Stream.h"
#include "acl/MemoryBlock.h"
#include "acl/File.h"
#include "acl/SockStream.h"

using namespace acl;

#define BEGIN_SERV(paramtype, param, port) \
        paramtype& _param = param; \
        apl_int_t liRet = -1; \
        CSockAddr loLocalAddr(port, "127.0.0.1", APL_AF_INET); \
        liRet = _param.Open(APL_AF_INET, APL_SOCK_STREAM, 0); \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Create socket failed.",  (apl_int_t)0, liRet); \
        liRet = _param.SetOption(CSockHandle::OPT_REUSEADDR, 1); \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set reuse addr failed.",  (apl_int_t)0, liRet); \
        if(apl_bind(_param.GetHandle(), loLocalAddr.GetAddr(), loLocalAddr.GetLength()) != 0) \
        { \
            _param.Close(); \
            CPPUNIT_FAIL("Bind failed"); \
            return ; \
        } \
        if ( apl_listen(_param.GetHandle(), 10) != 0 ) \
        { \
            _param.Close(); \
            CPPUNIT_FAIL("Listen failed"); \
            return ; \
        } \
        apl_size_t liLen = 0; \
        apl_handle_t liListenfd = _param.GetHandle(); \
        while(true) \
        { \
            apl_handle_t liConnfd = apl_accept(liListenfd, NULL, &liLen) ; \
            if (liConnfd < 0) \
            { \
                if(errno == APL_EINTR) \
                { \
                    continue; \
                } \
                CPPUNIT_FAIL("Accept failed"); \
                return ; \
            } \
            _param.SetHandle(liConnfd); 

#define END_SERV(param) \
            param.Close(); \
            break; \
        }

/* ----------------------------------------------------------------- */

class CTestAclStream:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclStream);
    CPPUNIT_TEST(testRDWRMem);
    CPPUNIT_TEST(testRDWRFile);
    CPPUNIT_TEST(testRDWRSock);
    CPPUNIT_TEST(testRdHWrN);
    CPPUNIT_TEST(testReadUntil);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void testRDWRMem(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char const* lpcIn = "1234567890";
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));

        CMemoryBlock loMemBlock(100);
        COutputStream loMemOutput(loMemBlock);
        CInputStream loMemInput(loMemBlock);

        //case
        loMemOutput.Write(lpcIn,10);
        loMemInput.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("CMemoryBlock Read/Write failed.", apl_strcmp(lsBuf, lpcIn) == 0);

        loMemOutput.Write("abcd");
        memset(lsBuf, 0, sizeof(lsBuf));
        loMemBlock.SetReadPtr(loMemBlock.GetBase());
        loMemInput.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("CMemoryBlock Read/Write failed.", 
                apl_strcmp(lsBuf, "1234567890abcd") == 0);

        //end environment
    }

    void testRDWRFile(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char const* lpcIn = "1234567890";
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));

        CFile loFile;
        loFile.Open("test.txt", APL_O_RDWR|APL_O_CREAT|APL_O_TRUNC, 0666);
        COutputStream loFileOutput(loFile);
        CInputStream loFileInput(loFile);

        //case
        loFileOutput.Write(lpcIn);
        loFile.Seek(0, APL_SEEK_SET);

        loFileInput.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("CFile Read Write failed.", apl_strcmp(lsBuf, lpcIn) == 0);

        loFile.Seek(-1, APL_SEEK_END);
        loFileOutput.Write(lpcIn, 5);
        memset(lsBuf, 0, sizeof(lsBuf));
        loFile.Seek(0, APL_SEEK_SET);
        loFileInput.Read(lsBuf, sizeof(lsBuf));
        CPPUNIT_ASSERT_MESSAGE("CFile Read Write failed.", 
                apl_strcmp(lsBuf, "123456789012345") == 0);

        //end environment
        loFile.Close();
    }

    void testRDWRSock(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;

        //case
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;
            CInputStream loSockInput(loSockStream);

            BEGIN_SERV(CSockStream, loSockStream, 10001)
            {
                char lsRecvBuf[50];
                memset(lsRecvBuf, 0, sizeof(lsRecvBuf));
                const char *lpcSendBuf = "1234567890";

                loSockInput.Read(lsRecvBuf, sizeof(lsRecvBuf));
                CPPUNIT_ASSERT_MESSAGE("Sock stream Read failed.", 
                        apl_strcmp(lsRecvBuf, lpcSendBuf) == 0);
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;
        CSockAddr loRemoteAddr(10001, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;
        COutputStream loSockOutput(loSockStream);
        const char *lpcBuf = "1234567890";

        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Create socket failed.",  (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), loRemoteAddr.GetAddr(), loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Connect socket failed.",  (apl_int_t)0, liRet);
        apl_ssize_t liSendSize = loSockOutput.Write((void*)lpcBuf, apl_strlen(lpcBuf));
        CPPUNIT_ASSERT_MESSAGE("Send failed.", (apl_ssize_t)apl_strlen(lpcBuf) == liSendSize);

        //end environment
        loSockStream.Close();
    }

    void testRdHWrN(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liTmp = 0;

        //case
        START_THREAD_BODY(mybody, apl_int_t, liTmp)
        {
            CSockStream loSockStream;
            CInputStream loSockInput(loSockStream);

            BEGIN_SERV(CSockStream, loSockStream, 10002)
            {
                apl_uint16_t liUint16;
                loSockInput.ReadToH(liUint16);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("ReadToH apl_uint16_t failed.", 
                        (apl_uint16_t)0x1020, liUint16);

                apl_uint32_t liUint32;
                loSockInput.ReadToH(liUint32);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("ReadToH apl_uint32_t failed.", 
                        (apl_uint32_t)0x10203040, liUint32);

                apl_uint64_t liUint64;
                loSockInput.ReadToH(liUint64);
                CPPUNIT_ASSERT_MESSAGE("ReadToH apl_uint64_t failed.", 
                        (apl_uint64_t)0x1020304050607080ull == liUint64);

                apl_int16_t liInt16;
                loSockInput.ReadToH(liInt16);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("ReadToH apl_int16_t failed.", 
                        (apl_int16_t)0x1020, liInt16);

                apl_int32_t liInt32;
                loSockInput.ReadToH(liInt32);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("ReadToH apl_int32_t failed.", 
                        (apl_int32_t)0x10203040, liInt32);

                apl_int64_t liInt64;
                loSockInput.ReadToH(liInt64);
                CPPUNIT_ASSERT_MESSAGE("ReadToH apl_int64_t failed.", 
                        (apl_int64_t)0x1020304050607080ll == liInt64);
            }
            END_SERV(loSockStream)
        }
        END_THREAD_BODY(mybody)
        RUN_THREAD_BODY(mybody)

        apl_sleep(100*APL_TIME_MSEC);

        apl_int_t liRet = -1;                                   
        CSockAddr loRemoteAddr(10002, "127.0.0.1", APL_AF_INET);
        CSockStream loSockStream;                               
        COutputStream loSockOutput(loSockStream);               

        liRet = loSockStream.Open(APL_AF_INET, APL_SOCK_STREAM, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Create socket failed.",  (apl_int_t)0, liRet);
        liRet = apl_connect(loSockStream.GetHandle(), loRemoteAddr.GetAddr(), 
                loRemoteAddr.GetLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Connect socket failed.",  (apl_int_t)0, liRet);                             
        apl_ssize_t liSendSize = loSockOutput.WriteToN((apl_uint16_t)0x1020);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write apl_uint16_t failed.", 
                (apl_ssize_t)sizeof(apl_uint16_t), liSendSize);
        liSendSize = loSockOutput.WriteToN((apl_uint32_t)0x10203040);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write apl_uint32_t failed.", 
                (apl_ssize_t)sizeof(apl_uint32_t), liSendSize);
        liSendSize = loSockOutput.WriteToN((apl_uint64_t)0x1020304050607080ull); 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write apl_uint64_t failed.", 
                (apl_ssize_t)sizeof(apl_uint64_t), liSendSize);

        liSendSize = loSockOutput.WriteToN((apl_int16_t)0x1020);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write apl_int16_t failed.", 
                (apl_ssize_t)sizeof(apl_int16_t), liSendSize);
        liSendSize = loSockOutput.WriteToN((apl_int32_t)0x10203040);                                                 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write apl_int32_t failed.", 
                (apl_ssize_t)sizeof(apl_int32_t), liSendSize);  
        liSendSize = loSockOutput.WriteToN((apl_int64_t)0x1020304050607080ll); 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write apl_int64_t failed.", 
                (apl_ssize_t)sizeof(apl_int64_t), liSendSize);

        //end environment
        loSockStream.Close();
    }

    void testReadUntil(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char const* lpcIn = "1234567890abcd";
        CMemoryBlock loMemBlock(100);
        COutputStream loMemOutput(loMemBlock);
        loMemOutput.Write(lpcIn);

        CInputStream loMemInput(loMemBlock);
        char const* lpcTag1 = "789";
        char const* lpcTag2 = "ab";
        char lsBuf[50];
        memset(lsBuf, 0, sizeof(lsBuf));

        //case
        loMemInput.ReadUntil(lsBuf, sizeof(lsBuf), lpcTag1);
        CPPUNIT_ASSERT_MESSAGE("ReadUntil failed.", apl_strcmp(lsBuf, "123456789") == 0);
        loMemBlock.SetReadPtr(loMemBlock.GetBase());
        loMemInput.ReadUntil(lsBuf, sizeof(lsBuf), lpcTag2);
        CPPUNIT_ASSERT_MESSAGE("ReadUntil failed.", apl_strcmp(lsBuf, "1234567890ab") == 0);

        //end environment
    }
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclStream);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

