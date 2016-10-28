#include "Test.h"
#include "acl/Regex.h"
#include <string>
using std::string;
using std::vector;
using namespace acl;

#define TEST_REGEX_MATCH(pattern, str, res, cflag, mflag) \
    do { \
       CRegex cr; \
       apl_int_t ret; \
       ret = cr.Compile(pattern.c_str(), cflag); \
       PrintErrMsg("compile error", ret); \
       ret = cr.Match(str.c_str(), mflag); \
       CPPUNIT_ASSERT_MESSAGE("match error", res == ret); \
      }while(0)

#define TEST_REGEX_SEARCH(pattern,str,cflag,sflag,eres) \
    do { \
       CRegex cr; \
       CRegexMatchResult cmr; \
       apl_int_t ret; \
       ret = cr.Compile(pattern.c_str(), cflag); \
       PrintErrMsg("compile error", ret); \
       cr.Search(str.c_str(), &cmr, sflag); \
       TEST_REGEX_EQUAL(cmr, eres); \
      }while(0)
 
#define TEST_REGEX_EQUAL(mres, eres) \
    do { \
       CPPUNIT_ASSERT_MESSAGE("regex_search fail", mres.GetSize() == eres.size()); \
       for(apl_uint_t i = 0;i < mres.GetSize();i++) \
       { \
           CPPUNIT_ASSERT_MESSAGE("search error",apl_strcmp(mres[i].GetStr(), eres[i].c_str()) == 0); \
       } \
     }while(0)

#define TEST_REGEX_ISREADY \
    do { \
       CRegex cr; \
       apl_int_t ret; \
       CPPUNIT_ASSERT_MESSAGE("isready fail", cr.IsReady() == false); \
       ret = cr.Compile("a", CRegex::OPT_EXTENDED); \
       PrintErrMsg("complie error", ret); \
       CPPUNIT_ASSERT_MESSAGE("isready fail", cr.IsReady() == true); \
      }while(0)

class CTestAclRegex : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclRegex);
    CPPUNIT_TEST(testRegexMatch);
    CPPUNIT_TEST(testRegexSearch);
    CPPUNIT_TEST(testRegexReady);
    CPPUNIT_TEST_SUITE_END();
private:
    string str1, str2, str3, str4, str5, str6, str7, str8;
    string pattern1, pattern2, pattern3, pattern4, pattern5, pattern6, pattern7, pattern8;
