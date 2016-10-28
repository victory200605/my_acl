#include "Test.h"

/* In HPUX, 887M is the max mmap when the flag is MAP_PRIVATE.
 * When linking it with "-N" option, it will enlarge to 1911M */ 
#define _MAXLEN _MBYTE * 800
#define _MGBYTE (1024*1024*1024) 
#define _MBYTE (1024*1024) 
#define DATA_FILENAME "_file.data"

///call by testMman
void testmp(void* apv_mmap, apl_size_t ai_len, apl_uint_t  au_prot, apl_uint_t au_flags,  
              apl_int_t  ai_fildes, apl_off_t  ai64_offset)
{
    void* lpv_mmap = APL_NULL;
    lpv_mmap = apl_mmap( lpv_mmap, ai_len, au_prot, au_flags, ai_fildes, ai64_offset);
    if ( lpv_mmap == APL_NULL )
    {
        PrintErrMsg("apl_mmap",-1);
        return ;
    }
    apl_int_t li_ir = apl_msync(lpv_mmap, ai_len, APL_MS_SYNC);
    PrintErrMsg("apl_msync", li_ir);
    li_ir = apl_munmap(lpv_mmap, ai_len) ;
    PrintErrMsg("apl_munmap", li_ir);

/*        void* lpv_lock = malloc( 8*1024 ) ;
    li_ir = apl_mlock(lpv_lock, 8*1024) ;
    PrintErrMsg("apl_mlock", li_ir) ;
    li_ir = apl_munlock(lpv_lock, 8*1024) ;
    PrintErrMsg("apl_munlock", li_ir) ;
    li_ir = apl_mlockall(APL_MCL_CURRENT) ; 
    PrintErrMsg("apl_mlockall", li_ir);
    li_ir = apl_munlockall() ;
    PrintErrMsg("apl_munlockall", li_ir) ;
    free( lpv_lock ); */
}

/**
 * @brief Test suite Mman
 */
class CTestaplmman: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplmman);
    CPPUNIT_TEST(testMman);
    CPPUNIT_TEST(testMmanLFPriv);
    CPPUNIT_TEST(testMmanLFShar);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}

    /** 
     * Test case mman involved mmap, munmap and msync.
     */
    void testMman(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_ifd  = apl_open("testMan", APL_O_RDWR |APL_O_CREAT|APL_O_TRUNC, 0755);

        //case 
        testmp(NULL, 1024, APL_PROT_READ | APL_PROT_WRITE, APL_MAP_SHARED, li_ifd, 0);
        if ( apl_lseek(li_ifd, 15, APL_SEEK_SET) != 15 )
        {
            CPPUNIT_FAIL("fail");
        }

        //end environment
        apl_close(li_ifd);
    }

    /** 
     * Test case testManLargeFile, mmap largefile 
     */
    void testMmanLFPriv(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_ifd;
        apl_off_t li_len;
        void* lpv_mmap = APL_NULL;

        li_ifd = apl_open(DATA_FILENAME, APL_O_RDWR |APL_O_CREAT|APL_O_TRUNC, 0755);
        CPPUNIT_ASSERT(li_ifd > 0);

        //case
        for (li_len=10; li_len <=_MAXLEN; li_len += _MBYTE)
        {
            lpv_mmap = APL_NULL;

            apl_errprintf("creating file size %"APL_PRIu64"...", li_len);

            RUN_AND_CHECK_RET_INT(apl_ftruncate, (li_ifd, li_len));

            lpv_mmap = apl_mmap(lpv_mmap, li_len, APL_PROT_READ|APL_PROT_WRITE, APL_MAP_PRIVATE, li_ifd, 0);

            CPPUNIT_ASSERT(lpv_mmap != APL_MAP_FAILED);

            apl_errprintf("mapped address: %p\n", lpv_mmap);

            RUN_AND_CHECK_RET_INT(apl_munmap, (lpv_mmap, li_len));
        }

        //end environment
        apl_close(li_ifd);
        apl_unlink(DATA_FILENAME);
    }

    /** 
     * Test case testManLargeFile, mmap largefile 
     */
    void testMmanLFShar(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        apl_int_t li_ifd;
        apl_off_t li_len;
        void* lpv_mmap = APL_NULL;

        li_ifd = apl_open(DATA_FILENAME, APL_O_RDWR |APL_O_CREAT|APL_O_TRUNC, 0755);
        CPPUNIT_ASSERT(li_ifd > 0);

        //case
        for (li_len=10; li_len <=_MAXLEN; li_len += _MBYTE)
        {
            lpv_mmap = APL_NULL;

            apl_errprintf("creating file size %"APL_PRIu64"...", li_len);

            RUN_AND_CHECK_RET_INT(apl_ftruncate, (li_ifd, li_len));

            lpv_mmap = apl_mmap(lpv_mmap, li_len, APL_PROT_READ|APL_PROT_WRITE, APL_MAP_SHARED, li_ifd, 0);

            CPPUNIT_ASSERT(lpv_mmap != APL_MAP_FAILED);

            apl_errprintf("mapped address: %p\n", lpv_mmap);

            RUN_AND_CHECK_RET_INT(apl_munmap, (lpv_mmap, li_len));
        }

        //end environment
        apl_close(li_ifd);
        apl_unlink(DATA_FILENAME);
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplmman);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
