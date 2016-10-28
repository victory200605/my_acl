#include "Test.h"
#include "acl/codecs/Unicode.h"
#include "acl/codecs/Hex.h"
#include "acl/FileStream.h"
#include "GBKAndUnicodeTable.ini"
#include <map>

using namespace acl;

class CTestAclCodecsUnicode : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclCodecsUnicode);
//    CPPUNIT_TEST(testUTF16BEToUTF8);
    CPPUNIT_TEST(testUTF32BEToUTF16BE);
    CPPUNIT_TEST(testUTF32LEToUTF16BE);
    CPPUNIT_TEST(testUTF32BEToUTF16LE);
    CPPUNIT_TEST(testUTF32LEToUTF16LE);
    CPPUNIT_TEST(testUTF32BEToUTF8);
    CPPUNIT_TEST(testUTF32LEToUTF8);
    CPPUNIT_TEST(testUTF16BEToUTF8);
    CPPUNIT_TEST(testUTF16LEToUTF8);
    CPPUNIT_TEST(testUTF8ToUTF32BE);
    CPPUNIT_TEST(testUTF8ToUTF32LE);
    CPPUNIT_TEST(testUTF8ToUTF16BE);
    CPPUNIT_TEST(testUTF8ToUTF16LE);
    CPPUNIT_TEST(testUTF16BEToUTF32BE);   
    CPPUNIT_TEST(testUTF16BEToUTF32LE);
    CPPUNIT_TEST(testUTF16LEToUTF32BE);
    CPPUNIT_TEST(testUTF16LEToUTF32LE);
    CPPUNIT_TEST(testGBKToUTF16BE);
    CPPUNIT_TEST(testGBKToUTF16LE);
    CPPUNIT_TEST(testUTF16BEToGBK);
    CPPUNIT_TEST(testUTF16LEToGBK);
    CPPUNIT_TEST(testGBKToUTF8);
    CPPUNIT_TEST(testUTF8ToGBK);
    CPPUNIT_TEST(testTranscoderUpdate);
    CPPUNIT_TEST_SUITE_END();
    
public:
    void setUp(void) {}
    void tearDown(void) {}

/*    void testUTF16BEToUTF8(void)
    {
        {
            char lacBuffer[] = {0x4E, 0x25};
        
            CCUTF32ToUTF16Transcoder loTranscoder;
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        
        CHexEncoder loEncoder;
        CUTF16BEToUTF8Transcoder loTranscoder;
        ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == 0);
        loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() );
        printf("%s\n", loEncoder.GetOutput() );
    }
*/
    void testUTF32BEToUTF16BE(void)
    {
        {
            char lacBuffer[] = {0x00, 0x11, 0xFF, 0xFF};
        
            char lacTemp[1024];
            CUTF32ToUTF16Transcoder loTranscoder(lacTemp, sizeof(lacTemp), FROM_BIG_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0x00, 0x00, 0xD8, 0x00};
        
            CMemoryBlock loBuffer(1024);
            CUTF32ToUTF16Transcoder loTranscoder(loBuffer, FROM_BIG_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0x00, 0x00, 0xD8, 0x01};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0x00, 0x00, 0xDF, 0xFF};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

#define UTF32BE_TO_UTF16BE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF16Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_BIG_ENDIAN, true);                  \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }
        
        UTF32BE_TO_UTF16BE_STRICT_CHECK("DBFFDFFF", 4, 0x00, 0x10, 0xFF, 0xFF);
        UTF32BE_TO_UTF16BE_STRICT_CHECK("DB7FDFFF", 4, 0x00, 0x0E, 0xFF, 0xFF);
        UTF32BE_TO_UTF16BE_STRICT_CHECK("DBE0DC01", 4, 0x00, 0x10, 0x80, 0x01);
        UTF32BE_TO_UTF16BE_STRICT_CHECK("D1FF", 4, 0x00, 0x00, 0xD1, 0xFF);
        UTF32BE_TO_UTF16BE_STRICT_CHECK("FFFF", 4, 0x00, 0x00, 0xFF, 0xFF);

#define UTF32BE_TO_UTF16BE_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF16Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_BIG_ENDIAN, false);                 \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }
        
        UTF32BE_TO_UTF16BE_CHECK("DBFFDFFF", 4, 0x00, 0x10, 0xFF, 0xFF);
        UTF32BE_TO_UTF16BE_CHECK("DB7FDFFF", 4, 0x00, 0x0E, 0xFF, 0xFF);
        UTF32BE_TO_UTF16BE_CHECK("DBE0DC01", 4, 0x00, 0x10, 0x80, 0x01);
        UTF32BE_TO_UTF16BE_CHECK("D1FF", 4, 0x00, 0x00, 0xD1, 0xFF);
        UTF32BE_TO_UTF16BE_CHECK("FFFF", 4, 0x00, 0x00, 0xFF, 0xFF);
        UTF32BE_TO_UTF16BE_CHECK("003F", 4, 0x00, 0x11, 0x00, 0x00);
        UTF32BE_TO_UTF16BE_CHECK("003F", 4, 0x00, 0x00, 0xD8, 0x00);
        UTF32BE_TO_UTF16BE_CHECK("003F", 4, 0x00, 0x00, 0xDF, 0xFF);
        UTF32BE_TO_UTF16BE_CHECK("003F", 4, 0xFF, 0xFF, 0xFF, 0xFF);

    }
    
    void testUTF32LEToUTF16BE(void)
    {
        {
            char lacBuffer[] = {0xFF, 0xFF, 0x11, 0x00};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0x00, 0xD8, 0x00, 0x00};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0x01, 0xD8, 0x00, 0x00};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0xFF, 0xDF, 0x00, 0x00};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

#define UTF32LE_TO_UTF16BE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, true);               \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }
        
        UTF32LE_TO_UTF16BE_STRICT_CHECK("DBFFDFFF", 4, 0xFF, 0xFF, 0x10, 0x00);
        UTF32LE_TO_UTF16BE_STRICT_CHECK("DB7FDFFF", 4, 0xFF, 0xFF, 0x0E, 0x00);
        UTF32LE_TO_UTF16BE_STRICT_CHECK("DBE0DC01", 4, 0x01, 0x80, 0x10, 0x00);
        UTF32LE_TO_UTF16BE_STRICT_CHECK("D1FF", 4, 0xFF, 0xD1, 0x00, 0x00);
        UTF32LE_TO_UTF16BE_STRICT_CHECK("FFFF", 4, 0xFF, 0xFF, 0x00, 0x00);

#define UTF32LE_TO_UTF16BE_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, false);              \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }
        
        UTF32LE_TO_UTF16BE_CHECK("DBFFDFFF", 4, 0xFF, 0xFF, 0x10, 0x00);
        UTF32LE_TO_UTF16BE_CHECK("DB7FDFFF", 4, 0xFF, 0xFF, 0x0E, 0x00);
        UTF32LE_TO_UTF16BE_CHECK("DBE0DC01", 4, 0x01, 0x80, 0x10, 0x00);
        UTF32LE_TO_UTF16BE_CHECK("D1FF", 4, 0xFF, 0xD1, 0x00, 0x00);
        UTF32LE_TO_UTF16BE_CHECK("FFFF", 4, 0xFF, 0xFF, 0x00, 0x00);
        UTF32LE_TO_UTF16BE_CHECK("003F", 4, 0x00, 0x00, 0x11, 0x00);
        UTF32LE_TO_UTF16BE_CHECK("003F", 4, 0x00, 0xD8, 0x00, 0x00);
        UTF32LE_TO_UTF16BE_CHECK("003F", 4, 0xFF, 0xDF, 0x00, 0x00);
        UTF32LE_TO_UTF16BE_CHECK("003F", 4, 0xFF, 0xFF, 0xFF, 0xFF);

    }
    
    void testUTF32BEToUTF16LE(void)
    {
        {
            char lacBuffer[] = {0x00, 0x11, 0xFF, 0xFF};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0x00, 0x00, 0xD8, 0x00};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0x00, 0x00, 0xD8, 0x01};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0x00, 0x00, 0xDF, 0xFF};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

