#include "Test.h"
#include "acl/MemMap.h"
#include "acl/File.h"

using namespace acl;


class CTestAclMemMap: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclMemMap);
    CPPUNIT_TEST( testCommon );
    CPPUNIT_TEST( testLockMap );
    CPPUNIT_TEST( testRWMap );
    CPPUNIT_TEST( testMapFd );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) 
    {
        system("cat Makefile > TestMap");
        cuLen= 1000;
        
        if ( coMem.Map( "TestMap", APL_PROT_READ | APL_PROT_WRITE, APL_MAP_SHARED, cuLen, 0) != 0 )
        {
            CPPUNIT_FAIL(" map fail");
        }
    }
    void tearDown(void)
    {
        system("rm -f TestMap");
    }
    void testCommon(void)
    {
        PRINT_TITLE_2(__func__);
        //start environment

        //case
        apl_int_t lir= -1;
        lir=  coMem.Sync(); 
        CPPUNIT_ASSERT_EQUAL_MESSAGE(" sync fail", (apl_int_t)0, lir);
        CPPUNIT_ASSERT_MESSAGE("isMapped fail", coMem.IsMaped() );
        CPPUNIT_ASSERT_MESSAGE("Get Addr", coMem.GetAddr() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GetSize", cuLen, coMem.GetSize() );
        if ( coMem.Unmap() != 0 )
        {
            CPPUNIT_FAIL("Unmap fail");
        }

        //end environment
    }
    void testLockMap(void)
    {
        PRINT_TITLE_2(__func__);
        /*if ( coMem.MLock() != 0 )
        {
            CPPUNIT_FAIL(" MLock fail");
        }
        if ( coMem.MUnlock() != 0 )
        {
            CPPUNIT_FAIL(" MUnlock");
        }*/
        coMem.Unmap();
    }
    void testRWMap()
    {
        PRINT_TITLE_2(__func__);
        //start environment

        //case
        char* lpcChr  = APL_NULL;
        char* lpcRead = APL_NULL;
        apl_size_t luN = 0;
        CFile loFile;
        lpcChr = (char*)coMem.GetAddr();
        for( ; luN< cuLen; ++luN)
        {
           *lpcChr++ = 'a';
        }
        if ( luN != cuLen )
        {
            CPPUNIT_FAIL(" map length fail");
        }
        if ( coMem.Unmap() != 0 )
        {
            CPPUNIT_FAIL("Unmap fail");
        }
        if ( loFile.Open("TestMap", O_RDWR) != 0 )
        {
            return ;
        }
        lpcRead = new char[cuLen+1];
        readN( loFile.GetHandle(), lpcRead, cuLen);
        for(luN= 0; luN< cuLen; ++luN)
        {
            if ( *lpcRead++ != 'a' )
            {
                CPPUNIT_FAIL("map write fail");
            }
        }

        //end environment
    }
    apl_ssize_t readN(apl_handle_t aiHandle, void* apcBuf, apl_size_t auLen)
    {
        apl_size_t  luRecv = 0;
        char*       lpcBuf = (char*)apcBuf;
        while (luRecv < auLen)
        {
            apl_ssize_t liRetSz;
            liRetSz = read(aiHandle, lpcBuf + luRecv, auLen - luRecv);
            if ( liRetSz < 0 )
            {
                if (APL_EINTR == apl_get_errno())
                {
                    continue;
                }
                if (APL_EWOULDBLOCK == apl_get_errno())
                {
                    break;
                }
                return APL_INT_C(-1);
            }
            if (liRetSz == 0)
            {
                break;
            }
            luRecv += liRetSz;
        }
        assert(luRecv >= 0 && luRecv <= auLen);
        return luRecv;    
    }

    void testMapFd(void)
    {
        //start environment

        //case
        coMem.Unmap();
        CMemMap loMemMap;
        CFile loFile;
        if (loFile.Open("TestMap", O_RDWR) != 0)
        {
            return ;
        }
        if ( loMemMap.Map( loFile.GetHandle(), APL_PROT_READ | APL_PROT_WRITE, APL_MAP_SHARED, cuLen, 0) != 0 )
        {
            CPPUNIT_FAIL(" map fail");
        }
        if ( loFile.Seek(15, APL_SEEK_SET) != 15 )
        {
            CPPUNIT_FAIL("memMap fail");
        }
        loFile.Close();
        
        //end environment
    }
private:
    CMemMap   coMem;
    apl_size_t cuLen; 
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclMemMap);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