public:
    void setUp(void)
    {
        str1 = "ababab";
        str2 = "ababab";
        str3 = "ABABA";
        str4 = "aba\nba";
        str5 = "aba^ab";
        str6 = "aba$b";
        str7 = "abaab";
        str8 = "abab";
        pattern1 = "(ab)+";
        pattern2 = "(ab)+?";
        pattern3 = "aba"; 
        pattern4 = "aba(.)+";
        pattern5 = "^a";
        pattern6 = "[ab]$";
        pattern7 = "^b";
        pattern8 = "a$";
    }
    void tearDown(void) {}
    void testRegexMatch(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_REGEX_MATCH(pattern1, str1, true, CRegex::OPT_EXTENDED, 0);
        TEST_REGEX_MATCH(pattern1, str1, true, CRegex::OPT_EXTENDED | CRegex::OPT_ICASE, 0);
        TEST_REGEX_MATCH(pattern1, str1, true, CRegex::OPT_EXTENDED | CRegex::OPT_NOSUB, 0);
        TEST_REGEX_MATCH(pattern1, str1, true, CRegex::OPT_EXTENDED | CRegex::OPT_NEWLINE, 0);

        TEST_REGEX_MATCH(pattern2, str2, true, CRegex::OPT_EXTENDED, 0);
        TEST_REGEX_MATCH(pattern2, str2, true, CRegex::OPT_EXTENDED | CRegex::OPT_ICASE, 0);
        TEST_REGEX_MATCH(pattern2, str2, true, CRegex::OPT_EXTENDED | CRegex::OPT_NOSUB, 0);
        TEST_REGEX_MATCH(pattern2, str2, true, CRegex::OPT_EXTENDED | CRegex::OPT_NEWLINE, 0);
        
        TEST_REGEX_MATCH(pattern3, str3, true, CRegex::OPT_EXTENDED | CRegex::OPT_ICASE, 0);
        TEST_REGEX_MATCH(pattern3, str3, false, CRegex::OPT_EXTENDED, 0);
       
        TEST_REGEX_MATCH(pattern4, str4, false, CRegex::OPT_EXTENDED | CRegex::OPT_NEWLINE, 0);
        TEST_REGEX_MATCH(pattern4, str4, true, CRegex::OPT_EXTENDED, 0);
        TEST_REGEX_MATCH(pattern7, str4, true, CRegex::OPT_EXTENDED | CRegex::OPT_NEWLINE, 0);
        TEST_REGEX_MATCH(pattern7, str4, false, CRegex::OPT_EXTENDED, 0);
        TEST_REGEX_MATCH(pattern8, str4, true, CRegex::OPT_EXTENDED | CRegex::OPT_NEWLINE, 0);
        
        TEST_REGEX_MATCH(pattern5, str5, true, CRegex::OPT_EXTENDED, 0);
        TEST_REGEX_MATCH(pattern5, str5, false, CRegex::OPT_EXTENDED, CRegex::OPT_NOTBOL);
        TEST_REGEX_MATCH(pattern5, str7, false, CRegex::OPT_EXTENDED, CRegex::OPT_NOTBOL);
        
        TEST_REGEX_MATCH(pattern6, str6, true, CRegex::OPT_EXTENDED, 0);
        TEST_REGEX_MATCH(pattern6, str6, false, CRegex::OPT_EXTENDED, CRegex::OPT_NOTEOL);
        TEST_REGEX_MATCH(pattern6, str8, false, CRegex::OPT_EXTENDED, CRegex::OPT_NOTEOL); 
    }

    void testRegexSearch(void)
    {
        PRINT_TITLE_2(__func__); 
        vector<string> restring1;
        restring1.push_back("ababab");
        TEST_REGEX_SEARCH(pattern1, str1, CRegex::OPT_EXTENDED, 0, restring1); 
        TEST_REGEX_SEARCH(pattern1, str1, CRegex::OPT_EXTENDED | CRegex::OPT_ICASE, 0, restring1);
        TEST_REGEX_SEARCH(pattern1, str1, CRegex::OPT_EXTENDED | CRegex::OPT_NEWLINE, 0, restring1);
       
        vector<string> restring3;
        restring3.push_back("ABA");
        TEST_REGEX_SEARCH(pattern3, str3, CRegex::OPT_EXTENDED | CRegex::OPT_ICASE, 0, restring3); 

        vector<string> restring4;
        TEST_REGEX_SEARCH(pattern3, str3, CRegex::OPT_EXTENDED, 0, restring4);
         
        vector<string> restring5; 
        TEST_REGEX_SEARCH(pattern4, str4, CRegex::OPT_EXTENDED | CRegex::OPT_NEWLINE, 0, restring5);

        vector<string> restring6;
        restring6.push_back("aba\nba");
        TEST_REGEX_SEARCH(pattern4, str4, CRegex::OPT_EXTENDED, 0, restring6);
 
        vector<string> restring7;
        restring7.push_back("a");
        TEST_REGEX_SEARCH(pattern5, str5, CRegex::OPT_EXTENDED, 0, restring7); 

        vector<string> restring8;
        restring8.push_back("b");
        TEST_REGEX_SEARCH(pattern6, str6, CRegex::OPT_EXTENDED, 0, restring8);

        vector<string> restring9;
        TEST_REGEX_SEARCH(pattern1, str1, CRegex::OPT_EXTENDED | CRegex::OPT_NOSUB, 0, restring9);

        vector<string> restring10;
        TEST_REGEX_SEARCH(pattern2, str2, CRegex::OPT_EXTENDED | CRegex::OPT_NOSUB, 0, restring10);

        vector<string> restring11;
        TEST_REGEX_SEARCH(pattern5, str5, CRegex::OPT_EXTENDED, CRegex::OPT_NOTBOL, restring11);

        vector<string> restring12;
        TEST_REGEX_SEARCH(pattern6, str6, CRegex::OPT_EXTENDED, CRegex::OPT_NOTEOL, restring12);

        vector<string> restring13;
        restring13.push_back("b");
        TEST_REGEX_SEARCH(pattern7, str4, CRegex::OPT_EXTENDED | CRegex::OPT_NEWLINE, 0, restring13);
      
        vector<string> restring14;
        restring14.push_back("a");
        restring14.push_back("a");
        TEST_REGEX_SEARCH(pattern8, str4, CRegex::OPT_EXTENDED | CRegex::OPT_NEWLINE, 0, restring14);

        vector<string> restring15;
        TEST_REGEX_SEARCH(pattern7, str4, CRegex::OPT_EXTENDED, 0, restring15);
        
        vector<string> restring16;
        restring16.push_back("a");
        TEST_REGEX_SEARCH(pattern8, str4, CRegex::OPT_EXTENDED, 0, restring16);
    }
    void testRegexReady(void)
    {
        PRINT_TITLE_2(__func__);
        TEST_REGEX_ISREADY;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclRegex);
int main()
{
    RUN_ALL_TEST(__FILE__);
} 