#define UTF32BE_TO_UTF16LE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF16Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, true);               \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }
        
        UTF32BE_TO_UTF16LE_STRICT_CHECK("FFDBFFDF", 4, 0x00, 0x10, 0xFF, 0xFF);
        UTF32BE_TO_UTF16LE_STRICT_CHECK("7FDBFFDF", 4, 0x00, 0x0E, 0xFF, 0xFF);
        UTF32BE_TO_UTF16LE_STRICT_CHECK("E0DB01DC", 4, 0x00, 0x10, 0x80, 0x01);
        UTF32BE_TO_UTF16LE_STRICT_CHECK("FFD1", 4, 0x00, 0x00, 0xD1, 0xFF);
        UTF32BE_TO_UTF16LE_STRICT_CHECK("FFFF", 4, 0x00, 0x00, 0xFF, 0xFF);

#define UTF32BE_TO_UTF16LE_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF16Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, false);              \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }
        
        UTF32BE_TO_UTF16LE_CHECK("FFDBFFDF", 4, 0x00, 0x10, 0xFF, 0xFF);
        UTF32BE_TO_UTF16LE_CHECK("7FDBFFDF", 4, 0x00, 0x0E, 0xFF, 0xFF);
        UTF32BE_TO_UTF16LE_CHECK("E0DB01DC", 4, 0x00, 0x10, 0x80, 0x01);
        UTF32BE_TO_UTF16LE_CHECK("FFD1", 4, 0x00, 0x00, 0xD1, 0xFF);
        UTF32BE_TO_UTF16LE_CHECK("FFFF", 4, 0x00, 0x00, 0xFF, 0xFF);
        UTF32BE_TO_UTF16LE_CHECK("3F00", 4, 0x00, 0x11, 0x00, 0x00);
        UTF32BE_TO_UTF16LE_CHECK("3F00", 4, 0x00, 0x00, 0xD8, 0x00);
        UTF32BE_TO_UTF16LE_CHECK("3F00", 4, 0x00, 0x00, 0xDF, 0xFF);
        UTF32BE_TO_UTF16LE_CHECK("3F00", 4, 0xFF, 0xFF, 0xFF, 0xFF);

    }

    void testUTF32LEToUTF16LE()
    {
        {
            char lacBuffer[] = {0xFF, 0xFF, 0x11, 0x00};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0x00, 0xD8, 0x00, 0x00};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0x01, 0xD8, 0x00, 0x00};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }
        
        {
            char lacBuffer[] = {0xFF, 0xDF, 0x00, 0x00};
        
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

#define UTF32LE_TO_UTF16LE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, true);            \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }
        
        UTF32LE_TO_UTF16LE_STRICT_CHECK("FFDBFFDF", 4, 0xFF, 0xFF, 0x10, 0x00);
        UTF32LE_TO_UTF16LE_STRICT_CHECK("7FDBFFDF", 4, 0xFF, 0xFF, 0x0E, 0x00);
        UTF32LE_TO_UTF16LE_STRICT_CHECK("E0DB01DC", 4, 0x01, 0x80, 0x10, 0x00);
        UTF32LE_TO_UTF16LE_STRICT_CHECK("FFD1", 4, 0xFF, 0xD1, 0x00, 0x00);
        UTF32LE_TO_UTF16LE_STRICT_CHECK("FFFF", 4, 0xFF, 0xFF, 0x00, 0x00);

#define UTF32LE_TO_UTF16LE_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF16Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, false);           \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }
        
        UTF32LE_TO_UTF16LE_CHECK("FFDBFFDF", 4, 0xFF, 0xFF, 0x10, 0x00);
        UTF32LE_TO_UTF16LE_CHECK("7FDBFFDF", 4, 0xFF, 0xFF, 0x0E, 0x00);
        UTF32LE_TO_UTF16LE_CHECK("E0DB01DC", 4, 0x01, 0x80, 0x10, 0x00);
        UTF32LE_TO_UTF16LE_CHECK("FFD1", 4, 0xFF, 0xD1, 0x00, 0x00);
        UTF32LE_TO_UTF16LE_CHECK("FFFF", 4, 0xFF, 0xFF, 0x00, 0x00);
        UTF32LE_TO_UTF16LE_CHECK("3F00", 4, 0x00, 0x00, 0x11, 0x00);
        UTF32LE_TO_UTF16LE_CHECK("3F00", 4, 0x00, 0xD8, 0x00, 0x00);
        UTF32LE_TO_UTF16LE_CHECK("3F00", 4, 0xFF, 0xDF, 0x00, 0x00);
        UTF32LE_TO_UTF16LE_CHECK("3F00", 4, 0xFF, 0xFF, 0xFF, 0xFF);

    }

    void testUTF32BEToUTF8()
    {
        {
            char lacBuffer[] = {0x00, 0x11, 0x00, 0x00};
            
            char lacTemp[1024];
            CUTF32ToUTF8Transcoder loTranscoder(lacTemp, sizeof(lacTemp), FROM_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0x00, 0x00, 0xD8, 0x00};

            CMemoryBlock loBuffer(1024);
            CUTF32ToUTF8Transcoder loTranscoder(loBuffer, FROM_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0x00, 0x00, 0xDF, 0xFF};

            CUTF32ToUTF8Transcoder loTranscoder(FROM_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xFF, 0xFF, 0xFF, 0xFF};

            CUTF32ToUTF8Transcoder loTranscoder(FROM_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

#define UTF32BE_TO_UTF8_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF8Transcoder loTranscoder(FROM_BIG_ENDIAN, true);                                  \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF32BE_TO_UTF8_STRICT_CHECK("F48FBFBF", 4, 0x00, 0x10, 0xFF, 0xFF);
        UTF32BE_TO_UTF8_STRICT_CHECK("F0908080", 4, 0x00, 0x01, 0x00, 0x00);
        UTF32BE_TO_UTF8_STRICT_CHECK("EFBFBF", 4, 0x00, 0x00, 0xFF, 0xFF);
        UTF32BE_TO_UTF8_STRICT_CHECK("E0A080", 4, 0x00, 0x00, 0x08, 0x00);
        UTF32BE_TO_UTF8_STRICT_CHECK("DFBF", 4, 0x00, 0x00, 0x07, 0xFF);
        UTF32BE_TO_UTF8_STRICT_CHECK("C280", 4, 0x00, 0x00, 0x00, 0x80);
        UTF32BE_TO_UTF8_STRICT_CHECK("7F", 4, 0x00, 0x00, 0x00, 0x7F);
 
#define UTF32BE_TO_UTF8_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF8Transcoder loTranscoder(FROM_BIG_ENDIAN, false);                                  \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF32BE_TO_UTF8_CHECK("F48FBFBF", 4, 0x00, 0x10, 0xFF, 0xFF);
        UTF32BE_TO_UTF8_CHECK("F0908080", 4, 0x00, 0x01, 0x00, 0x00);
        UTF32BE_TO_UTF8_CHECK("EFBFBF", 4, 0x00, 0x00, 0xFF, 0xFF);
        UTF32BE_TO_UTF8_CHECK("E0A080", 4, 0x00, 0x00, 0x08, 0x00);
        UTF32BE_TO_UTF8_CHECK("DFBF", 4, 0x00, 0x00, 0x07, 0xFF);
        UTF32BE_TO_UTF8_CHECK("C280", 4, 0x00, 0x00, 0x00, 0x80);
        UTF32BE_TO_UTF8_CHECK("7F", 4, 0x00, 0x00, 0x00, 0x7F);
        UTF32BE_TO_UTF8_CHECK("3F", 4, 0x00, 0x11, 0x00, 0x00);
        UTF32BE_TO_UTF8_CHECK("3F", 4, 0x00, 0x00, 0xD8, 0x00);
        UTF32BE_TO_UTF8_CHECK("3F", 4, 0x00, 0x00, 0xDF, 0xFF);
        UTF32BE_TO_UTF8_CHECK("3F", 4, 0xFF, 0xFF, 0xFF, 0xFF);

    }

    void testUTF32LEToUTF8()
    {
        {
            char lacBuffer[] = {0x00, 0x00, 0x11, 0x00};
            
            CUTF32ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0x00, 0xD8, 0x00, 0x00};

            CUTF32ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xFF, 0xDF, 0x00, 0x00};

            CUTF32ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xFF, 0xFF, 0xFF, 0xFF};

            CUTF32ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

#define UTF32LE_TO_UTF8_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, true);                               \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF32LE_TO_UTF8_STRICT_CHECK("F48FBFBF", 4, 0xFF, 0xFF, 0x10, 0x00);
        UTF32LE_TO_UTF8_STRICT_CHECK("F0908080", 4, 0x00, 0x00, 0x01, 0x00);
        UTF32LE_TO_UTF8_STRICT_CHECK("EFBFBF", 4, 0xFF, 0xFF, 0x00, 0x00);
        UTF32LE_TO_UTF8_STRICT_CHECK("E0A080", 4, 0x00, 0x08, 0x00, 0x00);
        UTF32LE_TO_UTF8_STRICT_CHECK("DFBF", 4, 0xFF, 0x07, 0x00, 0x00);
        UTF32LE_TO_UTF8_STRICT_CHECK("C280", 4, 0x80, 0x00, 0x00, 0x00);
        UTF32LE_TO_UTF8_STRICT_CHECK("7F", 4, 0x7F, 0x00, 0x00, 0x00);
 
#define UTF32LE_TO_UTF8_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF32ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, false);                              \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF32LE_TO_UTF8_CHECK("F48FBFBF", 4, 0xFF, 0xFF, 0x10, 0x00);
        UTF32LE_TO_UTF8_CHECK("F0908080", 4, 0x00, 0x00, 0x01, 0x00);
        UTF32LE_TO_UTF8_CHECK("EFBFBF", 4, 0xFF, 0xFF, 0x00, 0x00);
        UTF32LE_TO_UTF8_CHECK("E0A080", 4, 0x00, 0x08, 0x00, 0x00);
        UTF32LE_TO_UTF8_CHECK("DFBF", 4, 0xFF, 0x07, 0x00, 0x00);
        UTF32LE_TO_UTF8_CHECK("C280", 4, 0x80, 0x00, 0x00, 0x00);
        UTF32LE_TO_UTF8_CHECK("7F", 4, 0x7F, 0x00, 0x00, 0x00);
        UTF32LE_TO_UTF8_CHECK("3F", 4, 0x00, 0x00, 0x11, 0x00);
        UTF32LE_TO_UTF8_CHECK("3F", 4, 0x00, 0xD8, 0x00, 0x00);
        UTF32LE_TO_UTF8_CHECK("3F", 4, 0xFF, 0xDF, 0x00, 0x00);
        UTF32LE_TO_UTF8_CHECK("3F", 4, 0xFF, 0xFF, 0xFF, 0xFF);

    }

    void testUTF16BEToUTF8()
    {
         {
            char lacBuffer[] = {0xD8, 0xFF};
            
            CUTF16ToUTF8Transcoder loTranscoder(FROM_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
         }

         {
            char lacBuffer[] = {0xDC, 0xFF};
            
            CUTF16ToUTF8Transcoder loTranscoder(FROM_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
         }

         {
            char lacBuffer[] = {0xD8, 0xFF, 0xFF, 0xFF};
            
            CUTF16ToUTF8Transcoder loTranscoder(FROM_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
         }

         {
            char lacBuffer[] = {0xDC, 0xFF, 0xFF, 0xFF};
            
            CUTF16ToUTF8Transcoder loTranscoder(FROM_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
         }


#define UTF16BE_TO_UTF8_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF8Transcoder loTranscoder(FROM_BIG_ENDIAN, true);                                  \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16BE_TO_UTF8_STRICT_CHECK("F48FBFBF", 4, 0xDB, 0xFF, 0xDF, 0xFF);
        UTF16BE_TO_UTF8_STRICT_CHECK("F0908080", 4, 0xD8, 0x00, 0xDC, 0x00);
        UTF16BE_TO_UTF8_STRICT_CHECK("EFBFBF", 2, 0xFF, 0xFF);
        UTF16BE_TO_UTF8_STRICT_CHECK("E0A080", 2, 0x08, 0x00);
        UTF16BE_TO_UTF8_STRICT_CHECK("DFBF", 2, 0x07, 0xFF);
        UTF16BE_TO_UTF8_STRICT_CHECK("C280", 2, 0x00, 0x80);
        UTF16BE_TO_UTF8_STRICT_CHECK("7F", 2, 0x00, 0x7F);

#define UTF16BE_TO_UTF8_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF8Transcoder loTranscoder(FROM_BIG_ENDIAN, false);                                  \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16BE_TO_UTF8_CHECK("F48FBFBF", 4, 0xDB, 0xFF, 0xDF, 0xFF);
        UTF16BE_TO_UTF8_CHECK("F0908080", 4, 0xD8, 0x00, 0xDC, 0x00);
        UTF16BE_TO_UTF8_CHECK("EFBFBF", 2, 0xFF, 0xFF);
        UTF16BE_TO_UTF8_CHECK("E0A080", 2, 0x08, 0x00);
        UTF16BE_TO_UTF8_CHECK("DFBF", 2, 0x07, 0xFF);
        UTF16BE_TO_UTF8_CHECK("C280", 2, 0x00, 0x80);
        UTF16BE_TO_UTF8_CHECK("7F", 2, 0x00, 0x7F);
        UTF16BE_TO_UTF8_CHECK("3FEFBFBF", 4, 0xD8, 0xFF, 0xFF, 0xFF);
        UTF16BE_TO_UTF8_CHECK("3FEFBFBF", 4, 0xDC, 0xFF, 0xFF, 0xFF);
      
    }

    void testUTF16LEToUTF8()
    {
         {
            char lacBuffer[] = {0xFF, 0xD8};
            
            CUTF16ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
         }

         {
            char lacBuffer[] = {0xFF, 0xDC};
            
            CUTF16ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
         }

         {
            char lacBuffer[] = {0xFF, 0xD8, 0xFF, 0xFF};
            
            CUTF16ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
         }

         {
            char lacBuffer[] = {0xFF, 0xDC, 0xFF, 0xFF};
            
            CUTF16ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
         }


#define UTF16LE_TO_UTF8_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, true);                               \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16LE_TO_UTF8_STRICT_CHECK("F48FBFBF", 4, 0xFF, 0xDB, 0xFF, 0xDF);
        UTF16LE_TO_UTF8_STRICT_CHECK("F0908080", 4, 0x00, 0xD8, 0x00, 0xDC);
        UTF16LE_TO_UTF8_STRICT_CHECK("EFBFBF", 2, 0xFF, 0xFF);
        UTF16LE_TO_UTF8_STRICT_CHECK("E0A080", 2, 0x00, 0x08);
        UTF16LE_TO_UTF8_STRICT_CHECK("DFBF", 2, 0xFF, 0x07);
        UTF16LE_TO_UTF8_STRICT_CHECK("C280", 2, 0x80, 0x00);
        UTF16LE_TO_UTF8_STRICT_CHECK("7F", 2, 0x7F, 0x00);

#define UTF16LE_TO_UTF8_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF8Transcoder loTranscoder(FROM_LITTLE_ENDIAN, false);                               \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16LE_TO_UTF8_CHECK("F48FBFBF", 4, 0xFF, 0xDB, 0xFF, 0xDF);
        UTF16LE_TO_UTF8_CHECK("F0908080", 4, 0x00, 0xD8, 0x00, 0xDC);
        UTF16LE_TO_UTF8_CHECK("EFBFBF", 2, 0xFF, 0xFF);
        UTF16LE_TO_UTF8_CHECK("E0A080", 2, 0x00, 0x08);
        UTF16LE_TO_UTF8_CHECK("DFBF", 2, 0xFF, 0x07);
        UTF16LE_TO_UTF8_CHECK("C280", 2, 0x80, 0x00);
        UTF16LE_TO_UTF8_CHECK("7F", 2, 0x7F, 0x00);
        UTF16LE_TO_UTF8_CHECK("3FEFBFBF", 4, 0xFF, 0xD8, 0xFF, 0xFF);
        UTF16LE_TO_UTF8_CHECK("3FEFBFBF", 4, 0xFF, 0xDC, 0xFF, 0xFF);
    }

    void testUTF8ToUTF32BE()
    {
        {
            char lacBuffer[] = {0xED, 0xA0, 0x80};
    
            char lacTemp[1024];
            CUTF8ToUTF32Transcoder loTranscoder(lacTemp, sizeof(lacTemp), TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 3) == -1);
        }

        {
            char lacBuffer[] = {0xED, 0xBF, 0xBF};
    
            CMemoryBlock loBuffer(1024);
            CUTF8ToUTF32Transcoder loTranscoder(loBuffer, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 3) == -1);
        }

        {
            char lacBuffer[] = {0xF4, 0x90, 0xBF, 0xBF};

            CUTF8ToUTF32Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xF5, 0x8F, 0xBF, 0xBF};

            CUTF8ToUTF32Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xF4, 0x8F, 0xBF, 0x7F};

            CUTF8ToUTF32Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xC0, 0x9F};

            CUTF8ToUTF32Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0x8F};

            CUTF8ToUTF32Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 1) == -1);
        }

        {
            char lacBuffer[] = {0xE0, 0x9F, 0x9F};

            CUTF8ToUTF32Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 3) == -1);
        }

        {
            char lacBuffer[] = {0xF0, 0x8F, 0x9F, 0x9F};

            CUTF8ToUTF32Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }


