#include "Test.h"
#include "acl/File.h"

#define FILENAME "test.txt"

using namespace acl;

class CTestAclFile:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestAclFile);
    CPPUNIT_TEST(testOpen);
    CPPUNIT_TEST(testTruncate);
    CPPUNIT_TEST(testAllocate);
    CPPUNIT_TEST(testSeek);
    CPPUNIT_TEST(testRead);
    CPPUNIT_TEST(testReadOff);
    CPPUNIT_TEST(testWrite);
    CPPUNIT_TEST(testWriteOff);
    CPPUNIT_TEST(testWriteV);
    CPPUNIT_TEST(testReadV);
    CPPUNIT_TEST(testWriteVOffset);
    CPPUNIT_TEST(testReadVOffset);
    CPPUNIT_TEST(testIsOpened);
    CPPUNIT_TEST(testGetFileInfoQuo);
    CPPUNIT_TEST(testGetFileInfoPtr);
    CPPUNIT_TEST(testAccess);
    CPPUNIT_TEST(testRemove);
    CPPUNIT_TEST(testRename);
    CPPUNIT_TEST_SUITE_END();

private:
    CFile coFile;

public:
    void setUp(void) 
    {
        apl_int_t liRet = coFile.Open(FILENAME, APL_O_RDWR |APL_O_CREAT|APL_O_TRUNC, 0755);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open file failed.", (apl_int_t)0, liRet);
    }

    void tearDown(void) 
    {
        coFile.Close();
        apl_int_t liRet = CFile::Remove(FILENAME);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Remove file failed.", (apl_int_t)0, liRet);
    }

    void testOpen(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment 
        CFile loFile;

        //case
        apl_int_t liRet = loFile.Open("testopen.txt", APL_O_RDWR|APL_O_CREAT, 0666);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open file failed.", (apl_int_t)0, liRet);

        //end environment
        loFile.Close();
        CFile::Remove("testopen.txt");
    }

    void testTruncate(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment 

        //case
        apl_int_t liRet = coFile.Truncate(9);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Truncate failed.", (apl_int_t)0, liRet);
        apl_int_t liPos = coFile.Seek(0, APL_SEEK_END);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Truncate failed.", (apl_int_t)9, liPos);

        //end environment
    }

    void testAllocate(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment 
                                 
        //case                   
        apl_int_t liRet = coFile.Allocate(0, 22);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Allocate failed.", (apl_int_t)0, liRet);

        //end environment
    }

    void testSeek(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char const lsBuf[] = "1234567890abcdefghij";
        if(write(coFile.GetHandle(),lsBuf, strlen(lsBuf)) != (ssize_t)strlen(lsBuf))
        {
            CPPUNIT_FAIL("write failed.");
        }

        apl_int_t liRet = coFile.Sync();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sync failed.", (apl_int_t)0, liRet);

        //case
        apl_int_t liPos = coFile.Seek(15, APL_SEEK_SET);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Seek failed.", (apl_int_t)15, liPos);
        liPos = coFile.Seek(15, APL_SEEK_CUR);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Seek failed.", (apl_int_t)30, liPos);
        liPos = coFile.Seek(15, APL_SEEK_END);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Seek failed.", (apl_int_t)35, liPos);

        //end environment
    }

    void testSync(void)
    {
        //start case environment
        char const lsBuf[] = "1234567890abcdefghij";                                
        if(write(coFile.GetHandle(),lsBuf, strlen(lsBuf)) != (ssize_t)strlen(lsBuf))
        {                                                                           
            CPPUNIT_FAIL("write failed.");                                          
        }                                                                           
                                
        //case
        apl_int_t liRet = coFile.Sync();                                            
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sync failed.", (apl_int_t)0, liRet);          

        //end environment
    }

    void testRead(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start case environment
        apl_int_t liRet = -1;
        char const lsBuf[] = "1234567890abcdefghij";
        if(write(coFile.GetHandle(),lsBuf, strlen(lsBuf)) != (ssize_t)strlen(lsBuf))
        {
            CPPUNIT_FAIL("write failed.");
        }
        liRet = coFile.Sync();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sync failed.", (apl_int_t)0, liRet);

        char lsOut[21];
        memset(lsOut, 0, sizeof(lsOut));
        CFile loFile;
        liRet = loFile.Open(FILENAME, APL_O_RDONLY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open file failed.", (apl_int_t)0, liRet);

        //case
        apl_int_t liSize = loFile.Read(lsOut, sizeof(lsOut));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Read file failed.", (apl_int_t)20, liSize);
        CPPUNIT_ASSERT_MESSAGE("Read file failed.", apl_strcmp(lsBuf, lsOut) == 0);

        //end environment
        loFile.Close();
    }

    void testReadOff(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start case environment
        apl_int_t liRet = -1;
        char const lsBuf[] = "1234567890abcdefghij";
        if(write(coFile.GetHandle(),lsBuf, strlen(lsBuf)) != (ssize_t)strlen(lsBuf))
        {
            CPPUNIT_FAIL("write failed.");
        }
        liRet = coFile.Sync();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sync failed.", (apl_int_t)0, liRet);

        char lsOut[21];
        memset(lsOut, 0, sizeof(lsOut));
        CFile loFile;
        liRet = loFile.Open(FILENAME, APL_O_RDONLY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open file failed.", (apl_int_t)0, liRet);

        char const lsReadBuf[] = "0abcdefghij";
        memset(lsOut, 0, sizeof(lsOut));
        
        //case
        apl_int_t liSize = loFile.Read(9, lsOut, sizeof(lsOut));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Read offset file failed.", (apl_int_t)11, liSize);
        CPPUNIT_ASSERT_MESSAGE("Read offset file failed.", apl_strcmp(lsReadBuf, lsOut) == 0);
        
        //end environment
        loFile.Close();
    }

    void testWrite(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char const lsIn[] = "1234567890abcdefghij";

        //case
        apl_int_t liRet = coFile.Write(lsIn, 20);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write file failed.", (apl_int_t)20, liRet);

        char lsOut[21];
        memset(lsOut, 0, sizeof(lsOut));
        CFile loFile;
        liRet = loFile.Open(FILENAME, APL_O_RDONLY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open file failed.", (apl_int_t)0, liRet);
        apl_int_t liSize = loFile.Read(lsOut, sizeof(lsOut));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Read file failed.", (apl_int_t)20, liSize);
        CPPUNIT_ASSERT_MESSAGE("Read file failed.", apl_strcmp("1234567890abcdefghij", lsOut) == 0);

        //end environment
        loFile.Close();
    }

    void testWriteOff(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char const lsIn[] = "1234567890abcdefghij";
        apl_int_t liRet = coFile.Write(lsIn, 20);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write file failed.", (apl_int_t)20, liRet);

        //case
        char const lsAdd[] = "xyz";
        liRet = coFile.Write(10, lsAdd, 3);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Write file failed.", (apl_int_t)3, liRet);

        char lsOut[21];
        memset(lsOut, 0, sizeof(lsOut));
        CFile loFile;
        liRet = loFile.Open(FILENAME, APL_O_RDONLY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open file failed.", (apl_int_t)0, liRet);
        apl_int_t liSize = loFile.Read(lsOut, sizeof(lsOut));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Read file failed.", (apl_int_t)20, liSize);
        CPPUNIT_ASSERT_MESSAGE("Read file failed.", apl_strcmp("1234567890xyzdefghij", lsOut) == 0);

        //end environment
        loFile.Close();
    }


    void testWriteV(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char lsIov1[] = "0987654321";
        char lsIov2[] = "abcdefghij";
        struct apl_iovec_t loIov[2];
        loIov[0].iov_base = lsIov1;
        loIov[0].iov_len = apl_strlen(lsIov1);
        loIov[1].iov_base = lsIov2;
        loIov[1].iov_len = apl_strlen(lsIov2);

        //case
        apl_ssize_t liSize = coFile.WriteV(loIov, sizeof(loIov)/sizeof(loIov[0]));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("WriteV failed.", (apl_ssize_t)20, liSize);

        apl_handle_t liHandle = apl_open(FILENAME, APL_O_RDONLY, 0666);

        char lsIovRead1[11];
        char lsIovRead2[11];
        apl_memset(lsIovRead1, 0x0, sizeof(lsIovRead1));
        apl_memset(lsIovRead2, 0x0, sizeof(lsIovRead2));
        struct apl_iovec_t loIovRead[2];
        loIovRead[0].iov_base = lsIovRead1;
        loIovRead[0].iov_len = sizeof(lsIovRead1)-1;
        loIovRead[1].iov_base = lsIovRead2;
        loIovRead[1].iov_len = sizeof(lsIovRead2)-1;

        apl_ssize_t liRDSize = apl_readv(liHandle, loIovRead, sizeof(loIov)/sizeof(loIov[0]));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("ReadV failed.", (apl_ssize_t)20, liRDSize);
        CPPUNIT_ASSERT_MESSAGE("ReadV failed.", apl_strcmp(lsIovRead1, lsIov1) == 0);
        CPPUNIT_ASSERT_MESSAGE("ReadV failed.", apl_strcmp(lsIovRead2, lsIov2) == 0);

        //end environment
        apl_close(liHandle);
    }

    void testReadV(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liRet = -1;

        char lsIov1[] = "0987654321";
        char lsIov2[] = "abcdefghij";
        struct apl_iovec_t loIov[2];
        loIov[0].iov_base = lsIov1;
        loIov[0].iov_len = apl_strlen(lsIov1);
        loIov[1].iov_base = lsIov2;
        loIov[1].iov_len = apl_strlen(lsIov2);

        apl_ssize_t liSize = coFile.WriteV(loIov, sizeof(loIov)/sizeof(loIov[0]));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("WriteV failed.", (apl_ssize_t)20, liSize);

        CFile loFile;
        liRet = loFile.Open(FILENAME, APL_O_RDONLY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open file failed.", (apl_int_t)0, liRet);

        char lsIovRead1[11];
        char lsIovRead2[11];
        apl_memset(lsIovRead1, 0x0, sizeof(lsIovRead1));
        apl_memset(lsIovRead2, 0x0, sizeof(lsIovRead2));
        struct apl_iovec_t loIovRead[2];
        loIovRead[0].iov_base = lsIovRead1;
        loIovRead[0].iov_len = sizeof(lsIovRead1)-1;
        loIovRead[1].iov_base = lsIovRead2;
        loIovRead[1].iov_len = sizeof(lsIovRead2)-1;

        //case
        apl_ssize_t liRDSize = loFile.ReadV(loIovRead, sizeof(loIov)/sizeof(loIov[0]));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("ReadV failed.", (apl_ssize_t)20, liRDSize);
        CPPUNIT_ASSERT_MESSAGE("ReadV failed.", apl_strcmp(lsIovRead1, lsIov1) == 0);
        CPPUNIT_ASSERT_MESSAGE("ReadV failed.", apl_strcmp(lsIovRead2, lsIov2) == 0);

        //end environment
        loFile.Close();
    }

    void testWriteVOffset(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        char lsIov1[] = "0987654321";
        char lsIov2[] = "abcdefghij";
        struct apl_iovec_t loIov[2];
        loIov[0].iov_base = lsIov1;
        loIov[0].iov_len = apl_strlen(lsIov1);
        loIov[1].iov_base = lsIov2;
        loIov[1].iov_len = apl_strlen(lsIov2);

        //case
        apl_ssize_t liSize = coFile.WriteV(5, loIov, sizeof(loIov)/sizeof(loIov[0]));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("WriteV failed.", (apl_ssize_t)20, liSize);

        apl_handle_t liHandle = apl_open(FILENAME, APL_O_RDONLY, 0666);

        char lsIovRead1[11];
        char lsIovRead2[11];
        apl_memset(lsIovRead1, 0x0, sizeof(lsIovRead1));
        apl_memset(lsIovRead2, 0x0, sizeof(lsIovRead2));
        struct apl_iovec_t loIovRead[2];
        loIovRead[0].iov_base = lsIovRead1;
        loIovRead[0].iov_len = sizeof(lsIovRead1)-1;
        loIovRead[1].iov_base = lsIovRead2;
        loIovRead[1].iov_len = sizeof(lsIovRead2)-1;

        if(apl_lseek(liHandle, 5, APL_SEEK_SET) != 5)
        {
            CPPUNIT_FAIL("seek failed.");
        }

        apl_ssize_t liRDSize = apl_readv(liHandle, loIovRead, sizeof(loIov)/sizeof(loIov[0]));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("ReadV failed.", (apl_ssize_t)20, liRDSize);
        CPPUNIT_ASSERT_MESSAGE("ReadV failed.", apl_strcmp(lsIovRead1, lsIov1) == 0);
        CPPUNIT_ASSERT_MESSAGE("ReadV failed.", apl_strcmp(lsIovRead2, lsIov2) == 0);

        //end environment
        apl_close(liHandle);
    }

    void testReadVOffset(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_int_t liRet = -1;
        char lsIov1[] = "0987654321";
        char lsIov2[] = "abcdefghij";
        struct apl_iovec_t loIov[2];
        loIov[0].iov_base = lsIov1;
        loIov[0].iov_len = apl_strlen(lsIov1);
        loIov[1].iov_base = lsIov2;
        loIov[1].iov_len = apl_strlen(lsIov2);

        apl_ssize_t liSize = coFile.WriteV(5, loIov, sizeof(loIov)/sizeof(loIov[0]));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("WriteV failed.", (apl_ssize_t)20, liSize);

        CFile loFile;
        liRet = loFile.Open(FILENAME, APL_O_RDONLY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Open file failed.", (apl_int_t)0, liRet);

        char lsIovRead1[11];
        char lsIovRead2[11];
        apl_memset(lsIovRead1, 0x0, sizeof(lsIovRead1));
        apl_memset(lsIovRead2, 0x0, sizeof(lsIovRead2));
        struct apl_iovec_t loIovRead[2];
        loIovRead[0].iov_base = lsIovRead1;
        loIovRead[0].iov_len = sizeof(lsIovRead1)-1;
        loIovRead[1].iov_base = lsIovRead2;
        loIovRead[1].iov_len = sizeof(lsIovRead2)-1;

        //case 
        apl_ssize_t liRDSize = loFile.ReadV(5, loIovRead, sizeof(loIov)/sizeof(loIov[0]));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("ReadV failed.", (apl_ssize_t)20, liRDSize);
        CPPUNIT_ASSERT_MESSAGE("ReadV failed.", apl_strcmp(lsIovRead1, lsIov1) == 0);
        CPPUNIT_ASSERT_MESSAGE("ReadV failed.", apl_strcmp(lsIovRead2, lsIov2) == 0);

        //end environment
        loFile.Close();

    }

    void testIsOpened(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
            
        //case
        bool lbFlag = coFile.IsOpened();
        if(!lbFlag)
        {
            CPPUNIT_FAIL("IsOpened failed.");
        }

        //end environment
    }

    void testGetFileInfoQuo(void)
    {
        PRINT_TITLE_2(__func__);
        
        //start case environment
        CFileInfo loFileInfo;
        coFile.GetFileInfo(loFileInfo);

        struct stat loStat;
        if ( fstat(coFile.GetHandle(), &loStat) != 0 )
        {
            return;
        }

        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get size failed.", 
                loFileInfo.GetSize(), (apl_size_t)loStat.st_size);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get device number failed.", 
                loFileInfo.GetDev(), (apl_int_t)loStat.st_dev);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get i-node number failed.", 
                loFileInfo.GetIno(), (apl_int_t)loStat.st_ino);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get file mode failed.", 
                loFileInfo.GetMode(), (apl_int_t)loStat.st_mode);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get number of links failed.", 
                loFileInfo.GetNLink(), (apl_int_t)loStat.st_nlink);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get uid failed.", 
                loFileInfo.GetUid(), (apl_int_t)loStat.st_uid);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get gid failed.", 
                loFileInfo.GetGid(), (apl_int_t)loStat.st_gid);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get time of last access failed.", 
                loFileInfo.GetATime(), (apl_time_t)loStat.st_atime);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get time of last modification failed.", 
                loFileInfo.GetMTime(), (apl_time_t)loStat.st_mtime);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get time of last file status change failed.", 
                loFileInfo.GetCTime(), (apl_time_t)loStat.st_ctime);

        //end environment
    }

    void testGetFileInfoPtr(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        struct stat loStat;
        if ( fstat(coFile.GetHandle(), &loStat) != 0 )
        {
            return;
        }

        CFileInfo *lpoFileInfo = new CFileInfo;
        coFile.GetFileInfo(lpoFileInfo);

        //case
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get size failed.", 
                lpoFileInfo->GetSize(), (apl_size_t)loStat.st_size);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get device number failed.", 
                lpoFileInfo->GetDev(), (apl_int_t)loStat.st_dev);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get i-node number failed.", 
                lpoFileInfo->GetIno(), (apl_int_t)loStat.st_ino);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get file mode failed.", 
                lpoFileInfo->GetMode(), (apl_int_t)loStat.st_mode);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get number of links failed.", 
                lpoFileInfo->GetNLink(), (apl_int_t)loStat.st_nlink);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get uid failed.", 
                lpoFileInfo->GetUid(), (apl_int_t)loStat.st_uid);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get gid failed.", 
                lpoFileInfo->GetGid(), (apl_int_t)loStat.st_gid);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get time of last access failed.", 
                lpoFileInfo->GetATime(), (apl_time_t)loStat.st_atime);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get time of last modification failed.", 
                lpoFileInfo->GetMTime(), (apl_time_t)loStat.st_mtime);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Get time of last file status change failed.", 
                lpoFileInfo->GetCTime(), (apl_time_t)loStat.st_ctime);

        //end environment
        delete lpoFileInfo;
    }

    void testAccess(void)
    {
        PRINT_TITLE_2(__func__);
        //start case environment

        //case
        apl_int_t liRet = CFile::Access(FILENAME, APL_F_OK | APL_R_OK | APL_W_OK | APL_X_OK);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Access failed.", (apl_int_t)0, liRet);

        //end environment
    }

    void testRemove(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment

        apl_handle_t liHandle = apl_open("testRemove.txt", APL_O_RDWR|APL_O_CREAT, 0666);
        if(liHandle < 0)
        {
            CPPUNIT_FAIL("apl_open failed.");
        }

        //case
        apl_int_t liRet = CFile::Remove("testRemove.txt");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Remove failed.", (apl_int_t)0, liRet);

        liHandle = apl_open("testRemove.txt", APL_O_RDONLY, 0666);
        if(liHandle > 0)
        {
            CPPUNIT_FAIL("Remove failed.");
        }

        //end environment
        apl_close(liHandle);
    }

    void testRename(void)
    {
        PRINT_TITLE_2(__func__);

        //start case environment
        apl_handle_t liHandle = apl_open("OldName.txt", APL_O_RDWR|APL_O_CREAT, 0666);
        if(liHandle < 0)                     
        {                                    
            CPPUNIT_FAIL("apl_open failed.");
        }                                    

        //case
        CFile::Rename("OldName.txt", "NewName.txt");
        liHandle = apl_open("OldName.txt", APL_O_RDONLY, 0666);
        if(liHandle > 0)                   
        {                                  
            CPPUNIT_FAIL("Rename failed.");
        }                                  

        liHandle = apl_open("NewName.txt", APL_O_RDONLY, 0666);
        if(liHandle < 0)                                       
        {                                                      
            CPPUNIT_FAIL("Rename failed.");                    
        }                                                      

        //end environment
        apl_close(liHandle);
        CFile::Remove("NewName.txt");
    }
};  

CPPUNIT_TEST_SUITE_REGISTRATION(CTestAclFile);

int main()
{
    RUN_ALL_TEST(__FILE__);
}

