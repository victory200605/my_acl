#include "Test.h"

void TestErrnoGetSet(apl_int_t ai_errnum, char const* msg) 
{ 
    if ( apl_strerror(ai_errnum) == APL_NULL) 
    { 
       CPPUNIT_FAIL(msg); 
    } 
    
    apl_set_errno(ai_errnum); 
    
    CPPUNIT_ASSERT(apl_get_errno() == ai_errnum); 
   
    apl_perror(msg); 
}

#define TEST_ERRNO_GETSET(num) TestErrnoGetSet(num, #num)


void TestAllErrno()
{
    TEST_ERRNO_GETSET(APL_EINTR);
    TEST_ERRNO_GETSET(APL_EAGAIN);
    TEST_ERRNO_GETSET(APL_EWOULDBLOCK);
    TEST_ERRNO_GETSET(APL_ENOSPC);
    TEST_ERRNO_GETSET(APL_E2BIG);
    TEST_ERRNO_GETSET(APL_EACCES);
    TEST_ERRNO_GETSET(APL_EADDRINUSE);
    TEST_ERRNO_GETSET(APL_EADDRNOTAVAIL);
    TEST_ERRNO_GETSET(APL_EAFNOSUPPORT);
    TEST_ERRNO_GETSET(APL_EAGAIN);
    TEST_ERRNO_GETSET(APL_EALREADY);
    TEST_ERRNO_GETSET(APL_EBADF);
    TEST_ERRNO_GETSET(APL_EBADMSG);
    TEST_ERRNO_GETSET(APL_EBUSY);
    TEST_ERRNO_GETSET(APL_ECANCELED);
    TEST_ERRNO_GETSET(APL_ECHILD);
    TEST_ERRNO_GETSET(APL_ECONNABORTED);
    TEST_ERRNO_GETSET(APL_ECONNREFUSED);
    TEST_ERRNO_GETSET(APL_ECONNRESET);
    TEST_ERRNO_GETSET(APL_EDEADLK);
    TEST_ERRNO_GETSET(APL_EDESTADDRREQ);
    TEST_ERRNO_GETSET(APL_EDOM);
    TEST_ERRNO_GETSET(APL_EDQUOT);
    TEST_ERRNO_GETSET(APL_EEXIST);
    TEST_ERRNO_GETSET(APL_EFAULT);
    TEST_ERRNO_GETSET(APL_EFBIG);
    TEST_ERRNO_GETSET(APL_EHOSTUNREACH);
    TEST_ERRNO_GETSET(APL_EIDRM);
    TEST_ERRNO_GETSET(APL_EILSEQ);
    TEST_ERRNO_GETSET(APL_EINPROGRESS);
    TEST_ERRNO_GETSET(APL_EINTR);
    TEST_ERRNO_GETSET(APL_EINVAL);
    TEST_ERRNO_GETSET(APL_EIO);
    TEST_ERRNO_GETSET(APL_EISCONN);
    TEST_ERRNO_GETSET(APL_EISDIR);
    TEST_ERRNO_GETSET(APL_ELOOP);
    TEST_ERRNO_GETSET(APL_EMFILE);
    TEST_ERRNO_GETSET(APL_EMLINK);
    TEST_ERRNO_GETSET(APL_EMSGSIZE);
    TEST_ERRNO_GETSET(APL_EMULTIHOP);
    TEST_ERRNO_GETSET(APL_ENAMETOOLONG);
    TEST_ERRNO_GETSET(APL_ENETDOWN);
    TEST_ERRNO_GETSET(APL_ENETRESET);
    TEST_ERRNO_GETSET(APL_ENETUNREACH);
    TEST_ERRNO_GETSET(APL_ENFILE);
    TEST_ERRNO_GETSET(APL_ENOBUFS);
    TEST_ERRNO_GETSET(APL_ENODATA);
    TEST_ERRNO_GETSET(APL_ENODEV);
    TEST_ERRNO_GETSET(APL_ENOENT);
    TEST_ERRNO_GETSET(APL_ENOEXEC);
    TEST_ERRNO_GETSET(APL_ENOLCK);
    TEST_ERRNO_GETSET(APL_ENOLINK);
    TEST_ERRNO_GETSET(APL_ENOMEM);
    TEST_ERRNO_GETSET(APL_ENOMSG);
    TEST_ERRNO_GETSET(APL_ENOPROTOOPT);
    TEST_ERRNO_GETSET(APL_ENOSPC);
    TEST_ERRNO_GETSET(APL_ENOSR);
    TEST_ERRNO_GETSET(APL_ENOSTR);
    TEST_ERRNO_GETSET(APL_ENOSYS);
    TEST_ERRNO_GETSET(APL_ENOTCONN);
    TEST_ERRNO_GETSET(APL_ENOTDIR);
    TEST_ERRNO_GETSET(APL_ENOTEMPTY);
    TEST_ERRNO_GETSET(APL_ENOTSOCK);
    TEST_ERRNO_GETSET(APL_ENOTSUP);
    TEST_ERRNO_GETSET(APL_ENOTTY);
    TEST_ERRNO_GETSET(APL_ENXIO);
    TEST_ERRNO_GETSET(APL_EOPNOTSUPP);
    TEST_ERRNO_GETSET(APL_EOVERFLOW);
    TEST_ERRNO_GETSET(APL_EPERM);
    TEST_ERRNO_GETSET(APL_EPIPE);
    TEST_ERRNO_GETSET(APL_EPROTO);
    TEST_ERRNO_GETSET(APL_EPROTONOSUPPORT);
    TEST_ERRNO_GETSET(APL_EPROTOTYPE);
    TEST_ERRNO_GETSET(APL_ERANGE);
    TEST_ERRNO_GETSET(APL_EROFS);
    TEST_ERRNO_GETSET(APL_ESPIPE);
    TEST_ERRNO_GETSET(APL_ESRCH);
    TEST_ERRNO_GETSET(APL_ESTALE);
    TEST_ERRNO_GETSET(APL_ETIME);
    TEST_ERRNO_GETSET(APL_ETIMEDOUT);
    TEST_ERRNO_GETSET(APL_ETXTBSY);
    TEST_ERRNO_GETSET(APL_EWOULDBLOCK);
    TEST_ERRNO_GETSET(APL_EXDEV);
}

/**
 * @brief Test suite Errno. Set and get then verify the errno in process 
 * and threads.
 */
class CTestaplerrno: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplerrno);
    CPPUNIT_TEST(testErrno);
    CPPUNIT_TEST(testErrnoThreaded);
    CPPUNIT_TEST_SUITE_END();
public:

    /**
     * Test case set and get then verify the errno in process.
     */
    void testErrno(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        TestAllErrno();

        //end environment
    }

    /**
     * Test case set and get then verify the errno in threads.
     */
    void testErrnoThreaded(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        START_THREAD_BODY(ThreadErrno1, void*, THR_NULL);
            TestAllErrno();
        END_THREAD_BODY(ThreadErrno1);
        RUN_THREAD_BODY(ThreadErrno1);

        START_THREAD_BODY(ThreadErrno2, void*, THR_NULL);
            TestAllErrno();
        END_THREAD_BODY(ThreadErrno2);
        RUN_THREAD_BODY(ThreadErrno2);

        START_THREAD_BODY(ThreadErrno3, void*, THR_NULL);
            TestAllErrno();
        END_THREAD_BODY(ThreadErrno3);
        RUN_THREAD_BODY(ThreadErrno3);

        apl_sleep(3 * APL_TIME_SEC );

        //end environment
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplerrno);

int main()
{
    RUN_ALL_TEST(__FILE__);
}