#define UTF8_TO_UTF32BE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF8ToUTF32Transcoder loTranscoder(TO_BIG_ENDIAN, true);                                    \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF8_TO_UTF32BE_STRICT_CHECK("0010FFFF", 4, 0xF4, 0x8F, 0xBF, 0xBF);
        UTF8_TO_UTF32BE_STRICT_CHECK("00010000", 4, 0xF0, 0x90, 0x80, 0x80);
        UTF8_TO_UTF32BE_STRICT_CHECK("0000FFFF", 3, 0xEF, 0xBF, 0xBF);
        UTF8_TO_UTF32BE_STRICT_CHECK("00000800", 3, 0xE0, 0xA0, 0x80);
        UTF8_TO_UTF32BE_STRICT_CHECK("000007FF", 2, 0xDF, 0xBF);
        UTF8_TO_UTF32BE_STRICT_CHECK("00000080", 2, 0xC2, 0x80);
        UTF8_TO_UTF32BE_STRICT_CHECK("0000007F", 1, 0x7F);

#define UTF8_TO_UTF32BE_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF8ToUTF32Transcoder loTranscoder(TO_BIG_ENDIAN, false);                                   \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF8_TO_UTF32BE_CHECK("0010FFFF", 4, 0xF4, 0x8F, 0xBF, 0xBF);
        UTF8_TO_UTF32BE_CHECK("00010000", 4, 0xF0, 0x90, 0x80, 0x80);
        UTF8_TO_UTF32BE_CHECK("0000FFFF", 3, 0xEF, 0xBF, 0xBF);
        UTF8_TO_UTF32BE_CHECK("00000800", 3, 0xE0, 0xA0, 0x80);
        UTF8_TO_UTF32BE_CHECK("000007FF", 2, 0xDF, 0xBF);
        UTF8_TO_UTF32BE_CHECK("00000080", 2, 0xC2, 0x80);
        UTF8_TO_UTF32BE_CHECK("0000007F", 1, 0x7F);
        UTF8_TO_UTF32BE_CHECK("0000003F", 3, 0xED, 0xA0, 0x80);
        UTF8_TO_UTF32BE_CHECK("0000003F", 3, 0xED, 0xBF, 0xBF);
        UTF8_TO_UTF32BE_CHECK("0000003F", 4, 0xF4, 0x90, 0xBF, 0xBF);
        UTF8_TO_UTF32BE_CHECK("0000003F", 4, 0xF5, 0x8F, 0xBF, 0xBF);
        UTF8_TO_UTF32BE_CHECK("0000003F", 4, 0xF4, 0x8F, 0xBF, 0x7F);
        UTF8_TO_UTF32BE_CHECK("0000003F", 2, 0xC0, 0x9F);
        UTF8_TO_UTF32BE_CHECK("0000003F", 1, 0x8F);
        UTF8_TO_UTF32BE_CHECK("0000003F", 3, 0xE0, 0x9F, 0x9F);
        UTF8_TO_UTF32BE_CHECK("0000003F", 4, 0xF0, 0x8F, 0x9F, 0x9F);

    }

    void testUTF8ToUTF32LE()
    {
        {
            char lacBuffer[] = {0xED, 0xA0, 0x80};

            CUTF8ToUTF32Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 3) == -1);
        }

        {
            char lacBuffer[] = {0xED, 0xBF, 0xBF};

            CUTF8ToUTF32Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 3) == -1);
        }

        {
            char lacBuffer[] = {0xF4, 0x90, 0xBF, 0xBF};

            CUTF8ToUTF32Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xF5, 0x8F, 0xBF, 0xBF};

            CUTF8ToUTF32Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xF4, 0x8F, 0xBF, 0x7F};

            CUTF8ToUTF32Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xC0, 0x9F};

            CUTF8ToUTF32Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0x8F};

            CUTF8ToUTF32Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 1) == -1);
        }

        {
            char lacBuffer[] = {0xE0, 0x9F, 0x9F};

            CUTF8ToUTF32Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 3) == -1);
        }

        {
            char lacBuffer[] = {0xF0, 0x8F, 0x9F, 0x9F};

            CUTF8ToUTF32Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

       
#define UTF8_TO_UTF32LE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF8ToUTF32Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);                                 \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF8_TO_UTF32LE_STRICT_CHECK("FFFF1000", 4, 0xF4, 0x8F, 0xBF, 0xBF);
        UTF8_TO_UTF32LE_STRICT_CHECK("00000100", 4, 0xF0, 0x90, 0x80, 0x80);
        UTF8_TO_UTF32LE_STRICT_CHECK("FFFF0000", 3, 0xEF, 0xBF, 0xBF);
        UTF8_TO_UTF32LE_STRICT_CHECK("00080000", 3, 0xE0, 0xA0, 0x80);
        UTF8_TO_UTF32LE_STRICT_CHECK("FF070000", 2, 0xDF, 0xBF);
        UTF8_TO_UTF32LE_STRICT_CHECK("80000000", 2, 0xC2, 0x80);
        UTF8_TO_UTF32LE_STRICT_CHECK("7F000000", 1, 0x7F);

#define UTF8_TO_UTF32LE_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF8ToUTF32Transcoder loTranscoder(TO_LITTLE_ENDIAN, false);                                \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF8_TO_UTF32LE_CHECK("FFFF1000", 4, 0xF4, 0x8F, 0xBF, 0xBF);
        UTF8_TO_UTF32LE_CHECK("00000100", 4, 0xF0, 0x90, 0x80, 0x80);
        UTF8_TO_UTF32LE_CHECK("FFFF0000", 3, 0xEF, 0xBF, 0xBF);
        UTF8_TO_UTF32LE_CHECK("00080000", 3, 0xE0, 0xA0, 0x80);
        UTF8_TO_UTF32LE_CHECK("FF070000", 2, 0xDF, 0xBF);
        UTF8_TO_UTF32LE_CHECK("80000000", 2, 0xC2, 0x80);
        UTF8_TO_UTF32LE_CHECK("7F000000", 1, 0x7F);
        UTF8_TO_UTF32LE_CHECK("3F000000", 3, 0xED, 0xA0, 0x80);
        UTF8_TO_UTF32LE_CHECK("3F000000", 3, 0xED, 0xBF, 0xBF);
        UTF8_TO_UTF32LE_CHECK("3F000000", 4, 0xF4, 0x90, 0xBF, 0xBF);
        UTF8_TO_UTF32LE_CHECK("3F000000", 4, 0xF5, 0x8F, 0xBF, 0xBF);
        UTF8_TO_UTF32LE_CHECK("3F000000", 4, 0xF4, 0x8F, 0xBF, 0x7F);
        UTF8_TO_UTF32LE_CHECK("3F000000", 2, 0xC0, 0x9F);
        UTF8_TO_UTF32LE_CHECK("3F000000", 1, 0x8F);
        UTF8_TO_UTF32LE_CHECK("3F000000", 3, 0xE0, 0x9F, 0x9F);
        UTF8_TO_UTF32LE_CHECK("3F000000", 4, 0xF0, 0x8F, 0x9F, 0x9F);


    }

    void testUTF8ToUTF16BE()
    {
        {
            char lacBuffer[] = {0xED, 0xA0, 0x80};

            CUTF8ToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 3) == -1);
        }

        {
            char lacBuffer[] = {0xED, 0xBF, 0xBF};

            CUTF8ToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 3) == -1);
        }

        {
            char lacBuffer[] = {0xF4, 0x90, 0xBF, 0xBF};

            CUTF8ToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xF5, 0x8F, 0xBF, 0xBF};

            CUTF8ToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xF4, 0x8F, 0xBF, 0x7F};

            CUTF8ToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

        {
            char lacBuffer[] = {0xC0, 0x9F};

            CUTF8ToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0x8F};

            CUTF8ToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 1) == -1);
        }

        {
            char lacBuffer[] = {0xE0, 0x9F, 0x9F};

            CUTF8ToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 3) == -1);
        }

        {
            char lacBuffer[] = {0xF0, 0x8F, 0x9F, 0x9F};

            CUTF8ToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }


