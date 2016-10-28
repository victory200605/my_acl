#include "Test.h"
#include "acl/StrAlgo.h"
#include <string>
using std::string;
using namespace acl::stralgo;

#define TEST_STRALGO_TRIM_STR(func,tstr,rstr) \
    do { \
       func (tstr); \
       CPPUNIT_ASSERT_MESSAGE("fail", apl_strcmp(tstr.c_str(), rstr.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_TRIMCOPY_STR(func, tstr, rstr) \
    do { \
       string test_res; \
       func (tstr, test_res); \
       CPPUNIT_ASSERT_MESSAGE("fail", apl_strcmp(rstr.c_str(), test_res.c_str()) == 0); \
      }while(0) 

#define TEST_STRALGO_TRIM_CHR(func, tstr, rstr) \
    do { \
       func (tstr); \
       CPPUNIT_ASSERT_MESSAGE("fail", apl_strcmp(tstr, rstr) == 0); \
      }while(0)

#define TEST_STRALGO_TRIMCOPY_CHR(func, tstr, rstr, estr) \
    do { \
       func (tstr, rstr, sizeof(rstr)); \
       CPPUNIT_ASSERT_MESSAGE("fail", apl_strncmp(estr, rstr, sizeof(rstr)) == 0); \
      }while(0)

#define TEST_STRALGO_ERASERANGE_STR(tstr, rstr, pos, len) \
    do { \
       EraseRange(tstr, (apl_size_t)pos, (apl_size_t)len); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(tstr.c_str(), rstr.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_ERASERANGE_CHR(tstr, rstr, pos, len) \
    do { \
       EraseRange(tstr, pos, len); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(tstr, rstr) == 0); \
      }while(0)

#define TEST_STRALGO_ERASERANGECOPY_STR(tstr, rstr, pos, len) \
    do { \
       string tmp; \
       EraseRangeCopy(tstr, pos, len, tmp); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(tmp.c_str(), rstr.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_ERASERANGECOPY_CHR(tstr, rstr, pos, len) \
    do { \
       char tmp[20]; \
       EraseRangeCopy(tstr, pos, len, tmp, sizeof(tmp)); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(tmp, rstr) == 0); \
      }while(0)

#define TEST_STRALGO_ERASESUB_STR(func, tstr, substr, rstr) \
    do { \
       func (tstr, substr); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(tstr.c_str(), rstr.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_ERASESUB_CHR(func, tstr, substr, rstr) \
    do { \
       func (tstr, substr); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(tstr, rstr) == 0); \
      }while(0)

#define TEST_STRALGO_ERASESUBCOPY_STR(func, tstr, substr, rstr) \
    do { \
       string temp; \
       func (tstr, substr, temp); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(temp.c_str(), rstr.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_ERASESUBCOPY_CHR(func, tstr, substr, rstr) \
    do { \
       char temp[50]; \
       func (tstr, substr, temp, sizeof(temp)); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(temp, rstr) == 0); \
      }while(0)

#define TEST_STRALGO_ERASESUBN_STR(tstr, sub, n, rstr) \
    do { \
       EraseNth(tstr, sub, n); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(tstr.c_str(), rstr.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_ERASESUBN_CHR(tstr, sub, n, rstr) \
    do { \
       EraseNth(tstr, sub, n); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(tstr, rstr) == 0); \
      }while(0)

#define TEST_STRALGO_ERASESUBNCOPY_STR(tstr, sub, n, rstr) \
    do { \
       string temp; \
       EraseNthCopy(tstr, sub, n, temp); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(temp.c_str(), rstr.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_ERASESUBNCOPY_CHR(tstr, sub, n, rstr) \
    do { \
       char temp[50]; \
       EraseNthCopy(tstr, sub, n, temp, sizeof(temp)); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(temp, rstr) == 0); \
      }while(0)

#define TEST_STRALGO_ERASEEND_STR(func, tstr, n, rstr) \
    do { \
       func(tstr, n); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(tstr.c_str(), rstr.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_ERASEEND_CHR(func, tstr, n, rstr) \
    do { \
       func(tstr, n); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(tstr, rstr) == 0); \
      }while(0)

#define TEST_STRALGO_ERASEENDCOPY_STR(func, tstr, n, rstr) \
    do { \
       string temp; \
       func(tstr, n, temp); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(temp.c_str(), rstr.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_ERASEENDCOPY_CHR(func, tstr, n, rstr) \
    do { \
       char temp[50]; \
       func(tstr, n, temp, sizeof(temp)); \
       CPPUNIT_ASSERT_MESSAGE("erase fail", apl_strcmp(temp, rstr) == 0); \
      }while(0)

#define TEST_STRALGO_FIND(func, tstr, sub, position) \
    do { \
       apl_int_t temp; \
       temp = func(tstr, sub); \
       CPPUNIT_ASSERT_MESSAGE("find fail", temp == position); \
      }while(0)

#define TEST_STRALGO_FINDN(tstr, sub, n, position) \
    do { \
       apl_int_t temp; \
       temp = FindNth(tstr, sub, n); \
       CPPUNIT_ASSERT_MESSAGE("find fail", temp == position); \
      }while(0)

#define TEST_STRALGO_REPLACERANGE_STR(tstr, position, len, sub, res) \
    do { \
       string temp; \
       ReplaceRangeCopy(tstr, position, len, sub, temp); \
       CPPUNIT_ASSERT_MESSAGE("replace fail", apl_strcmp(temp.c_str(), res.c_str()) ==0); \
      }while(0)

#define TEST_STRALGO_REPLACERANGE_CHR(tstr, position, len, sub, res) \
    do { \
       char temp[20]; \
       ReplaceRangeCopy(tstr, position, len, sub, temp, sizeof(temp)); \
       CPPUNIT_ASSERT_MESSAGE("replace fail", apl_strcmp(temp, res) == 0); \
      }while(0)

#define TEST_STRALGO_REPLACE_STR(func, tstr, ssub, rsub, res) \
    do { \
       string temp; \
       func(tstr,ssub, rsub, temp); \
       CPPUNIT_ASSERT_MESSAGE("replace fail", apl_strcmp(temp.c_str(), res.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_REPLACE_CHR(func, tstr, ssub, rsub, res) \
    do { \
       char temp[20]; \
       func(tstr, ssub, rsub, temp, sizeof(temp)); \
       CPPUNIT_ASSERT_MESSAGE("replace fail", apl_strcmp(temp, res) == 0); \
      }while(0)

#define TEST_STRALGO_REPLACEN_STR(tstr, ssub, n, rsub, res) \
    do { \
       string temp; \
       ReplaceNthCopy(tstr, ssub, n, rsub, temp); \
       CPPUNIT_ASSERT_MESSAGE("replace fail", apl_strcmp(temp.c_str(), res.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_REPLACEN_CHR(tstr, ssub, n, rsub, res) \
    do { \
       char temp[20]; \
       ReplaceNthCopy(tstr, ssub, n, rsub, temp, sizeof(temp)); \
       CPPUNIT_ASSERT_MESSAGE("replace fail", apl_strcmp(temp, res) == 0); \
      }while(0)

#define TEST_STRALGO_COMPARE(func, str1, str2, res) \
    do { \
       bool temp; \
       temp = func(str1, str2); \
       CPPUNIT_ASSERT_MESSAGE("compare fail", temp == res); \
      }while(0)

#define TEST_STRALGO_TRIMIF_STR(tstr, fpred, res) \
    do { \
       TrimIf(tstr, fpred); \
       CPPUNIT_ASSERT_MESSAGE("trimif fail", apl_strcmp(tstr.c_str(), res.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_TRIMIF_CHR(tstr, fpred, res) \
    do { \
       TrimIf(tstr, fpred); \
       CPPUNIT_ASSERT_MESSAGE("trimif fail", apl_strcmp(tstr, res) == 0); \
      }while(0)

#define TEST_STRALGO_TRIMIFCOPY_STR(tstr, fpred, res) \
    do { \
       string temp; \
       TrimIfCopy(tstr, fpred, temp); \
       CPPUNIT_ASSERT_MESSAGE("trimif fail", apl_strcmp(temp.c_str(), res.c_str()) == 0); \
      }while(0)

#define TEST_STRALGO_TRIMIFCOPY_CHR(tstr, fpred, res) \
    do { \
       char temp[20]; \
       TrimIfCopy(tstr, fpred, temp, sizeof(temp)); \
       CPPUNIT_ASSERT_MESSAGE("trimif fail", apl_strcmp(temp, res) == 0); \
      }while(0)

#define TEST_STRALGO_ALL(tstr, fpred, res) \
    do { \
       bool temp; \
       temp = All(tstr, fpred); \
       CPPUNIT_ASSERT_MESSAGE("fail", temp == res); \
      }while(0)

bool checkA(char c) { return c == 'a';}
 
class CTestAclStralgo : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclStralgo);
    CPPUNIT_TEST(testUpper);
    CPPUNIT_TEST(testUpperCopy);
    CPPUNIT_TEST(testLower);
    CPPUNIT_TEST(testLowerCopy);
    CPPUNIT_TEST(testTrim);
    CPPUNIT_TEST(testTrimLeft);
    CPPUNIT_TEST(testTrimRight);
    CPPUNIT_TEST(testTrimCopy);
    CPPUNIT_TEST(testTrimLeftCopy);
    CPPUNIT_TEST(testTrimRightCopy);   
    CPPUNIT_TEST(testErase);
    CPPUNIT_TEST(testEraseCopy);
    CPPUNIT_TEST(testEraseSubFirst);
    CPPUNIT_TEST(testEraseSubLast);
    CPPUNIT_TEST(testEraseSubFirstCopy);
    CPPUNIT_TEST(testEraseSubLastCopy);
    CPPUNIT_TEST(testEraseSubN);
    CPPUNIT_TEST(testEraseSubNCopy);
    CPPUNIT_TEST(testEraseHead);
    CPPUNIT_TEST(testEraseTail);
    CPPUNIT_TEST(testEraseHeadCopy);
    CPPUNIT_TEST(testEraseTailCopy);
    CPPUNIT_TEST(testFind);
    CPPUNIT_TEST(testFindFirst);
    CPPUNIT_TEST(testFindLast);
    CPPUNIT_TEST(testFindN);
    CPPUNIT_TEST(testReplaceRange);
    CPPUNIT_TEST(testReplace);
    CPPUNIT_TEST(testReplaceFirst);
    CPPUNIT_TEST(testReplaceLast);
    CPPUNIT_TEST(testReplaceN);
    CPPUNIT_TEST(testStartWith);
    CPPUNIT_TEST(testEndWith);
    CPPUNIT_TEST(testContain);
    CPPUNIT_TEST(testEqual);
    CPPUNIT_TEST(testTrimIf);
    CPPUNIT_TEST(testTrimIfCopy);
    CPPUNIT_TEST(testAll);
    CPPUNIT_TEST(testKMPFind);
    CPPUNIT_TEST_SUITE_END();
private:
public:
    void setUp(void) {}
    void tearDown(void) {}

    void testUpper(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str = "A Test01";
        string res_str = "A TEST01";
        char test_str1[] = "A Test01";

        ToUpper(test_str);
        CPPUNIT_ASSERT_MESSAGE("upper fail", apl_strcmp(test_str.c_str(), res_str.c_str()) == 0);
        ToUpper(test_str1);
        CPPUNIT_ASSERT_MESSAGE("upper fail", apl_strcmp(test_str1, res_str.c_str()) == 0);
    }

    void testUpperCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str = "A Test01";
        string test_res;
        string res_str = "A TEST01";
        char res_buffer[10];
        char s_buffer[5];
        string s_res_str = "A TE";
        
        ToUpperCopy(test_str, test_res);
        CPPUNIT_ASSERT_MESSAGE("UpperCopy fail", apl_strcmp(test_res.c_str(), res_str.c_str()) == 0);
        ToUpperCopy(test_str.c_str(), res_buffer, 10);
        CPPUNIT_ASSERT_MESSAGE("UpperCopy fail", apl_strcmp(res_str.c_str(), res_buffer) == 0);
        ToUpperCopy(test_str.c_str(), s_buffer, 5);
        CPPUNIT_ASSERT_MESSAGE("UpperCopy fail", apl_strncmp(s_res_str.c_str(), s_buffer, 5) == 0); 
    }
    void testLower(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str = "A Test01";
        string res_str = "a test01";
        char test_str1[] = "A Test01";
    
        ToLower(test_str);
        CPPUNIT_ASSERT_MESSAGE("Lower fail", apl_strcmp(res_str.c_str(), test_str.c_str()) == 0);
        ToLower(test_str1);
        CPPUNIT_ASSERT_MESSAGE("Lower fail", apl_strcmp(res_str.c_str(), test_str1) == 0);
    }
    
    void testLowerCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str = "A Test01";
        string test_res;
        string res_str = "a test01";
        char res_buffer[10];
        char s_buffer[5];
        string s_res_str = "a te";

        ToLowerCopy(test_str, test_res);
        CPPUNIT_ASSERT_MESSAGE("LowerCopy fail", apl_strcmp(res_str.c_str(), test_res.c_str()) == 0);
        ToLowerCopy(test_str.c_str(), res_buffer, 10);
        CPPUNIT_ASSERT_MESSAGE("LowerCopy fail", apl_strcmp(res_str.c_str(), res_buffer) == 0);
        ToLowerCopy(test_str.c_str(), s_buffer, 5);
        CPPUNIT_ASSERT_MESSAGE("LowerCopy fail", apl_strncmp(s_res_str.c_str(), s_buffer, 5) == 0);
    } 

    void testTrim(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str = "  A Test01  ";
        string res_str = "A Test01";
        char test_chr[] = "  A Test01  ";
        TEST_STRALGO_TRIM_STR(Trim, test_str, res_str);
        TEST_STRALGO_TRIM_CHR(Trim, test_chr, res_str.c_str());
    }

    void testTrimLeft(void)
    {
        PRINT_TITLE_2(__func__);

        string test_str1 = "  A Test01  ";
        string res_str1 = "A Test01  ";
        char test_chr1[] = "  A Test01  ";
        TEST_STRALGO_TRIM_STR(TrimLeft, test_str1, res_str1);
        TEST_STRALGO_TRIM_CHR(TrimLeft, test_chr1, res_str1.c_str());
    }

    void testTrimRight(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str2 = "  A Test01  ";
        string res_str2 = "  A Test01";
        char test_chr2[] = "  A Test01  ";
        TEST_STRALGO_TRIM_STR(TrimRight, test_str2, res_str2);
        TEST_STRALGO_TRIM_CHR(TrimRight, test_chr2, res_str2.c_str());
    }

    void testTrimCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str = "  A Test01  ";
        char res_buffer[15];

        string res_str = "A Test01";
        TEST_STRALGO_TRIMCOPY_STR(TrimCopy, test_str, res_str);
        TEST_STRALGO_TRIMCOPY_CHR(TrimCopy, test_str.c_str(), res_buffer, res_str.c_str());
    }
    
    void testTrimLeftCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str = "  A Test01  ";
        char res_buffer[15];

        string res_str1 = "A Test01  ";
        TEST_STRALGO_TRIMCOPY_STR(TrimLeftCopy, test_str, res_str1);
        TEST_STRALGO_TRIMCOPY_CHR(TrimLeftCopy, test_str.c_str(), res_buffer, res_str1.c_str());
    }

    void testTrimRightCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str = "  A Test01  ";
        char res_buffer[15];

        string res_str2 = "  A Test01";
        TEST_STRALGO_TRIMCOPY_STR(TrimRightCopy, test_str, res_str2);
        TEST_STRALGO_TRIMCOPY_CHR(TrimRightCopy, test_str.c_str(), res_buffer, res_str2.c_str());
    }
 
    void testErase(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str = "A Test01";
        string res_str = "Ast01";
        char test_chr[] = "A Test01";
        TEST_STRALGO_ERASERANGE_STR(test_str, res_str, 1, 3);
        TEST_STRALGO_ERASERANGE_CHR(test_chr, res_str.c_str(), 1, 3);

        string test_str1 = "A Test01";
        string res_str1 = "est01";
        char test_chr1[] = "A Test01";
        TEST_STRALGO_ERASERANGE_STR(test_str1, res_str1, 0, 3);
        TEST_STRALGO_ERASERANGE_CHR(test_chr1, res_str1.c_str(), 0, 3);

        string test_str2 = "A Test01";
        string res_str2 = "A Test";
        char test_chr2[] = "A Test01";
        TEST_STRALGO_ERASERANGE_STR(test_str2, res_str2, 6, 2);
        TEST_STRALGO_ERASERANGE_CHR(test_chr2, res_str2.c_str(), 6, 2);

        string test_str3 = "A Test01";
        string res_str3 = "A Test";
        char test_chr3[] = "A Test01";
        TEST_STRALGO_ERASERANGE_STR(test_str3, res_str3, 6, 4);
        TEST_STRALGO_ERASERANGE_CHR(test_chr3, res_str3.c_str(), 6, 4);
    }
    
    void testEraseCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str = "A Test01";
        string res_str = "Ast01";
        char test_chr[] = "A Test01";
        TEST_STRALGO_ERASERANGECOPY_STR(test_str, res_str, 1, 3);
        TEST_STRALGO_ERASERANGECOPY_CHR(test_chr, res_str.c_str(), 1, 3);

        string test_str1 = "A Test01";
        string res_str1 = "est01";
        char test_chr1[] = "A Test01";
        TEST_STRALGO_ERASERANGECOPY_STR(test_str1, res_str1, 0, 3);
        TEST_STRALGO_ERASERANGECOPY_CHR(test_chr1, res_str1.c_str(), 0, 3);

        string test_str2 = "A Test01";
        string res_str2 = "A Test";
        char test_chr2[] = "A Test01";
        TEST_STRALGO_ERASERANGECOPY_STR(test_str2, res_str2, 6, 2);
        TEST_STRALGO_ERASERANGECOPY_CHR(test_chr2, res_str2.c_str(), 6, 2);

        string test_str3 = "A Test01";
        string res_str3 = "A Test";
        char test_chr3[] = "A Test01";
        TEST_STRALGO_ERASERANGECOPY_STR(test_str3, res_str3, 6, 4);
        TEST_STRALGO_ERASERANGECOPY_CHR(test_chr3, res_str3.c_str(), 6, 4);
    }

    void testEraseSubFirst(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string test_sub1 = "ab";
        string res_str1 = "aab abaab";
        char test_chr1[] = "abaab abaab";
        TEST_STRALGO_ERASESUB_STR(EraseFirst, test_str1, test_sub1, res_str1);
        TEST_STRALGO_ERASESUB_CHR(EraseFirst, test_chr1, test_sub1.c_str(), res_str1.c_str());

        string test_str2 = "abaab abaab";
        string test_sub2 = "ba";
        string res_str2 = "aab abaab";
        char test_chr2[] = "abaab abaab"; 
        TEST_STRALGO_ERASESUB_STR(EraseFirst, test_str2, test_sub2, res_str2);
        TEST_STRALGO_ERASESUB_CHR(EraseFirst, test_chr2, test_sub2.c_str(), res_str2.c_str());

        string test_str3 = "abaab abaab";
        string test_sub3 = " abaab";
        string res_str3 = "abaab";
        char test_chr3[] = "abaab abaab";
        TEST_STRALGO_ERASESUB_STR(EraseFirst, test_str3, test_sub3, res_str3);
        TEST_STRALGO_ERASESUB_CHR(EraseFirst, test_chr3, test_sub3.c_str(), res_str3.c_str());
    }
    
    void testEraseSubLast(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str4 = "abaab abaab";
        string test_sub4 = "ab";
        string res_str4 = "abaab aba";
        char test_chr4[] = "abaab abaab";
        TEST_STRALGO_ERASESUB_STR(EraseLast, test_str4, test_sub4, res_str4);
        TEST_STRALGO_ERASESUB_CHR(EraseLast, test_chr4, test_sub4.c_str(), res_str4.c_str());

        string test_str5 = "abaab abaab";
        string test_sub5 = "ba";
        string res_str5 = "abaab aab";
        char test_chr5[] = "abaab abaab";
        TEST_STRALGO_ERASESUB_STR(EraseLast, test_str5, test_sub5, res_str5);
        TEST_STRALGO_ERASESUB_CHR(EraseLast, test_chr5, test_sub5.c_str(), res_str5.c_str());
        
        string test_str6 = "abaab abaab";
        string test_sub6 = "abaab ";
        string res_str6 = "abaab";
        char test_chr6[] = "abaab abaab";
        TEST_STRALGO_ERASESUB_STR(EraseLast, test_str6, test_sub6, res_str6);
        TEST_STRALGO_ERASESUB_CHR(EraseLast, test_chr6, test_sub6.c_str(), res_str6.c_str());
    }
 
    void testEraseSubFirstCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string test_sub1 = "ab";
        string res_str1 = "aab abaab";
        char test_chr1[] = "abaab abaab";
        TEST_STRALGO_ERASESUBCOPY_STR(EraseFirstCopy, test_str1, test_sub1, res_str1);
        TEST_STRALGO_ERASESUBCOPY_CHR(EraseFirstCopy, test_chr1, test_sub1.c_str(), res_str1.c_str());

        string test_str2 = "abaab abaab";
        string test_sub2 = "ba";
        string res_str2 = "aab abaab";
        char test_chr2[] = "abaab abaab"; 
        TEST_STRALGO_ERASESUBCOPY_STR(EraseFirstCopy, test_str2, test_sub2, res_str2);
        TEST_STRALGO_ERASESUBCOPY_CHR(EraseFirstCopy, test_chr2, test_sub2.c_str(), res_str2.c_str());

        string test_str3 = "abaab abaab";
        string test_sub3 = " abaab";
        string res_str3 = "abaab";
        char test_chr3[] = "abaab abaab";
        TEST_STRALGO_ERASESUBCOPY_STR(EraseFirstCopy, test_str3, test_sub3, res_str3);
        TEST_STRALGO_ERASESUBCOPY_CHR(EraseFirstCopy, test_chr3, test_sub3.c_str(), res_str3.c_str());
    }

    void testEraseSubLastCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str4 = "abaab abaab";
        string test_sub4 = "ab";
        string res_str4 = "abaab aba";
        char test_chr4[] = "abaab abaab";
        TEST_STRALGO_ERASESUBCOPY_STR(EraseLastCopy, test_str4, test_sub4, res_str4);
        TEST_STRALGO_ERASESUBCOPY_CHR(EraseLastCopy, test_chr4, test_sub4.c_str(), res_str4.c_str());

        string test_str5 = "abaab abaab";
        string test_sub5 = "ba";
        string res_str5 = "abaab aab";
        char test_chr5[] = "abaab abaab";
        TEST_STRALGO_ERASESUBCOPY_STR(EraseLastCopy, test_str5, test_sub5, res_str5);
        TEST_STRALGO_ERASESUBCOPY_CHR(EraseLastCopy, test_chr5, test_sub5.c_str(), res_str5.c_str());

        string test_str6 = "abaab abaab";
        string test_sub6 = "abaab ";
        string res_str6 = "abaab";
        char test_chr6[] = "abaab abaab";
        TEST_STRALGO_ERASESUBCOPY_STR(EraseLastCopy, test_str6, test_sub6, res_str6);
        TEST_STRALGO_ERASESUBCOPY_CHR(EraseLastCopy, test_chr6, test_sub6.c_str(), res_str6.c_str());
    }

    void testEraseSubN(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string test_sub1 = "ab";
        string res_str1 = "aab abaab";
        char test_chr1[] = "abaab abaab";
        TEST_STRALGO_ERASESUBN_STR(test_str1, test_sub1, 1, res_str1);
        TEST_STRALGO_ERASESUBN_CHR(test_chr1, test_sub1.c_str(), 1, res_str1.c_str());

        string test_str2 = "abaab abaab";
        string test_sub2 = "ab";
        string res_str2 = "abaab aab";
        char test_chr2[] = "abaab abaab";
        TEST_STRALGO_ERASESUBN_STR(test_str2, test_sub2, 3, res_str2);
        TEST_STRALGO_ERASESUBN_CHR(test_chr2, test_sub2.c_str(), 3, res_str2.c_str());

        string test_str3 = "abaab abaab";
        string test_sub3 = "ab";
        string res_str3 = "abaab aba";
        char test_chr3[] = "abaab abaab";
        TEST_STRALGO_ERASESUBN_STR(test_str3, test_sub3, 4, res_str3);
        TEST_STRALGO_ERASESUBN_CHR(test_chr3, test_sub3.c_str(), 4, res_str3.c_str());

        string test_str4 = "abaab abaab";
        string test_sub4 = "ab";
        string res_str4 = "abaab abaab";
        char test_chr4[] = "abaab abaab";
        TEST_STRALGO_ERASESUBN_STR(test_str4, test_sub4, 5, res_str4);
        TEST_STRALGO_ERASESUBN_CHR(test_chr4, test_sub4.c_str(), 5, res_str4.c_str());
    }
   
    void testEraseSubNCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string test_sub1 = "ab";
        string res_str1 = "aab abaab";
        char test_chr1[] = "abaab abaab";
        TEST_STRALGO_ERASESUBNCOPY_STR(test_str1, test_sub1, 1, res_str1);
        TEST_STRALGO_ERASESUBNCOPY_CHR(test_chr1, test_sub1.c_str(), 1, res_str1.c_str());

        string test_str2 = "abaab abaab";
        string test_sub2 = "ab";
        string res_str2 = "abaab aab";
        char test_chr2[] = "abaab abaab";
        TEST_STRALGO_ERASESUBNCOPY_STR(test_str2, test_sub2, 3, res_str2);
        TEST_STRALGO_ERASESUBNCOPY_CHR(test_chr2, test_sub2.c_str(), 3, res_str2.c_str());

        string test_str3 = "abaab abaab";
        string test_sub3 = "ab";
        string res_str3 = "abaab aba";
        char test_chr3[] = "abaab abaab";
        TEST_STRALGO_ERASESUBNCOPY_STR(test_str3, test_sub3, 4, res_str3);
        TEST_STRALGO_ERASESUBNCOPY_CHR(test_chr3, test_sub3.c_str(), 4, res_str3.c_str());

        string test_str4 = "abaab abaab";
        string test_sub4 = "ab";
        string res_str4 = "abaab abaab";
        char test_chr4[] = "abaab abaab";
        TEST_STRALGO_ERASESUBNCOPY_STR(test_str4, test_sub4, 5, res_str4);
        TEST_STRALGO_ERASESUBNCOPY_CHR(test_chr4, test_sub4.c_str(), 5, res_str4.c_str());
    }
 
    void testEraseHead(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string res_str1 = " abaab";
        char test_chr1[] = "abaab abaab";
        TEST_STRALGO_ERASEEND_STR(EraseHead, test_str1, 5, res_str1);
        TEST_STRALGO_ERASEEND_CHR(EraseHead, test_chr1, 5, res_str1.c_str());

        string test_str2 = "abaab abaab";
        string res_str2 = "abaab ";
        char test_chr2[] = "abaab abaab";
        TEST_STRALGO_ERASEEND_STR(EraseTail, test_str2, 5, res_str2);
        TEST_STRALGO_ERASEEND_CHR(EraseTail, test_chr2, 5, res_str2.c_str());
    }
    
    void testEraseTail(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str2 = "abaab abaab";
        string res_str2 = "abaab ";
        char test_chr2[] = "abaab abaab";
        TEST_STRALGO_ERASEEND_STR(EraseTail, test_str2, 5, res_str2);
        TEST_STRALGO_ERASEEND_CHR(EraseTail, test_chr2, 5, res_str2.c_str());
    }

    void testEraseHeadCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string res_str1 = " abaab";
        char test_chr1[] = "abaab abaab";
        TEST_STRALGO_ERASEENDCOPY_STR(EraseHeadCopy, test_str1, 5, res_str1);
        TEST_STRALGO_ERASEENDCOPY_CHR(EraseHeadCopy, test_chr1, 5, res_str1.c_str());
    }
  
    void testEraseTailCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str2 = "abaab abaab";
        string res_str2 = "abaab ";
        char test_chr2[] = "abaab abaab";
        TEST_STRALGO_ERASEENDCOPY_STR(EraseTailCopy, test_str2, 5, res_str2);
        TEST_STRALGO_ERASEENDCOPY_CHR(EraseTailCopy, test_chr2, 5, res_str2.c_str());
    }
 
    void testFind(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string sub_str1 = "ab";
        TEST_STRALGO_FIND(Find, test_str1, sub_str1, 0);
        TEST_STRALGO_FIND(Find, test_str1.c_str(), sub_str1.c_str(), 0);
 
        string sub_str2 = " abaab";
        TEST_STRALGO_FIND(Find, test_str1, sub_str2, 5);
        TEST_STRALGO_FIND(Find, test_str1.c_str(), sub_str2.c_str(), 5);

        string sub_str3 = "abc";
        TEST_STRALGO_FIND(Find, test_str1, sub_str3, -1);
        TEST_STRALGO_FIND(Find, test_str1.c_str(), sub_str3.c_str(), -1);
    }

    void testFindFirst(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string sub_str1 = "ab";
        TEST_STRALGO_FIND(FindFirst, test_str1, sub_str1, 0);
        TEST_STRALGO_FIND(FindFirst, test_str1.c_str(), sub_str1.c_str(), 0);
 
        string sub_str2 = " abaab";
        TEST_STRALGO_FIND(FindFirst, test_str1, sub_str2, 5);
        TEST_STRALGO_FIND(FindFirst, test_str1.c_str(), sub_str2.c_str(), 5);

        string sub_str3 = "abc";
        TEST_STRALGO_FIND(FindFirst, test_str1, sub_str3, -1);
        TEST_STRALGO_FIND(FindFirst, test_str1.c_str(), sub_str3.c_str(), -1);
    }

    void testFindLast(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string sub_str1 = "ab";
        TEST_STRALGO_FIND(FindLast, test_str1, sub_str1, 9);
        TEST_STRALGO_FIND(FindLast, test_str1.c_str(), sub_str1.c_str(), 9);
 
        string sub_str2 = " abaab";
        TEST_STRALGO_FIND(FindLast, test_str1, sub_str2, 5);
        TEST_STRALGO_FIND(FindLast, test_str1.c_str(), sub_str2.c_str(), 5);

        string sub_str3 = "abc";
        TEST_STRALGO_FIND(FindLast, test_str1, sub_str3, -1);
        TEST_STRALGO_FIND(FindLast, test_str1.c_str(), sub_str3.c_str(), -1);
    }

    void testFindN(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string sub_str1 = "ab";
        TEST_STRALGO_FINDN(test_str1, sub_str1, 1, 0);
        TEST_STRALGO_FINDN(test_str1.c_str(), sub_str1.c_str(), 1, 0);
        TEST_STRALGO_FINDN(test_str1, sub_str1, 2, 3);
        TEST_STRALGO_FINDN(test_str1.c_str(), sub_str1.c_str(), 2, 3);
        TEST_STRALGO_FINDN(test_str1, sub_str1, 3, 6);
        TEST_STRALGO_FINDN(test_str1.c_str(), sub_str1.c_str(), 3, 6);
        TEST_STRALGO_FINDN(test_str1, sub_str1, 4, 9);
        TEST_STRALGO_FINDN(test_str1.c_str(), sub_str1.c_str(), 4, 9);
        TEST_STRALGO_FINDN(test_str1, sub_str1, 5, -1);
        TEST_STRALGO_FINDN(test_str1.c_str(), sub_str1.c_str(), 5, -1);
    }
    void testReplaceRange(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string sub_str1 = "b";
        string res_str1 = "bbaab abaab";
        TEST_STRALGO_REPLACERANGE_STR(test_str1, 0, 1, sub_str1, res_str1);
        TEST_STRALGO_REPLACERANGE_CHR(test_str1.c_str(), 0, 1, sub_str1.c_str(), res_str1.c_str());

        string res_str2 = "abaab abab";
        TEST_STRALGO_REPLACERANGE_STR(test_str1, 9, 2, sub_str1, res_str2);
        TEST_STRALGO_REPLACERANGE_CHR(test_str1.c_str(), 9, 2, sub_str1.c_str(), res_str2.c_str());

        string res_str3 = "abaab abab";
        TEST_STRALGO_REPLACERANGE_STR(test_str1, 9, 3, sub_str1, res_str3);
        TEST_STRALGO_REPLACERANGE_CHR(test_str1.c_str(), 9, 3, sub_str1.c_str(), res_str3.c_str());
        
        string res_str4 = "abaab abab";
        TEST_STRALGO_REPLACERANGE_STR(test_str1, 9, 4, sub_str1, res_str4);
        TEST_STRALGO_REPLACERANGE_CHR(test_str1.c_str(), 9, 4, sub_str1.c_str(), res_str4.c_str());
    }
    void testReplace(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string rsub_str1 = "ba";
        string ssub_str7 = "ab";
        string res_str7 = "baaba baaba";
        TEST_STRALGO_REPLACE_STR(ReplaceCopy, test_str1, ssub_str7, rsub_str1, res_str7);
        TEST_STRALGO_REPLACE_CHR(ReplaceCopy, test_str1.c_str(), ssub_str7.c_str(), rsub_str1.c_str(), res_str7.c_str());

        string ssub_str8 = "ba";
	string res_str8 = "abaab abaab";
        TEST_STRALGO_REPLACE_STR(ReplaceCopy, test_str1, ssub_str8, rsub_str1, res_str8);
        TEST_STRALGO_REPLACE_CHR(ReplaceCopy, test_str1.c_str(), ssub_str8.c_str(), rsub_str1.c_str(), res_str8.c_str());
    }

    void testReplaceFirst(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string ssub_str1 = "ab";
        string rsub_str1 = "ba";
        string res_str1 = "baaab abaab";
        TEST_STRALGO_REPLACE_STR(ReplaceFirstCopy, test_str1, ssub_str1, rsub_str1, res_str1);
        TEST_STRALGO_REPLACE_CHR(ReplaceFirstCopy, test_str1.c_str(), ssub_str1.c_str(), rsub_str1.c_str(), res_str1.c_str());
        
        string ssub_str2 = " abaab";
        string res_str2 = "abaabba";
        TEST_STRALGO_REPLACE_STR(ReplaceFirstCopy, test_str1, ssub_str1, rsub_str1, res_str1);
        TEST_STRALGO_REPLACE_CHR(ReplaceFirstCopy, test_str1.c_str(), ssub_str2.c_str(), rsub_str1.c_str(), res_str2.c_str());

        string ssub_str3 = "bab";
        string res_str3 = "abaab abaab";
        TEST_STRALGO_REPLACE_STR(ReplaceFirstCopy, test_str1, ssub_str3, rsub_str1, res_str3);
        TEST_STRALGO_REPLACE_CHR(ReplaceFirstCopy, test_str1.c_str(), ssub_str3.c_str(), rsub_str1.c_str(), res_str3.c_str());
    }

    void testReplaceLast(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string rsub_str1 = "ba";
        string ssub_str4 = "abaab ";
        string res_str4 = "baabaab";
        TEST_STRALGO_REPLACE_STR(ReplaceLastCopy, test_str1, ssub_str4, rsub_str1, res_str4);
        TEST_STRALGO_REPLACE_CHR(ReplaceLastCopy, test_str1.c_str(), ssub_str4.c_str(), rsub_str1.c_str(), res_str4.c_str());

        string ssub_str5 = "ab";
        string res_str5 = "abaab ababa";
        TEST_STRALGO_REPLACE_STR(ReplaceLastCopy, test_str1, ssub_str5, rsub_str1, res_str5);
        TEST_STRALGO_REPLACE_CHR(ReplaceLastCopy, test_str1.c_str(), ssub_str5.c_str(), rsub_str1.c_str(), res_str5.c_str());

        string ssub_str6 = "bab";
        string res_str6 = "abaab abaab";
        TEST_STRALGO_REPLACE_STR(ReplaceLastCopy, test_str1, ssub_str6, rsub_str1, res_str6);
        TEST_STRALGO_REPLACE_CHR(ReplaceLastCopy, test_str1.c_str(), ssub_str6.c_str(), rsub_str1.c_str(), res_str6.c_str());
    }

    void testReplaceN(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "abaab abaab";
        string ssub_str1 = "ab";
        string rsub_str1 = "ba";
        string res_str1 = "baaab abaab";
        TEST_STRALGO_REPLACEN_STR(test_str1, ssub_str1, 1, rsub_str1, res_str1);
        TEST_STRALGO_REPLACEN_CHR(test_str1.c_str(), ssub_str1.c_str(), 1, rsub_str1.c_str(), res_str1.c_str());

        string res_str2 = "ababa abaab";
        TEST_STRALGO_REPLACEN_STR(test_str1, ssub_str1, 2, rsub_str1, res_str2);
        TEST_STRALGO_REPLACEN_CHR(test_str1.c_str(), ssub_str1.c_str(), 2, rsub_str1.c_str(), res_str2.c_str());

        string res_str3 = "abaab baaab";
        TEST_STRALGO_REPLACEN_STR(test_str1, ssub_str1, 3, rsub_str1, res_str3);
        TEST_STRALGO_REPLACEN_CHR(test_str1.c_str(), ssub_str1.c_str(), 3, rsub_str1.c_str(), res_str3.c_str());

        string res_str4 = "abaab ababa";
        TEST_STRALGO_REPLACEN_STR(test_str1, ssub_str1, 4, rsub_str1, res_str4);
        TEST_STRALGO_REPLACEN_CHR(test_str1.c_str(), ssub_str1.c_str(), 4, rsub_str1.c_str(), res_str4.c_str());

        string res_str5 = "abaab abaab";
        TEST_STRALGO_REPLACEN_STR(test_str1, ssub_str1, 5, rsub_str1, res_str5);
        TEST_STRALGO_REPLACEN_CHR(test_str1.c_str(), ssub_str1.c_str(), 5, rsub_str1.c_str(), res_str5.c_str());
    }

    void testStartWith(void)
    {
        PRINT_TITLE_2(__func__);
        string fstr1 = "abaab abaab";
        string sstr1 = "a";
        TEST_STRALGO_COMPARE(StartWith, fstr1, sstr1, true);
        TEST_STRALGO_COMPARE(StartWith, fstr1.c_str(), sstr1, true);
        TEST_STRALGO_COMPARE(StartWith, fstr1, sstr1.c_str(), true);
        TEST_STRALGO_COMPARE(StartWith, fstr1.c_str(), sstr1.c_str(), true);
        
        string fstr2 = "abaab abaab";
        string sstr2 = "abaab abaab";
        TEST_STRALGO_COMPARE(StartWith, fstr2, sstr2, true);
        TEST_STRALGO_COMPARE(StartWith, fstr2.c_str(), sstr2, true);
        TEST_STRALGO_COMPARE(StartWith, fstr2, sstr2.c_str(), true);
        TEST_STRALGO_COMPARE(StartWith, fstr2.c_str(), sstr2.c_str(), true);

        string fstr3 = "abaab abaab";
        string sstr3 = "abaab abaaba";
        TEST_STRALGO_COMPARE(StartWith, fstr3, sstr3, false);
        TEST_STRALGO_COMPARE(StartWith, fstr3.c_str(), sstr3, false);
        TEST_STRALGO_COMPARE(StartWith, fstr3, sstr3.c_str(), false);
        TEST_STRALGO_COMPARE(StartWith, fstr3.c_str(), sstr3.c_str(), false);
    }
    
    void testEndWith(void)
    {
        PRINT_TITLE_2(__func__);
        string fstr4 = "abaab abaab";
        string sstr4 = "b";
        TEST_STRALGO_COMPARE(EndWith, fstr4, sstr4, true);
        TEST_STRALGO_COMPARE(EndWith, fstr4.c_str(), sstr4, true);
        TEST_STRALGO_COMPARE(EndWith, fstr4, sstr4.c_str(), true);
        TEST_STRALGO_COMPARE(EndWith, fstr4.c_str(), sstr4.c_str(), true);

        string fstr5 = "abaab abaab";
        string sstr5 = "abaab abaab";
        TEST_STRALGO_COMPARE(EndWith, fstr5, sstr5, true);
        TEST_STRALGO_COMPARE(EndWith, fstr5.c_str(), sstr5, true);
        TEST_STRALGO_COMPARE(EndWith, fstr5, sstr5.c_str(), true);
        TEST_STRALGO_COMPARE(EndWith, fstr5.c_str(), sstr5.c_str(), true);

        string fstr6 = "abaab abaab";
        string sstr6 = "aabaab abaab";
        TEST_STRALGO_COMPARE(EndWith, fstr6, sstr6, false);
        TEST_STRALGO_COMPARE(EndWith, fstr6.c_str(), sstr6, false);
        TEST_STRALGO_COMPARE(EndWith, fstr6, sstr6.c_str(), false);
        TEST_STRALGO_COMPARE(EndWith, fstr6.c_str(), sstr6.c_str(), false);
    }
   
    void testContain(void)
    {
        PRINT_TITLE_2(__func__);
        string fstr7 = "abaab abaab";
        string sstr7 = "ab";
        TEST_STRALGO_COMPARE(Contain, fstr7, sstr7, true);
        TEST_STRALGO_COMPARE(Contain, fstr7.c_str(), sstr7, true);
        TEST_STRALGO_COMPARE(Contain, fstr7, sstr7.c_str(), true);
        TEST_STRALGO_COMPARE(Contain, fstr7.c_str(), fstr7.c_str(), true);

        string fstr8 = "abaab abaab";
        string sstr8 = " abaab";
        TEST_STRALGO_COMPARE(Contain, fstr8, sstr8, true);
        TEST_STRALGO_COMPARE(Contain, fstr8.c_str(), sstr8, true);
        TEST_STRALGO_COMPARE(Contain, fstr8, sstr8.c_str(), true);
        TEST_STRALGO_COMPARE(Contain, fstr8.c_str(), sstr8.c_str(), true);

        string fstr9 = "abaab abaab";
        string sstr9 = "aab";
        TEST_STRALGO_COMPARE(Contain, fstr9, sstr9, true);
        TEST_STRALGO_COMPARE(Contain, fstr9.c_str(), sstr9, true);
        TEST_STRALGO_COMPARE(Contain, fstr9, sstr9.c_str(), true);
        TEST_STRALGO_COMPARE(Contain, fstr9.c_str(), sstr9.c_str(), true);

        string fstr10 = "abaab abaab";
        string sstr10 = "abaab abaaba";
        TEST_STRALGO_COMPARE(Contain, fstr10, sstr10, false);
        TEST_STRALGO_COMPARE(Contain, fstr10.c_str(), sstr10, false);
        TEST_STRALGO_COMPARE(Contain, fstr10, sstr10.c_str(), false);
        TEST_STRALGO_COMPARE(Contain, fstr10.c_str(), sstr10.c_str(), false);
    }

    void testEqual(void)
    {
        PRINT_TITLE_2(__func__);
        string fstr11 = "abaab abaab";
        string sstr11 = "abaab abaab";
        TEST_STRALGO_COMPARE(Equal, fstr11, sstr11, true);
        TEST_STRALGO_COMPARE(Equal, fstr11.c_str(), sstr11, true);
        TEST_STRALGO_COMPARE(Equal, fstr11, sstr11.c_str(), true);
        TEST_STRALGO_COMPARE(Equal, fstr11.c_str(), sstr11.c_str(), true);

        string fstr12 = "abaab abaab";
        string sstr12 = "abaab abaabb";
        TEST_STRALGO_COMPARE(Equal, fstr12, sstr12, false);
        TEST_STRALGO_COMPARE(Equal, fstr12.c_str(), sstr12, false);
        TEST_STRALGO_COMPARE(Equal, fstr12, sstr12.c_str(), false);
        TEST_STRALGO_COMPARE(Equal, fstr12.c_str(), sstr12.c_str(), false);
    } 
    void testTrimIf(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "aaabbb bbb aaa";
        string res_str1 = "bbb bbb ";
        char test_chr1[] = "aaabbb bbb aaa"; 
        TEST_STRALGO_TRIMIF_STR(test_str1, checkA, res_str1);
        TEST_STRALGO_TRIMIF_CHR(test_chr1, checkA, res_str1.c_str()); 
        
        string test_str2 = "aaabbab aab aaa";
        string res_str2 = "bbab aab ";
        char test_chr2[] = "aaabbab aab aaa";
        TEST_STRALGO_TRIMIF_STR(test_str2, checkA, res_str2);
        TEST_STRALGO_TRIMIF_CHR(test_chr2, checkA, res_str2.c_str());
    }

    void testTrimIfCopy(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "aaabbb bbb aaa";
        string res_str1 = "bbb bbb ";
        char test_chr1[] = "aaabbb bbb aaa";
        TEST_STRALGO_TRIMIFCOPY_STR(test_str1, checkA, res_str1);
        TEST_STRALGO_TRIMIFCOPY_CHR(test_chr1, checkA, res_str1.c_str());

        string test_str2 = "aaabbab aab aaa";
        string res_str2 = "bbab aab ";
        char test_chr2[] = "aaabbab aab aaa";
        TEST_STRALGO_TRIMIFCOPY_STR(test_str2, checkA, res_str2);
        TEST_STRALGO_TRIMIFCOPY_CHR(test_chr2, checkA, res_str2.c_str());
    }

    void testAll(void)
    {
        PRINT_TITLE_2(__func__);
        string test_str1 = "aa";
        char test_chr1[] = "aa";
        TEST_STRALGO_ALL(test_str1, checkA, true);
        TEST_STRALGO_ALL(test_chr1, checkA, true);

        string test_str2 = "ab";
        char test_chr2[] = "ab";
        TEST_STRALGO_ALL(test_str2, checkA, false);
        TEST_STRALGO_ALL(test_chr2, checkA, false);
    }
    
    void testKMPFind(void)
    {
        PRINT_TITLE_2(__func__);
        struct CTest1
        {
           std::string moInput;
           std::string moSearch;
           apl_ssize_t moResult;
        } TestNode1[] = 
        {
            {"abcdefg", "abc",   0},
            {"abcdefg", "a",     0},
            {"abcdefg", "cde",   2},
            {"abcdefg", "cd",    2},
            {"abcdefg", "efg",   4},
            {"abcdefg", "e",     4},
            {"abcdefg", "g",     6},
            {"abcdefg", "exfg", -1},
        };
        
        struct CTest2
        {
           char macInput[64];
           char macSearch[64];
           apl_ssize_t moResult;
        } TestNode2[] = 
        {
            {"abcdefg", "abc",   0},
            {"abcdefg", "a",     0},
            {"abcdefg", "cde",   2},
            {"abcdefg", "cd",    2},
            {"abcdefg", "efg",   4},
            {"abcdefg", "e",     4},
            {"abcdefg", "g",     6},
            {"abcdefg", "exfg", -1},
        };
        
        //case 1
        for (apl_size_t liN = 0; liN < 8; liN++)
        {
            CPPUNIT_ASSERT_MESSAGE("KMPFind fail", 
                KMPFind(
                        TestNode1[liN].moInput,
                        TestNode1[liN].moSearch ) == TestNode1[liN].moResult );
        }
        
        //case 2
        for (apl_size_t liN = 0; liN < 8; liN++)
        {
            CPPUNIT_ASSERT_MESSAGE("KMPFind fail", 
                KMPFind(
                        TestNode2[liN].macInput,
                        TestNode2[liN].macSearch ) == TestNode2[liN].moResult );
        }
        
        //case 3
        for (apl_size_t liN = 0; liN < 8; liN++)
        {
            CPPUNIT_ASSERT_MESSAGE("KMPFind fail", 
                KMPFind(
                        TestNode2[liN].macInput,
                        apl_strlen(TestNode2[liN].macInput),
                        TestNode2[liN].macSearch ) == TestNode2[liN].moResult );
        }
        
        //case 4
        for (apl_size_t liN = 0; liN < 8; liN++)
        {
            CPPUNIT_ASSERT_MESSAGE("KMPFind fail", 
                KMPFind(
                        TestNode2[liN].macInput,
                        apl_strlen(TestNode2[liN].macInput),
                        TestNode2[liN].macSearch,
                        apl_strlen(TestNode2[liN].macSearch) ) == TestNode2[liN].moResult );
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclStralgo);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
 
        
