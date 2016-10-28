#include "Test.h"
#include "acl/Codecs.h"
#include "acl/Timestamp.h"

using namespace acl;

class CTestAclCodecsEscape : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclCodecsEscape);
    CPPUNIT_TEST(testEncode1);
    CPPUNIT_TEST(testEncode2);
    CPPUNIT_TEST(testEncode3);
    CPPUNIT_TEST(testDecode1);
    CPPUNIT_TEST(testDecode2);
    CPPUNIT_TEST(testDecode3);
    CPPUNIT_TEST(testStability);
    CPPUNIT_TEST_SUITE_END();
    
public:
    void setUp(void) {}
    void tearDown(void) {}

    void testEncode1(void)
    {
        PRINT_TITLE_2(__func__);
        
        CXmlEscapeEncoder loEncoder;
        char lacBuffer[1024] = "&lt;1&gt;2&amp;3&quot;4&quot;,&apos;5&apos;&lt;daizh&gt;[&quot;]&amp;[&apos;]";
        
        //case 1
        ASSERT_MESSAGE(loEncoder.Update("<1>2&3\"4\",\'5\'") == 0);
        ASSERT_MESSAGE(loEncoder.Final("<daizh>[\"]&[\']") == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), lacBuffer) == 0);
        
        //case 2
        ASSERT_MESSAGE(loEncoder.Final("<1>2&3\"4\",\'5\'<daizh>[\"]&[\']") == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), lacBuffer) == 0);
    }
    
    void testEncode2(void)
    {
        PRINT_TITLE_2(__func__);
        
        char lacTemp[1024];
        CXmlEscapeEncoder loEncoder(lacTemp, sizeof(lacTemp) );
        char lacBuffer[1024] = "&lt;1&gt;2&amp;3&quot;4&quot;,&apos;5&apos;&lt;daizh&gt;[&quot;]&amp;[&apos;]";

        //case 1
        ASSERT_MESSAGE(loEncoder.Update("<1>2&3\"4\",\'5\'", apl_strlen("<1>2&3\"4\",\'5\'") ) == 0);
        ASSERT_MESSAGE(loEncoder.Final("<daizh>[\"]&[\']", apl_strlen("<daizh>[\"]&[\']") ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), lacBuffer) == 0);
        
        //case 2
        ASSERT_MESSAGE(loEncoder.Final("<1>2&3\"4\",\'5\'<daizh>[\"]&[\']", apl_strlen("<1>2&3\"4\",\'5\'<daizh>[\"]&[\']") ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), lacBuffer) == 0);
    }
    
    void testEncode3(void)
    {
        PRINT_TITLE_2(__func__);
        
        CMemoryBlock loBuffer(1024);
        CXmlEscapeEncoder loEncoder(loBuffer);
        char lacBuffer[1024] = "&lt;1&gt;2&amp;3&quot;4&quot;,&apos;5&apos;&lt;daizh&gt;[&quot;]&amp;[&apos;]";

        //case 1
        ASSERT_MESSAGE(loEncoder.Update("<1>2&3\"4\",\'5\'", apl_strlen("<1>2&3\"4\",\'5\'") ) == 0);
        ASSERT_MESSAGE(loEncoder.Final("<daizh>[\"]&[\']", apl_strlen("<daizh>[\"]&[\']") ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), lacBuffer) == 0);
        
        //case 2
        ASSERT_MESSAGE(loEncoder.Final("<1>2&3\"4\",\'5\'<daizh>[\"]&[\']", apl_strlen("<1>2&3\"4\",\'5\'<daizh>[\"]&[\']") ) == 0);
        ASSERT_MESSAGE( apl_strcmp(loEncoder.GetOutput(), lacBuffer) == 0);
    }
    
    void testDecode1(void)
    {
        PRINT_TITLE_2(__func__);
        
        CXmlEscapeDecoder loDecoder;
        char lacBuffer[1024] = "&lt;1&gt;2&amp;3&quot;4&quot;,&#1633;&apos;5&apos;&lt;daizh&gt;[&quot;]&amp;[&apos;]";
        
        //case 1
        ASSERT_MESSAGE(loDecoder.Update(lacBuffer) == 0);
        ASSERT_MESSAGE(loDecoder.Final() == 0);
        ASSERT_MESSAGE( apl_strcmp(loDecoder.GetOutput(), "<1>2&3\"4\",&#1633;\'5\'<daizh>[\"]&[\']") == 0);
        
        //case 2
        ASSERT_MESSAGE(loDecoder.Final(lacBuffer) == 0);
        ASSERT_MESSAGE( apl_strcmp(loDecoder.GetOutput(), "<1>2&3\"4\",&#1633;\'5\'<daizh>[\"]&[\']") == 0);
    }
   
    void testDecode2(void)
    {
        PRINT_TITLE_2(__func__);
        
        char lacTemp[1024];
        CXmlEscapeDecoder loDecoder(lacTemp, sizeof(lacTemp) );
        char lacBuffer[1024] = "&lt;1&gt;2&amp;3&quot;4&quot;,&#1633;&apos;5&apos;&lt;daizh&gt;[&quot;]&amp;[&apos;]";
        
        //case 1
        ASSERT_MESSAGE(loDecoder.Update(lacBuffer) == 0);
        ASSERT_MESSAGE(loDecoder.Final() == 0);
        ASSERT_MESSAGE( apl_strcmp(loDecoder.GetOutput(), "<1>2&3\"4\",&#1633;\'5\'<daizh>[\"]&[\']") == 0);
        
        //case 2
        ASSERT_MESSAGE(loDecoder.Final(lacBuffer) == 0);
        ASSERT_MESSAGE( apl_strcmp(loDecoder.GetOutput(), "<1>2&3\"4\",&#1633;\'5\'<daizh>[\"]&[\']") == 0);
    }
   
    void testDecode3(void)
    {
        PRINT_TITLE_2(__func__);
        
        CMemoryBlock loBuffer(1024);
        CXmlEscapeDecoder loDecoder(loBuffer);
        char lacBuffer[1024] = "&lt;1&gt;2&amp;3&quot;4&quot;,&#1633;&apos;5&apos;&lt;daizh&gt;[&quot;]&amp;[&apos;]";
        
        //case 1
        ASSERT_MESSAGE(loDecoder.Update(lacBuffer) == 0);
        ASSERT_MESSAGE(loDecoder.Final() == 0);
        ASSERT_MESSAGE( apl_strcmp(loDecoder.GetOutput(), "<1>2&3\"4\",&#1633;\'5\'<daizh>[\"]&[\']") == 0);
        
        //case 2
        ASSERT_MESSAGE(loDecoder.Final(lacBuffer) == 0);
        ASSERT_MESSAGE( apl_strcmp(loDecoder.GetOutput(), "<1>2&3\"4\",&#1633;\'5\'<daizh>[\"]&[\']") == 0);
    }

    void testStability(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start environment
        struct CParam {
            CParam(CXmlEscapeEncoder& aoEncoder, CXmlEscapeDecoder& aoDecoder, apl_int_t& aiCount)
                : moEncoder(aoEncoder)
                , moDecoder(aoDecoder)
                , miCount(aiCount)
            {}
            
            CXmlEscapeEncoder& moEncoder;
            CXmlEscapeDecoder& moDecoder;
            volatile apl_int_t& miCount;
        };
        
        CXmlEscapeEncoder loEncoder;
        CXmlEscapeDecoder loDecoder;
        apl_int_t liCount = 0;
        
        CParam loParam(loEncoder, loDecoder, liCount);
        
        printf("\n");
        START_THREAD_BODY(mybody1, CParam, loParam)
            START_LOOP(100000)
            {
                assert(loParam.moEncoder.Final("<1>2&3\"4\",\'5\'<daizh>[\"]&[\']") == 0);
            }
            END_LOOP();
            //Use time=0.039000, 2564102.564103/pre
            loParam.miCount = 1;
        END_THREAD_BODY(mybody1);
        
        START_THREAD_BODY(mybody2, CParam, loParam)
            char lacBuffer[1024] = "&lt;1&gt;2&amp;3&quot;4&quot;,&#1633;&apos;5&apos;&lt;daizh&gt;[&quot;]&amp;[&apos;]";
            
            START_LOOP(100000)
            {
                assert(loParam.moDecoder.Final(lacBuffer) == 0);
            }
            END_LOOP();
            //Use time=0.046000, 2173913.043478/pre
            WAIT_EXP(loParam.miCount == 1);
            loParam.miCount = 2;
        END_THREAD_BODY(mybody2);

        RUN_THREAD_BODY(mybody1);
        RUN_THREAD_BODY(mybody2);
        WAIT_EXP(liCount == 2);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclCodecsEscape);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