#define UTF8_TO_UTF16BE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF8ToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);                                    \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF8_TO_UTF16BE_STRICT_CHECK("DBFFDFFF", 4, 0xF4, 0x8F, 0xBF, 0xBF);
        UTF8_TO_UTF16BE_STRICT_CHECK("D800DC00", 4, 0xF0, 0x90, 0x80, 0x80);
        UTF8_TO_UTF16BE_STRICT_CHECK("DBFFDFFFD800DC00", 8, 0xF4, 0x8F, 0xBF, 0xBF, 0xF0, 0x90, 0x80, 0x80);
        UTF8_TO_UTF16BE_STRICT_CHECK("FFFF", 3, 0xEF, 0xBF, 0xBF);
        UTF8_TO_UTF16BE_STRICT_CHECK("0800", 3, 0xE0, 0xA0, 0x80);
        UTF8_TO_UTF16BE_STRICT_CHECK("07FF", 2, 0xDF, 0xBF);
        UTF8_TO_UTF16BE_STRICT_CHECK("0080", 2, 0xC2, 0x80);
        UTF8_TO_UTF16BE_STRICT_CHECK("007F", 1, 0x7F);

#define UTF8_TO_UTF16BE_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF8ToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, false);                                   \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF8_TO_UTF16BE_CHECK("DBFFDFFF", 4, 0xF4, 0x8F, 0xBF, 0xBF);
        UTF8_TO_UTF16BE_CHECK("D800DC00", 4, 0xF0, 0x90, 0x80, 0x80);
        UTF8_TO_UTF16BE_CHECK("FFFF", 3, 0xEF, 0xBF, 0xBF);
        UTF8_TO_UTF16BE_CHECK("0800", 3, 0xE0, 0xA0, 0x80);
        UTF8_TO_UTF16BE_CHECK("07FF", 2, 0xDF, 0xBF);
        UTF8_TO_UTF16BE_CHECK("0080", 2, 0xC2, 0x80);
        UTF8_TO_UTF16BE_CHECK("007F", 1, 0x7F);
        UTF8_TO_UTF16BE_CHECK("003F", 3, 0xED, 0xA0, 0x80);
        UTF8_TO_UTF16BE_CHECK("003F", 3, 0xED, 0xBF, 0xBF);
        UTF8_TO_UTF16BE_CHECK("003F", 4, 0xF4, 0x90, 0xBF, 0xBF);
        UTF8_TO_UTF16BE_CHECK("003F", 4, 0xF5, 0x8F, 0xBF, 0xBF);
        UTF8_TO_UTF16BE_CHECK("003F", 4, 0xF4, 0x8F, 0xBF, 0x7F);
        UTF8_TO_UTF16BE_CHECK("003F", 2, 0xC0, 0x9F);
        UTF8_TO_UTF16BE_CHECK("003F", 1, 0x8F);
        UTF8_TO_UTF16BE_CHECK("003F", 3, 0xE0, 0x9F, 0x9F);
        UTF8_TO_UTF16BE_CHECK("003F", 4, 0xF0, 0x8F, 0x9F, 0x9F);
    }

    void testUTF8ToUTF16LE()
    {
#define UTF8_TO_UTF16LE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF8ToUTF16Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);                                 \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF8_TO_UTF16LE_STRICT_CHECK("FFDBFFDF", 4, 0xF4, 0x8F, 0xBF, 0xBF);
        UTF8_TO_UTF16LE_STRICT_CHECK("00D800DC", 4, 0xF0, 0x90, 0x80, 0x80);
        UTF8_TO_UTF16LE_STRICT_CHECK("FFFF", 3, 0xEF, 0xBF, 0xBF);
        UTF8_TO_UTF16LE_STRICT_CHECK("0008", 3, 0xE0, 0xA0, 0x80);
        UTF8_TO_UTF16LE_STRICT_CHECK("FF07", 2, 0xDF, 0xBF);
        UTF8_TO_UTF16LE_STRICT_CHECK("8000", 2, 0xC2, 0x80);
        UTF8_TO_UTF16LE_STRICT_CHECK("7F00", 1, 0x7F);
    }

    void testUTF16BEToUTF32BE()
    {
        {
            char lacBuffer[] = {0xD8, 0x00};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xDC, 0x00};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xDF, 0xFF};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xD8, 0xFF, 0xD9, 0xFF};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

