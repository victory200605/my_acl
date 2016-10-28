#include "Test.h"
#include "acl/FileStream.h"

using namespace acl;

class CTestAclFileStream : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclFileStream);
    CPPUNIT_TEST(testReadLine1);
    CPPUNIT_TEST(testReadLine2);
    CPPUNIT_TEST(testReadLine3);
    CPPUNIT_TEST(testReadLine4);
    CPPUNIT_TEST(testReadLine5);
    CPPUNIT_TEST(testReadLine6);
    CPPUNIT_TEST(testReadLine7);
    CPPUNIT_TEST(testIsEof);
    CPPUNIT_TEST_SUITE_END();
    
public:
    void setUp(void) {}
    void tearDown(void) {}

    void testReadLine1(void)
    {
        CFileStream  loFileStream;
        char lacBuffer[1024] = {0};

        ASSERT_MESSAGE( loFileStream.Open("tmp.txt", O_CREAT | O_RDWR, 0755) == 0);
        
        ASSERT_MESSAGE( loFileStream.Write(
            "1234567890\r\n1234567890\n1234567890", 
            apl_strlen("1234567890\r\n1234567890\n1234567890") ) == (apl_ssize_t)apl_strlen("1234567890\r\n1234567890\n1234567890") );

        loFileStream.Sync();
        loFileStream.Seek(0);
        
        loFileStream.SetBufferSize(1024);
        
        //case 1
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 20) == 10);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "1234567890") == 0);
        ASSERT_MESSAGE(loFileStream.Tell() == 12);
        
        //case 2
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 6) == 5);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "12345") == 0);
        ASSERT_MESSAGE(loFileStream.Tell() == 17);
        
        //case 2
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 6) == 5);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "67890") == 0);
        ASSERT_MESSAGE(loFileStream.Tell() == 23);
        
        //case 3
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 20) == 10);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "1234567890") == 0);
        ASSERT_MESSAGE(loFileStream.Tell() == 33);
        
        //case 4
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 10) == -1);
        ASSERT_MESSAGE(loFileStream.Tell() == 33);
    }
    
    void testReadLine2(void)
    {
        CFileStream  loFileStream;
        char lacBuffer[1024] = {0};
        
        ASSERT_MESSAGE( loFileStream.Open("tmp.txt", O_CREAT | O_RDWR, 0755) == 0);
        
        ASSERT_MESSAGE( loFileStream.Write(
            "1234567890\r\n1234567890\n1234567890", 
            apl_strlen("1234567890\r\n1234567890\n1234567890") ) == (apl_ssize_t)apl_strlen("1234567890\r\n1234567890\n1234567890") );
        
        loFileStream.Sync();
        loFileStream.Seek(0);
        
        loFileStream.SetBufferSize(5);
        
        //case 1
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 20) == 10);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "1234567890") == 0);
        ASSERT_MESSAGE(loFileStream.Tell() == 12);
        
        //case 2
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 6) == 5);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "12345") == 0);
        ASSERT_MESSAGE(loFileStream.Tell() == 17);
        
        
        //case 2
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 6) == 5);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "67890") == 0);
        ASSERT_MESSAGE(loFileStream.Tell() == 23);
        
        //case 3
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 20) == 10);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "1234567890") == 0);
        ASSERT_MESSAGE(loFileStream.Tell() == 33);
        
        //case 4
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 10) == -1);
        ASSERT_MESSAGE(loFileStream.Tell() == 33);
    }
    
    void testReadLine3(void)
    {
        CFileStream  loFileStream;
        char lacBuffer[1024] = {0};
        
        ASSERT_MESSAGE( loFileStream.Open("tmp.txt", O_CREAT | O_RDWR, 0755) == 0);
        
        ASSERT_MESSAGE( loFileStream.Write(
            "1234567890\r\n1234567890\n\n\n1234567890", 
            apl_strlen("1234567890\r\n1234567890\n1234567890") ) == (apl_ssize_t)apl_strlen("1234567890\r\n1234567890\n1234567890") );
        
        loFileStream.Sync();
        loFileStream.Seek(0);
        
        apl_int_t liCount = 0;
        while(loFileStream.ReadLine(lacBuffer, sizeof(lacBuffer) ) >= 0)
        {
            printf("%s\n", lacBuffer);
            liCount++;
        }

        ASSERT_MESSAGE(liCount == 5);
    }
    
    void testReadLine4(void)
    {
        CFileStream  loFileStream;
        char lacBuffer[1024] = {0};
        
        ASSERT_MESSAGE( loFileStream.Open("tmp.txt", O_CREAT | O_RDWR, 0755) == 0);
        
        ASSERT_MESSAGE( loFileStream.Write(
            "1234567890\r\n1234567890\n1234567890", 
            apl_strlen("1234567890\r\n1234567890\n1234567890") ) == (apl_ssize_t)apl_strlen("1234567890\r\n1234567890\n1234567890") );
        
        loFileStream.Sync();
        loFileStream.Seek(0);
        
        loFileStream.SetBufferSize(11);
        
        //case 1
        ASSERT_MESSAGE(loFileStream.Tell() == 0);
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 20) == 10);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "1234567890") == 0);
        
        //case 2
        ASSERT_MESSAGE(loFileStream.Tell() == 12);
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 6) == 5);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "12345") == 0);
        
        //case 2
        ASSERT_MESSAGE(loFileStream.Tell() == 17);
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 6) == 5);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "67890") == 0);
        
        //case 3
        ASSERT_MESSAGE(loFileStream.Tell() == 23);
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 20) == 10);
        ASSERT_MESSAGE(apl_strcmp(lacBuffer, "1234567890") == 0);
        
        //case 4
        ASSERT_MESSAGE(loFileStream.Tell() == 33);
        ASSERT_MESSAGE(loFileStream.ReadLine(lacBuffer, 10) == -1);
        ASSERT_MESSAGE(loFileStream.Tell() == 33);
    }

    void testReadLine5(void)
    {
        CFileStream  loFileStream;
        std::string  loLine;

        ASSERT_MESSAGE( loFileStream.Open("tmp.txt", O_CREAT | O_RDWR, 0755) == 0);
        
        ASSERT_MESSAGE( loFileStream.Write(
            "1234567890\r\n1234567890\n1234567890", 
            apl_strlen("1234567890\r\n1234567890\n1234567890") ) == (apl_ssize_t)apl_strlen("1234567890\r\n1234567890\n1234567890") );

        loFileStream.Sync();
        loFileStream.Seek(0);
        
        loFileStream.SetBufferSize(1024);
        
        //case 1
        ASSERT_MESSAGE(loFileStream.Tell() == 0);
        ASSERT_MESSAGE(loFileStream.ReadLine(loLine) == 10);
        ASSERT_MESSAGE(loLine == "1234567890");
        
        //case 2
        ASSERT_MESSAGE(loFileStream.Tell() == 12);
        ASSERT_MESSAGE(loFileStream.ReadLine(loLine) == 10);
        ASSERT_MESSAGE(loLine == "1234567890");
        
        //case 3
        ASSERT_MESSAGE(loFileStream.Tell() == 23);
        ASSERT_MESSAGE(loFileStream.ReadLine(loLine) == 10);
        ASSERT_MESSAGE(loLine == "1234567890");
        
        //case 4
        ASSERT_MESSAGE(loFileStream.Tell() == 33);
        ASSERT_MESSAGE(loFileStream.ReadLine(loLine) == -1);
        ASSERT_MESSAGE(loFileStream.Tell() == 33);
    }
    
    void testReadLine6(void)
    {
        CFileStream  loFileStream;
        std::string  loLine;
        
        ASSERT_MESSAGE( loFileStream.Open("tmp.txt", O_CREAT | O_RDWR, 0755) == 0);
        
        ASSERT_MESSAGE( loFileStream.Write(
            "1234567890\r\n1234567890\n\n\n1234567890", 
            apl_strlen("1234567890\r\n1234567890\n1234567890") ) == (apl_ssize_t)apl_strlen("1234567890\r\n1234567890\n1234567890") );
        
        loFileStream.Sync();
        loFileStream.Seek(0);
        
        apl_int_t liCount = 0;
        while(loFileStream.ReadLine(loLine) >= 0)
        {
            printf("%s\n", loLine.c_str() );
            liCount++;
        }
        
        ASSERT_MESSAGE(liCount == 5);
    }
    
    void testReadLine7(void)
    {
        CFileStream  loFileStream;
        std::string  loLine;
        
        ASSERT_MESSAGE( loFileStream.Open("tmp.txt", O_CREAT | O_RDWR, 0755) == 0);
        
        ASSERT_MESSAGE( loFileStream.Write(
            "1234567890\r\n1234567890\n1234567890", 
            apl_strlen("1234567890\r\n1234567890\n1234567890") ) == (apl_ssize_t)apl_strlen("1234567890\r\n1234567890\n1234567890") );
        
        loFileStream.Sync();
        loFileStream.Seek(0);
        
        loFileStream.SetBufferSize(11);
        
        //case 1
        ASSERT_MESSAGE(loFileStream.ReadLine(loLine) == 10);
        ASSERT_MESSAGE(loLine == "1234567890");
        
        //case 2
        ASSERT_MESSAGE(loFileStream.ReadLine(loLine) == 10);
        ASSERT_MESSAGE(loLine == "1234567890");
        
        //case 3
        ASSERT_MESSAGE(loFileStream.ReadLine(loLine) == 10);
        ASSERT_MESSAGE(loLine == "1234567890");
        
        //case 4
        ASSERT_MESSAGE(loFileStream.ReadLine(loLine) == -1);
    }

    void testIsEof(void)
    {
        CFileStream  loFileStream;
        char lacBuffer[1024] = {0};

        ASSERT_MESSAGE( loFileStream.Open("tmp.txt", O_CREAT | O_RDWR, 0755) == 0);
        
        ASSERT_MESSAGE( loFileStream.Write(
            "1234567890\r\n1234567890\n1234567890", 
            apl_strlen("1234567890\r\n1234567890\n1234567890") ) == (apl_ssize_t)apl_strlen("1234567890\r\n1234567890\n1234567890") );

        loFileStream.Sync();
        loFileStream.Seek(0);
        
        loFileStream.SetBufferSize(1024);

        //case 1
        ASSERT_MESSAGE(loFileStream.IsEof() == false);
        
        //case 2
        ASSERT_MESSAGE(loFileStream.Read(lacBuffer, 1024) == loFileStream.GetFileSize() );
        ASSERT_MESSAGE(loFileStream.IsEof() == true);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclFileStream);

int main()
{
    RUN_ALL_TEST(__FILE__);
}
