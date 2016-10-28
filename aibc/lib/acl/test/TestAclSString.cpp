#include "Test.h"
#include "acl/SString.h"
#include <string>
using namespace acl;
using std::string;
using std::vector;

#define TEST_SSTRING_CHECK_RESULT(spliter, res) \
    do { \
       apl_int_t size; \
       size = spliter.GetSize(); \
       CPPUNIT_ASSERT_MESSAGE("spliter fail", (unsigned)size == res.size()); \
       for(apl_int_t i = 0;i < size;i++) \
       { \
           CPPUNIT_ASSERT_MESSAGE("spliter fail", apl_strcmp(spliter.GetField(i), res[i].c_str()) == 0); \
       } \
      }while(0)
 
class CTestAclSString : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclSString);
    CPPUNIT_TEST(testNormalParse);
    CPPUNIT_TEST(testPreserverParse);
    CPPUNIT_TEST(testFixSpliter);
    CPPUNIT_TEST(testTokenizer);
    CPPUNIT_TEST(testPreserveTokenizer);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(void) {}
    void tearDown(void) {}
    void testNormalParse(void)
    {
        PRINT_TITLE_2(__func__);
        CSpliter spliter;
 
        string str1 = "This is a test01";
        vector<string> res1;
        string delimiter1 = " ";

        res1.push_back("This");
        res1.push_back("is");
        res1.push_back("a");
        res1.push_back("test01");
         
        spliter.Delimiter(delimiter1.c_str());
        spliter.Parse(str1);
        TEST_SSTRING_CHECK_RESULT(spliter, res1);

////////////////////////////////////////////////////////////

        string str2 = "abaab abaab";
        vector<string> res2;
        string delimiter2 = "a";
        
        res2.push_back("b");
        res2.push_back("b ");
        res2.push_back("b");
        res2.push_back("b");

        spliter.Delimiter(delimiter2.c_str());
        spliter.Parse(str2);
        TEST_SSTRING_CHECK_RESULT(spliter, res2);

////////////////////////////////////////////////////////////////

        string str3 = "This is a test01";
        vector<string> res3;
        string delimiter3 = " ";
     
        res3.push_back("This");
        res3.push_back("is");
        res3.push_back("a");
        res3.push_back("test01");

        spliter.Delimiter(delimiter3.c_str(), false);
        spliter.Parse(str3);
        TEST_SSTRING_CHECK_RESULT(spliter, res3);

//////////////////////////////////////////////////////////////

        string str4 = "abaab abaab";
        vector<string> res4;
        string delimiter4 = "a";

        res4.push_back("b");
        res4.push_back("");
        res4.push_back("b ");
        res4.push_back("b");
        res4.push_back("");
        res4.push_back("b");

        spliter.Delimiter(delimiter4.c_str(), false);
        spliter.Parse(str4);
        TEST_SSTRING_CHECK_RESULT(spliter,res4);
    }

    void testPreserverParse(void)
    {
        PRINT_TITLE_2(__func__);
        CSpliter spliter1;
        string str1 = "This is a test01";
        vector<string> res1;
        string delimiter1 = " ";
        
        res1.push_back("Thi");
        res1.push_back(" i");
        res1.push_back("a");
        res1.push_back("te");
        res1.push_back("t01");
        
        spliter1.Delimiter(delimiter1.c_str());
        spliter1.Preserve('s', 's');
        spliter1.Parse(str1);
        TEST_SSTRING_CHECK_RESULT(spliter1,res1);

////////////////////////////////////////////////////////////

        CSpliter spliter2;
        string str2 = "This is a test01";
        vector<string> res2;
        string delimiter2 = " ";
       
        res2.push_back("Th");
        res2.push_back("s ");
        res2.push_back(" a te");
        res2.push_back("t01");
 
        spliter2.Delimiter(delimiter2.c_str());
        spliter2.Preserve('s', 's');
        spliter2.Preserve('i', 'i');
        spliter2.Parse(str2);
        TEST_SSTRING_CHECK_RESULT(spliter2, res2);

/////////////////////////////////////////////////////////////

        CSpliter spliter3;
        string str3 = "abaab abaab";
        vector<string> res3;
        string delimiter3 = "a";

        res3.push_back("aa");
        res3.push_back(" ");
        res3.push_back("aa");
        
        spliter3.Delimiter(delimiter3.c_str());
        spliter3.Preserve('b', 'b');
        spliter3.Parse(str3);
        TEST_SSTRING_CHECK_RESULT(spliter3, res3);

///////////////////////////////////////////////////////////

        CSpliter spliter4;
        string str4 = "abaab abaab";
        vector<string> res4;
        string delimiter4 = "a";

        res4.push_back("b");
        res4.push_back("b ");
        res4.push_back("b");
        res4.push_back("b");

        spliter4.Delimiter(delimiter4.c_str());
        spliter4.Preserve('a', 'a');
        spliter4.Parse(str4);
        TEST_SSTRING_CHECK_RESULT(spliter4, res4);

/////////////////////////////////////////////////////////////

        CSpliter spliter5;
        string str5 = "This is a test01";
        vector<string> res5;
        string delimiter5 = " ";

        res5.push_back("Thi");
        res5.push_back(" is ");
        res5.push_back("te");
        res5.push_back("t01");

        spliter5.Delimiter(delimiter5.c_str(), false);
        spliter5.Preserve('s', 'a');
        spliter5.Parse(str5);
        TEST_SSTRING_CHECK_RESULT(spliter5, res5);

/////////////////////////////////////////////////////////////

        CSpliter spliter6;
        string str6 = "abaab abaab";
        vector<string> res6;
        string delimiter6 = "a";
    
        res6.push_back("b");
        res6.push_back("");
        res6.push_back("b ");
        res6.push_back("b");
        res6.push_back("");
        res6.push_back("b");

        spliter6.Delimiter(delimiter6.c_str(), false);
        spliter6.Preserve('a', 'a');
        spliter6.Parse(str6);
        TEST_SSTRING_CHECK_RESULT(spliter6, res6);

///////////////////////////////////////////////////////////

        CSpliter spliter7;
        string str7 = "This is a test01";
        vector<string> res7;
        string delimiter7 = " ";
    
        res7.push_back("Thi");
        res7.push_back("s is");
        res7.push_back("a");
        res7.push_back("te");
        res7.push_back("st01");

        spliter7.Delimiter(delimiter7.c_str());
        spliter7.Preserve('s', 's', false);
        spliter7.Parse(str7);
        TEST_SSTRING_CHECK_RESULT(spliter7, res7);

///////////////////////////////////////////////////////////

        CSpliter spliter8;
        string str8 = "abaab abaab";
        vector<string> res8;
        string delimiter8 = "a";

        res8.push_back("b");
        res8.push_back("b ");
        res8.push_back("b");
        res8.push_back("b");

        spliter8.Delimiter(delimiter8.c_str());
        spliter8.Preserve('a', 'a', false);
        spliter8.Parse(str8);
        TEST_SSTRING_CHECK_RESULT(spliter8, res8);

    }

    void testFixSpliter(void)
    {
        PRINT_TITLE_2(__func__);
        apl_size_t fixarray1[4] = {3, 3, 3, 3};
        CFixSpliter fixspliter1(fixarray1, 4);
        string str1 = "This is a test01";
        vector<string> res1;
       
        res1.push_back("Thi");
        res1.push_back("s i");
        res1.push_back("s a");
        res1.push_back(" te");
        
        fixspliter1.Parse(str1);
        TEST_SSTRING_CHECK_RESULT(fixspliter1, res1);

//////////////////////////////////////////////////////

        apl_size_t fixarray2[5] = {1, 1, 1, 1, 1};
        CFixSpliter fixspliter2(fixarray2, 5);
        string str2 = "This is a test01";
        vector<string> res2;

        res2.push_back("T");
        res2.push_back("h");
        res2.push_back("i");
        res2.push_back("s");
        res2.push_back(" ");

        fixspliter2.Parse(str2);
        TEST_SSTRING_CHECK_RESULT(fixspliter2, res2);

/////////////////////////////////////////////////////

        apl_size_t fixarray3[5] = {4, 4, 4, 4, 4};
        CFixSpliter fixspliter3(fixarray3, 5);
        string str3 = "This is a test01";
        apl_ssize_t ret;

        ret = fixspliter3.Parse(str3);
        CPPUNIT_ASSERT_MESSAGE("spliter fail", ret == -1);
    } 

    void testTokenizer(void)
    {
        PRINT_TITLE_2(__func__);
        CTokenizer tokenizer1;
        char str1[] = "This is a test01";
        vector<string> res1;

        res1.push_back("This is a test01");
        
        tokenizer1.Delimiter("2");
        tokenizer1.Parse(str1);
        TEST_SSTRING_CHECK_RESULT(tokenizer1, res1);

///////////////////////////////////////////////////////

        CTokenizer tokenizer2;
        char str2[] = "This is a test01";
        vector<string> res2;

        res2.push_back("This");
        res2.push_back("is");
        res2.push_back("a");
        res2.push_back("test01");

        tokenizer2.Delimiter(" ");
        tokenizer2.Parse(str2);
        TEST_SSTRING_CHECK_RESULT(tokenizer2, res2);

//////////////////////////////////////////////////////

        CTokenizer tokenizer3;
        char str3[] = "abaab abaab";
        vector<string> res3;

        res3.push_back("b");
        res3.push_back("b ");
        res3.push_back("b");
        res3.push_back("b");

        tokenizer3.Delimiter("a");
        tokenizer3.Parse(str3);
        TEST_SSTRING_CHECK_RESULT(tokenizer3, res3);

/////////////////////////////////////////////////////

        CTokenizer tokenizer4;
        char str4[] = "This is a test01";
        vector<string> res4;

        res4.push_back("This");
        res4.push_back("is");
        res4.push_back("a");
        res4.push_back("test01");

        tokenizer4.Delimiter(" ", false);
        tokenizer4.Parse(str4);
        TEST_SSTRING_CHECK_RESULT(tokenizer4, res4);

//////////////////////////////////////////////////////

        CTokenizer tokenizer5;
        char str5[] = "abaab abaab";
        vector<string> res5;

        res5.push_back("b");
        res5.push_back("");
        res5.push_back("b ");
        res5.push_back("b");
        res5.push_back("");
        res5.push_back("b");

        tokenizer5.Delimiter("a", false);
        tokenizer5.Parse(str5);
        TEST_SSTRING_CHECK_RESULT(tokenizer5, res5);
    }

    void testPreserveTokenizer(void)
    {
        PRINT_TITLE_2(__func__);
        CTokenizer tokenizer1;
        char str1[] = "This is a test01";
        vector<string> res1;

        res1.push_back("Thi");
        res1.push_back(" i");
        res1.push_back("a");
        res1.push_back("te");
        res1.push_back("t01");

        tokenizer1.Delimiter(" ");
        tokenizer1.Preserve('s', 's');
        tokenizer1.Parse(str1);
        TEST_SSTRING_CHECK_RESULT(tokenizer1, res1);

////////////////////////////////////////////////////////////////

        CTokenizer tokenizer2;
        char str2[] = "abaab abaab";
        vector<string> res2;

        res2.push_back("aa");
        res2.push_back(" ");
        res2.push_back("aa");
        
        tokenizer2.Delimiter("a");
        tokenizer2.Preserve('b', 'b');
        tokenizer2.Parse(str2);
        TEST_SSTRING_CHECK_RESULT(tokenizer2, res2);

/////////////////////////////////////////////////////////////////

        CTokenizer tokenizer3;
        char str3[] = "abaab abaab";
        vector<string> res3;

        res3.push_back("b");
        res3.push_back("b ");
        res3.push_back("b");
        res3.push_back("b");

        tokenizer3.Delimiter("a");
        tokenizer3.Preserve('a', 'a');
        tokenizer3.Parse(str3);
        TEST_SSTRING_CHECK_RESULT(tokenizer3, res3);

/////////////////////////////////////////////////////////////////

        CTokenizer tokenizer4;
        char str4[] = "This is a test01";
        vector<string> res4;

        res4.push_back("Thi");
        res4.push_back(" i");
        res4.push_back("a");
        res4.push_back("te");
        res4.push_back("t01");

        tokenizer4.Delimiter(" ", false);
        tokenizer4.Preserve('s', 's');
        tokenizer4.Parse(str4);
        TEST_SSTRING_CHECK_RESULT(tokenizer4, res4);

//////////////////////////////////////////////////////////////////

        CTokenizer tokenizer5;
        char str5[] = "abaab abaab";
        vector<string> res5;

        res5.push_back("b");
        res5.push_back("");
        res5.push_back("b ");
        res5.push_back("b");
        res5.push_back("");
        res5.push_back("b");

        tokenizer5.Delimiter("a", false);
        tokenizer5.Preserve('a', 'a');
        tokenizer5.Parse(str5);
        TEST_SSTRING_CHECK_RESULT(tokenizer5, res5);

/////////////////////////////////////////////////////////////////

        CTokenizer tokenizer6;
        char str6[] = "This is a test01";
        vector<string> res6;

        res6.push_back("This is");
        res6.push_back("s is");
        res6.push_back("a");
        res6.push_back("test01");
        res6.push_back("st01");

        tokenizer6.Delimiter(" ");
        tokenizer6.Preserve('s', 's', false);
        tokenizer6.Parse(str6);
        TEST_SSTRING_CHECK_RESULT(tokenizer6, res6);

//////////////////////////////////////////////////////////////////

        CTokenizer tokenizer7;
        char str7[] = "abaab abaab";
        vector<string> res7;

        res7.push_back("baab ");
        res7.push_back(" ");
        res7.push_back("baab");

        tokenizer7.Delimiter("a");
        tokenizer7.Preserve('b', 'b', false);
        tokenizer7.Parse(str7);
        TEST_SSTRING_CHECK_RESULT(tokenizer7, res7);
    } 
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclSString);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