#define UTF16BE_TO_UTF32BE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_BIG_ENDIAN, true);                  \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16BE_TO_UTF32BE_STRICT_CHECK("0010FFFF", 4, 0xDB, 0xFF, 0xDF, 0xFF);
        UTF16BE_TO_UTF32BE_STRICT_CHECK("00010000", 4, 0xD8, 0x00, 0xDC, 0x00);
        UTF16BE_TO_UTF32BE_STRICT_CHECK("0000D700", 2, 0xD7, 0x00);
        UTF16BE_TO_UTF32BE_STRICT_CHECK("0000E000", 2, 0xE0, 0x00);
        UTF16BE_TO_UTF32BE_STRICT_CHECK("0000FFFF", 2, 0xFF, 0xFF);

#define UTF16BE_TO_UTF32BE_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_BIG_ENDIAN, false);                 \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16BE_TO_UTF32BE_CHECK("0010FFFF", 4, 0xDB, 0xFF, 0xDF, 0xFF);
        UTF16BE_TO_UTF32BE_CHECK("00010000", 4, 0xD8, 0x00, 0xDC, 0x00);
        UTF16BE_TO_UTF32BE_CHECK("0000D700", 2, 0xD7, 0x00);
        UTF16BE_TO_UTF32BE_CHECK("0000E000", 2, 0xE0, 0x00);
        UTF16BE_TO_UTF32BE_CHECK("0000FFFF", 2, 0xFF, 0xFF);
        UTF16BE_TO_UTF32BE_CHECK("0000003F", 2, 0xD8, 0x00);
        UTF16BE_TO_UTF32BE_CHECK("0000003F", 2, 0xDC, 0x00);
        UTF16BE_TO_UTF32BE_CHECK("0000003F", 2, 0xDF, 0xFF);
        UTF16BE_TO_UTF32BE_CHECK("0000003F0000003F", 4, 0xD8, 0xFF, 0xD9, 0xFF);
    }

    void testUTF16BEToUTF32LE()
    {
        {
            char lacBuffer[] = {0xD8, 0x00};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xDC, 0x00};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xDF, 0xFF};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xD8, 0xFF, 0xD9, 0xFF};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

