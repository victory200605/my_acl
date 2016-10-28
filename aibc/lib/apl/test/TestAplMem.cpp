#include "Test.h" 

///call by testCpy
void testMemcpy(char* apc_dest, char* apc_src, apl_size_t au_n)
{
    if ( au_n > strlen(apc_src) )
    {
        au_n = strlen(apc_src);
    }

    apl_memcpy(apc_dest,apc_src,au_n);
    apl_size_t au_i;

    for(au_i =0; au_i < au_n; ++au_i)
    {
        if ( apc_dest[au_i] != apc_src[au_i] )
        {
            CPPUNIT_FAIL("apl_memcpy fail");
        }
    }
}

///call by testSet
void testMemset(char* apc_chr, char ac_c, apl_size_t au_n)
{
    if ( au_n > strlen(apc_chr) )
    {
        au_n = strlen(apc_chr);
    }
    apl_size_t au_i;
    apl_memset(apc_chr, ac_c, au_n);
    for( au_i=0; au_i < au_n; ++au_i)
    {
        if ( apc_chr[au_i] != ac_c )
        {
            CPPUNIT_FAIL("apl_memset fail");
        }
    } 

}

///call by testMove
void testMemmove(char* apc_dest, char* apc_src, apl_size_t au_n)
{
    if ( au_n > strlen(apc_src) )
    {
        au_n = strlen(apc_src);
    }

    char  *lpc_tmp = (char *)calloc(au_n, sizeof(char));
    //backup the src in case of mem overlap
    apl_memcpy(lpc_tmp, apc_src, au_n);
    apl_memmove(apc_dest, apc_src, au_n);

    apl_size_t au_i;
    for( au_i=0; au_i < au_n; ++au_i)
    {
        if ( apc_dest[au_i] != lpc_tmp[au_i] )
        {
            CPPUNIT_FAIL("apl_memmove fail");
        }
    }

    free(lpc_tmp);
}

///call by testCcpy
void testMemccpy(char* apc_dest, char* apc_src, char ac_c, apl_size_t au_n)
{
    if ( au_n > strlen(apc_src) )
    {
        au_n = strlen(apc_src);
    }
    apl_size_t au_i;
    apl_memccpy(apc_dest, apc_src, ac_c, au_n);
    for( au_i=0; au_i < au_n; ++au_i)
    {
        if ( ac_c == apc_src[au_i] )
        {
            break;
        }
        if ( apc_dest[au_i] != apc_src[au_i] )
        {
            CPPUNIT_FAIL("apl_memccpy fail");
        }
    }
}

///call by testChr
void testMemchr(char* apc_chr, char lc_c, apl_size_t au_n)
{
    if ( au_n > strlen(apc_chr) )
    {
        au_n = strlen(apc_chr);
    }
    apl_size_t lu_i;
    char* lpc_c = (char*)apl_memchr(apc_chr, lc_c, au_n);
    char* lpc_d = NULL;
    for( lu_i=0; lu_i<au_n; ++lu_i)
    {
        if ( lc_c==apc_chr[lu_i] )
        {
            lpc_d = (char*)(apc_chr+lu_i);
            break;
        } 
    }
    if ( lpc_c != lpc_d )
    {
        CPPUNIT_FAIL("apl_memchr fail");
    }
} 

///call by testCmp
void testMemcmp(const char* apc_chr, const char* apc_chs, apl_size_t au_n)
{
    if ( au_n > strlen(apc_chr) )
    {
        au_n = strlen(apc_chr);
    }
    if ( au_n > strlen(apc_chs) )
    {
        au_n = strlen(apc_chs);
    }
    apl_size_t lu_i;
    apl_int_t  li_i = apl_memcmp(apc_chr, apc_chs, au_n);
    apl_int_t  li_j = 0;
    for( lu_i=0; lu_i<au_n; ++lu_i)
    {
        if ( apc_chr[lu_i] > apc_chs[lu_i] )
        {
            li_j = 1;
            break;
        }else if ( apc_chr[lu_i] < apc_chs[lu_i] )
        {
            li_j = -1;
            break;
        }
    }
    if ( li_i>0 && li_j>0 )
    {
        return ; 
    }
    if ( li_i==0 && li_j==0 )
    {
        return ;
    }
    if ( li_i<0 && li_j<0 )
    {
        return ;
    }
    printf("%s,%s,%d,%d\n",apc_chr,apc_chs,li_i,li_j);
    CPPUNIT_FAIL("apl_memcmp fail"); 
}

/**
 * @brief Test suite Mem
 * @brief Including five test cases: memcpy, memccpy, memchr, memcmp, memmove and memset.
 */
class CTestaplmem: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestaplmem);
    CPPUNIT_TEST(testCpy);
    CPPUNIT_TEST(testCcpy);
    CPPUNIT_TEST(testChr);
    CPPUNIT_TEST(testCmp);
    CPPUNIT_TEST(testMove);
    CPPUNIT_TEST(testSet);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}

    /** 
     * Test case memcpy.
     */
    void testCpy(void)
    {
        //start case enviornment
        char lac_src[16] = "0123456789abcde";
        char lac_des[16] = {0};
        char lac_src1[10] = "ABCDEFGHI";
        char lac_des1[10] = {0};

        //case
        testMemcpy(lac_des, lac_src, strlen(lac_src) );
        testMemcpy(lac_des1, lac_src1, strlen(lac_src1) );

        //end enviornment
    }

    /** 
     * Test case memccpy.
     */
    void testCcpy(void)
    {
        //start case enviornment
        char lac_src[20] = "asdvasdfe";
        char lac_srd[20] = {0}; 

        //case
        testMemccpy(lac_srd, lac_src, 'v', 5);
        testMemccpy(lac_srd, lac_src,'z',strlen(lac_src) );

        //end enviornment
    }

    /** 
     * Test case memchr
     */
    void testChr(void)
    {
        //start case enviornment
        char lac_src[]="vsefaefse";

        //case
        testMemchr(lac_src,'v',3);
        testMemchr(lac_src,'f',strlen(lac_src) );
        testMemchr(lac_src,'y',strlen(lac_src) );

        //end enviornment
    } 

    /** 
     * Test case memcmp
     */
    void testCmp(void)
    {
        //start case enviornment
        char lac_src[]="yiew1231v";
        char lac_srd[]="aswese";
        char lac_sre[]="yiew1231v";
        char lac_srf[]="zefs";

        //case 
        testMemcmp(lac_src, lac_srd, sizeof(lac_srd) );
        testMemcmp(lac_src, lac_sre, sizeof(lac_sre) );
        testMemcmp(lac_src, lac_srf, sizeof(lac_srf) );

        //end enviornment
    }
   
    /** 
     * Test case memmove
     */
    void testMove(void)
    {
        //start case enviornment
        char lac_src[20]="123456789";
        char lac_dst[20] = {0};
        char *lpc_dst = NULL;

        //case 
        testMemmove(lac_dst, lac_src, strlen(lac_src));
        //mem overlap test
        lpc_dst = lac_src + 4; 
        testMemmove(lpc_dst, lac_src, strlen(lac_src));

        //end enviornment
    }

    /** 
     * Test case memset
     */
    void testSet(void)
    {
        //start case enviornment
        char lac_src[]="asdvasdfe";

        //case
        testMemset(lac_src, 'a', 3);
        testMemset(lac_src, 'd', strlen(lac_src) );
        testMemset(lac_src, 'j', strlen(lac_src) );

        //end enviornment
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestaplmem);

int main(int argc, char *argv[])
{
    RUN_ALL_TEST(__FILE__);
}