#define UTF16BE_TO_UTF32LE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, true);               \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16BE_TO_UTF32LE_STRICT_CHECK("FFFF1000", 4, 0xDB, 0xFF, 0xDF, 0xFF);
        UTF16BE_TO_UTF32LE_STRICT_CHECK("00000100", 4, 0xD8, 0x00, 0xDC, 0x00);
        UTF16BE_TO_UTF32LE_STRICT_CHECK("00D70000", 2, 0xD7, 0x00);
        UTF16BE_TO_UTF32LE_STRICT_CHECK("00E00000", 2, 0xE0, 0x00);
        UTF16BE_TO_UTF32LE_STRICT_CHECK("FFFF0000", 2, 0xFF, 0xFF);

#define UTF16BE_TO_UTF32LE_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF32Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_LITTLE_ENDIAN, false);              \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16BE_TO_UTF32LE_CHECK("FFFF1000", 4, 0xDB, 0xFF, 0xDF, 0xFF);
        UTF16BE_TO_UTF32LE_CHECK("00000100", 4, 0xD8, 0x00, 0xDC, 0x00);
        UTF16BE_TO_UTF32LE_CHECK("00D70000", 2, 0xD7, 0x00);
        UTF16BE_TO_UTF32LE_CHECK("00E00000", 2, 0xE0, 0x00);
        UTF16BE_TO_UTF32LE_CHECK("FFFF0000", 2, 0xFF, 0xFF);
        UTF16BE_TO_UTF32LE_CHECK("3F000000", 2, 0xD8, 0x00);
        UTF16BE_TO_UTF32LE_CHECK("3F000000", 2, 0xDC, 0x00);
        UTF16BE_TO_UTF32LE_CHECK("3F000000", 2, 0xDF, 0xFF);
        UTF16BE_TO_UTF32LE_CHECK("3F0000003F000000", 4, 0xD8, 0xFF, 0xD9, 0xFF);
 
    }

    void testUTF16LEToUTF32BE()
    {
        {
            char lacBuffer[] = {0x00, 0xD8};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0x00, 0xDC};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xFF, 0xDF};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xFF, 0xD8, 0xFF, 0xD9};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

#define UTF16LE_TO_UTF32BE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, true);               \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16LE_TO_UTF32BE_STRICT_CHECK("0010FFFF", 4, 0xFF, 0xDB, 0xFF, 0xDF);
        UTF16LE_TO_UTF32BE_STRICT_CHECK("00010000", 4, 0x00, 0xD8, 0x00, 0xDC);
        UTF16LE_TO_UTF32BE_STRICT_CHECK("0000D700", 2, 0x00, 0xD7);
        UTF16LE_TO_UTF32BE_STRICT_CHECK("0000E000", 2, 0x00, 0xE0);
        UTF16LE_TO_UTF32BE_STRICT_CHECK("0000FFFF", 2, 0xFF, 0xFF);

#define UTF16LE_TO_UTF32BE_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_BIG_ENDIAN, false);             \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16LE_TO_UTF32BE_CHECK("0010FFFF", 4, 0xFF, 0xDB, 0xFF, 0xDF);
        UTF16LE_TO_UTF32BE_CHECK("00010000", 4, 0x00, 0xD8, 0x00, 0xDC);
        UTF16LE_TO_UTF32BE_CHECK("0000D700", 2, 0x00, 0xD7);
        UTF16LE_TO_UTF32BE_CHECK("0000E000", 2, 0x00, 0xE0);
        UTF16LE_TO_UTF32BE_CHECK("0000FFFF", 2, 0xFF, 0xFF);
        UTF16LE_TO_UTF32BE_CHECK("0000003F", 2, 0x00, 0xD8);
        UTF16LE_TO_UTF32BE_CHECK("0000003F", 2, 0x00, 0xDC);
        UTF16LE_TO_UTF32BE_CHECK("0000003F", 2, 0xFF, 0xDF);
        UTF16LE_TO_UTF32BE_CHECK("0000003F0000003F", 4, 0xFF, 0xD8, 0xFF, 0xD9);
    }

    void testUTF16LEToUTF32LE()
    {
        {
            char lacBuffer[] = {0x00, 0xD8};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0x00, 0xDC};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xFF, 0xDF};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xFF, 0xD8, 0xFF, 0xD9};

            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 4) == -1);
        }

#define UTF16LE_TO_UTF32LE_STRICT_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, true);            \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16LE_TO_UTF32LE_STRICT_CHECK("FFFF1000", 4, 0xFF, 0xDB, 0xFF, 0xDF);
        UTF16LE_TO_UTF32LE_STRICT_CHECK("00000100", 4, 0x00, 0xD8, 0x00, 0xDC);
        UTF16LE_TO_UTF32LE_STRICT_CHECK("00D70000", 2, 0x00, 0xD7);
        UTF16LE_TO_UTF32LE_STRICT_CHECK("00E00000", 2, 0x00, 0xE0);
        UTF16LE_TO_UTF32LE_STRICT_CHECK("FFFF0000", 2, 0xFF, 0xFF);

#define UTF16LE_TO_UTF32LE_CHECK(val, size, ...)                                                         \
        {                                                                                                \
            char lacBuffer[] = {__VA_ARGS__};                                                            \
                                                                                                         \
            acl::CHexEncoder loEncoder;                                                                  \
            CUTF16ToUTF32Transcoder loTranscoder(FROM_LITTLE_ENDIAN, TO_LITTLE_ENDIAN, false);           \
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, size) == 0);                                    \
            ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);   \
            ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), val) == 0);                                 \
        }

        UTF16LE_TO_UTF32LE_CHECK("FFFF1000", 4, 0xFF, 0xDB, 0xFF, 0xDF);
        UTF16LE_TO_UTF32LE_CHECK("00000100", 4, 0x00, 0xD8, 0x00, 0xDC);
        UTF16LE_TO_UTF32LE_CHECK("00D70000", 2, 0x00, 0xD7);
        UTF16LE_TO_UTF32LE_CHECK("00E00000", 2, 0x00, 0xE0);
        UTF16LE_TO_UTF32LE_CHECK("FFFF0000", 2, 0xFF, 0xFF);
        UTF16LE_TO_UTF32LE_CHECK("3F000000", 2, 0x00, 0xD8);
        UTF16LE_TO_UTF32LE_CHECK("3F000000", 2, 0x00, 0xDC);
        UTF16LE_TO_UTF32LE_CHECK("3F000000", 2, 0xFF, 0xDF);
        UTF16LE_TO_UTF32LE_CHECK("3F0000003F000000", 4, 0xFF, 0xD8, 0xFF, 0xD9);
 
    }

    void testGBKToUTF16BE()
    {
        acl::CFileStream loFileGBK, loFileUnicode;
        std::map<std::string, std::string> loGBKToUnicode;
        unsigned char lacGBKcode[2] = { 0 };
        unsigned char lacUnicode[2] = { 0 };
        char lacBit[5] = { 0 };

        for ( CGBKAndUnicodeTable::iterator loIter = CGBKAndUnicodeTable::Instance()->Begin();
            loIter != CGBKAndUnicodeTable::Instance()->End(); ++loIter )
        {
            loGBKToUnicode[loIter->gbk] = loIter->unicode;
        }

        {
            char lacBuffer[] = {0x81, 0x39};

            CGBKToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xFE, 0xA0};

            CGBKToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        std::map<std::string, std::string>::iterator loIter;
        CGBKToUTF16Transcoder loTranscoder(TO_BIG_ENDIAN, true);
        
        lacBit[0] = '0';
        lacBit[1] = 'x';
        //apl_int_t i = 0;
        for(loIter = loGBKToUnicode.begin();loIter != loGBKToUnicode.end();loIter++)
        {
            lacBit[2] = loIter->first[0];
            lacBit[3] = loIter->first[1];
            lacGBKcode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->first[2];
            lacBit[3] = loIter->first[3];
            lacGBKcode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            lacBit[2] = loIter->second[0];
            lacBit[3] = loIter->second[1];
            lacUnicode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->second[2];
            lacBit[3] = loIter->second[3];
            lacUnicode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            if(loTranscoder.Final(lacGBKcode, sizeof(lacGBKcode)) != 0)
            {
//                printf("%x%x = %x%x\n", lacGBKcode[0], lacGBKcode[1], lacUnicode[1], lacUnicode[0]);
                continue;
            } 

            ASSERT_MESSAGE(loTranscoder.GetLength() == sizeof(lacUnicode));
            ASSERT_MESSAGE(apl_memcmp(lacUnicode, loTranscoder.GetOutput(), sizeof(lacUnicode)) == 0);

//            i++;
//            printf("%d\n", i);
        }

    }

    void testGBKToUTF16LE()
    {
        acl::CFileStream loFileGBK, loFileUnicode;
        std::map<std::string, std::string> loGBKToUnicode;
        unsigned char lacGBKcode[2] = { 0 };
        unsigned char lacUnicode[2] = { 0 };
        char lacBit[5] = { 0 };
        
        for ( CGBKAndUnicodeTable::iterator loIter = CGBKAndUnicodeTable::Instance()->Begin();
            loIter != CGBKAndUnicodeTable::Instance()->End(); ++loIter )
        {
            loGBKToUnicode[loIter->gbk] = loIter->unicode;
        }

        loFileGBK.Close();
        loFileUnicode.Close();

        {
            char lacBuffer[] = {0x81, 0x39};

            CGBKToUTF16Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        {
            char lacBuffer[] = {0xFE, 0xA0};

            CGBKToUTF16Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
            ASSERT_MESSAGE(loTranscoder.Final(lacBuffer, 2) == -1);
        }

        std::map<std::string, std::string>::iterator loIter;
        CGBKToUTF16Transcoder loTranscoder(TO_LITTLE_ENDIAN, true);
        
        lacBit[0] = '0';
        lacBit[1] = 'x';
        //apl_int_t i = 0;
        for(loIter = loGBKToUnicode.begin();loIter != loGBKToUnicode.end();loIter++)
        {
            lacBit[2] = loIter->first[0];
            lacBit[3] = loIter->first[1];
            lacGBKcode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->first[2];
            lacBit[3] = loIter->first[3];
            lacGBKcode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            lacBit[2] = loIter->second[0];
            lacBit[3] = loIter->second[1];
            lacUnicode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->second[2];
            lacBit[3] = loIter->second[3];
            lacUnicode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            if(loTranscoder.Final(lacGBKcode, sizeof(lacGBKcode)) != 0)
            {
//                printf("%x%x = %x%x\n", lacGBKcode[0], lacGBKcode[1], lacUnicode[0], lacUnicode[1]);
                continue;
            } 

            ASSERT_MESSAGE(loTranscoder.GetLength() == sizeof(lacUnicode));
            ASSERT_MESSAGE(apl_memcmp(lacUnicode, loTranscoder.GetOutput(), sizeof(lacUnicode)) == 0);

            //i++;
            //printf("%d\n", i);
        }

    }

    void testUTF16BEToGBK()
    {
        acl::CFileStream loFileGBK, loFileUnicode;
        std::map<std::string, std::string> loGBKToUnicode;
        unsigned char lacGBKcode[2] = { 0 };
        unsigned char lacUnicode[2] = { 0 };
        char lacBit[5] = { 0 };

        for ( CGBKAndUnicodeTable::iterator loIter = CGBKAndUnicodeTable::Instance()->Begin();
            loIter != CGBKAndUnicodeTable::Instance()->End(); ++loIter )
        {
            loGBKToUnicode[loIter->gbk] = loIter->unicode;
        }

        std::map<std::string, std::string>::iterator loIter;
        CUTF16ToGBKTranscoder loTranscoder(FROM_BIG_ENDIAN, true);
        
        lacBit[0] = '0';
        lacBit[1] = 'x';
        //apl_int_t i = 0;
        for(loIter = loGBKToUnicode.begin();loIter != loGBKToUnicode.end();loIter++)
        {
            lacBit[2] = loIter->first[0];
            lacBit[3] = loIter->first[1];
            lacGBKcode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->first[2];
            lacBit[3] = loIter->first[3];
            lacGBKcode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            lacBit[2] = loIter->second[0];
            lacBit[3] = loIter->second[1];
            lacUnicode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->second[2];
            lacBit[3] = loIter->second[3];
            lacUnicode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            if(loTranscoder.Final(lacUnicode, sizeof(lacUnicode)) != 0)
            {
//                printf("%x%x = %x%x\n", lacGBKcode[0], lacGBKcode[1], lacUnicode[1], lacUnicode[0]);
                continue;
            } 

            ASSERT_MESSAGE(loTranscoder.GetLength() == sizeof(lacGBKcode));
            ASSERT_MESSAGE(apl_memcmp(lacGBKcode, loTranscoder.GetOutput(), sizeof(lacGBKcode)) == 0);

            //i++;
            //printf("%d\n", i);
        }

    }

    void testUTF16LEToGBK()
    {
        acl::CFileStream loFileGBK, loFileUnicode;
        std::map<std::string, std::string> loGBKToUnicode;
        unsigned char lacGBKcode[2] = { 0 };
        unsigned char lacUnicode[2] = { 0 };
        char lacBit[5] = { 0 };

        for ( CGBKAndUnicodeTable::iterator loIter = CGBKAndUnicodeTable::Instance()->Begin();
            loIter != CGBKAndUnicodeTable::Instance()->End(); ++loIter )
        {
            loGBKToUnicode[loIter->gbk] = loIter->unicode;
        }

        std::map<std::string, std::string>::iterator loIter;
        CUTF16ToGBKTranscoder loTranscoder(FROM_LITTLE_ENDIAN, true);
        
        lacBit[0] = '0';
        lacBit[1] = 'x';
        //apl_int_t i = 0;
        for(loIter = loGBKToUnicode.begin();loIter != loGBKToUnicode.end();loIter++)
        {
            lacBit[2] = loIter->first[0];
            lacBit[3] = loIter->first[1];
            lacGBKcode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->first[2];
            lacBit[3] = loIter->first[3];
            lacGBKcode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            lacBit[2] = loIter->second[0];
            lacBit[3] = loIter->second[1];
            lacUnicode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->second[2];
            lacBit[3] = loIter->second[3];
            lacUnicode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            if(loTranscoder.Final(lacUnicode, sizeof(lacUnicode)) != 0)
            {
//                printf("%x%x = %x%x\n", lacGBKcode[0], lacGBKcode[1], lacUnicode[1], lacUnicode[0]);
                continue;
            } 

            ASSERT_MESSAGE(loTranscoder.GetLength() == sizeof(lacGBKcode));
            ASSERT_MESSAGE(apl_memcmp(lacGBKcode, loTranscoder.GetOutput(), sizeof(lacGBKcode)) == 0);

            //i++;
            //printf("%d\n", i);
        }
    }

    void testGBKToUTF8(void)
    {
        acl::CFileStream loFileGBK, loFileUnicode;
        std::map<std::string, std::string> loGBKToUnicode;
        unsigned char lacGBKcode[2] = { 0 };
        unsigned char lacUnicode[2] = { 0 };
        char lacBit[5] = { 0 };
        
        for ( CGBKAndUnicodeTable::iterator loIter = CGBKAndUnicodeTable::Instance()->Begin();
            loIter != CGBKAndUnicodeTable::Instance()->End(); ++loIter )
        {
            loGBKToUnicode[loIter->gbk] = loIter->unicode;
        }

        loFileGBK.Close();
        loFileUnicode.Close();

        std::map<std::string, std::string>::iterator loIter;
        CGBKToUTF8Transcoder   loTranscoder1(true);
        CUTF16ToUTF8Transcoder loTranscoder2(FROM_LITTLE_ENDIAN, true);
        
        lacBit[0] = '0';
        lacBit[1] = 'x';
        //apl_int_t i = 0;
        for(loIter = loGBKToUnicode.begin();loIter != loGBKToUnicode.end();loIter++)
        {
            lacBit[2] = loIter->first[0];
            lacBit[3] = loIter->first[1];
            lacGBKcode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->first[2];
            lacBit[3] = loIter->first[3];
            lacGBKcode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            lacBit[2] = loIter->second[0];
            lacBit[3] = loIter->second[1];
            lacUnicode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->second[2];
            lacBit[3] = loIter->second[3];
            lacUnicode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            if(loTranscoder1.Final(lacGBKcode, sizeof(lacGBKcode)) != 0)
            {
//                printf("%x%x = %x%x\n", lacGBKcode[0], lacGBKcode[1], lacUnicode[0], lacUnicode[1]);
                continue;
            } 

            if(loTranscoder2.Final(lacUnicode, sizeof(lacUnicode)) != 0)
            {
//                printf("%x%x = %x%x\n", lacGBKcode[0], lacGBKcode[1], lacUnicode[0], lacUnicode[1]);
                continue;
            } 

            ASSERT_MESSAGE(loTranscoder1.GetLength() == loTranscoder2.GetLength() );
            ASSERT_MESSAGE(apl_memcmp(loTranscoder2.GetOutput(), loTranscoder1.GetOutput(), loTranscoder2.GetLength() ) == 0);
            //i++;
            //printf("%d\n", i);
        }
    }

    void testUTF8ToGBK(void)
    {
        acl::CFileStream loFileGBK, loFileUnicode;
        std::map<std::string, std::string> loGBKToUnicode;
        unsigned char lacGBKcode[2] = { 0 };
        unsigned char lacUnicode[2] = { 0 };
        char lacBit[5] = { 0 };

        for ( CGBKAndUnicodeTable::iterator loIter = CGBKAndUnicodeTable::Instance()->Begin();
            loIter != CGBKAndUnicodeTable::Instance()->End(); ++loIter )
        {
            loGBKToUnicode[loIter->gbk] = loIter->unicode;
        }

        std::map<std::string, std::string>::iterator loIter;
        CUTF8ToGBKTranscoder loTranscoder1(true);
        CUTF16ToUTF8Transcoder loTranscoder2(FROM_BIG_ENDIAN, true);
        
        lacBit[0] = '0';
        lacBit[1] = 'x';
        //apl_int_t i = 0;
        for(loIter = loGBKToUnicode.begin();loIter != loGBKToUnicode.end();loIter++)
        {
            lacBit[2] = loIter->first[0];
            lacBit[3] = loIter->first[1];
            lacGBKcode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->first[2];
            lacBit[3] = loIter->first[3];
            lacGBKcode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            lacBit[2] = loIter->second[0];
            lacBit[3] = loIter->second[1];
            lacUnicode[0] = (unsigned char)apl_strtou32(lacBit, NULL, 16);
            lacBit[2] = loIter->second[2];
            lacBit[3] = loIter->second[3];
            lacUnicode[1] = (unsigned char)apl_strtou32(lacBit, NULL, 16);

            if(loTranscoder2.Final(lacUnicode, sizeof(lacUnicode)) != 0)
            {
//                printf("%x%x = %x%x\n", lacGBKcode[0], lacGBKcode[1], lacUnicode[1], lacUnicode[0]);
                continue;
            } 

            if(loTranscoder1.Final(loTranscoder2.GetOutput(), loTranscoder2.GetLength() ) != 0)
            {
//                printf("%x%x = %x%x\n", lacGBKcode[0], lacGBKcode[1], lacUnicode[1], lacUnicode[0]);
                continue;
            } 

            ASSERT_MESSAGE(loTranscoder1.GetLength() == sizeof(lacGBKcode));
            ASSERT_MESSAGE(apl_memcmp(lacGBKcode, loTranscoder1.GetOutput(), sizeof(lacGBKcode)) == 0);

            //i++;
            //printf("%d\n", i);
        }
    }
    
    void testTranscoderUpdate(void)
    {
        acl::CHexEncoder loEncoder;
        CUTF32ToUTF16Transcoder loTranscoder(FROM_BIG_ENDIAN, TO_BIG_ENDIAN, true);
        
        {
            char lacBuffer[] = {0x00};
            ASSERT_MESSAGE(loTranscoder.Update(lacBuffer, 1) == 0);
        }
        
        {
            char lacBuffer[] = {0x10, 0xFF, 0xFF};
            ASSERT_MESSAGE(loTranscoder.Update(lacBuffer, 3) == 0);
        }
        
        {
            char lacBuffer[] = {0x00, 0x00};
            ASSERT_MESSAGE(loTranscoder.Update(lacBuffer, 2) == 0);
        }
        
        {
            char lacBuffer[] = {0xD1, 0xFF};
            ASSERT_MESSAGE(loTranscoder.Update(lacBuffer, 2) == 0);
        }
        
        ASSERT_MESSAGE(loTranscoder.Final() == 0);
        ASSERT_MESSAGE(loEncoder.Final(loTranscoder.GetOutput(), loTranscoder.GetLength() ) == 0);
        ASSERT_MESSAGE(apl_strcmp(loEncoder.GetOutput(), "DBFFDFFFD1FF") == 0);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclCodecsUnicode);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
