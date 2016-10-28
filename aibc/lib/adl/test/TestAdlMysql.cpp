#include "Test.h"
#include "adl/Adl.h"
#include "acl/DateTime.h"
#include "acl/TimeValue.h"
#include <vector>

using namespace adl;

#define THREADS_FOR_GETTING_CONN 12
//Aibc's mysql interface doesn't like oracle which support sharing the connection.
//That's say, only one cursor can get from one connection in mysql
#define COURSOR_COUNTS_PER_CONN 1
#define EXECUTE_COUNTS_PER_CURSOR 500
#define SELECT_INTERVAL 499

#define BINDING_TEST_COUNTS 1000

// Performance test counts
#define PRFM_INSERT_COUNT 10000
#define PRFM_BINDINSERT_COUNT 10000
#define PRFM_SELECT_COUNT 10000
#define PRFM_DELETE_COUNT 10000
/*
 * database: test, TABLE: ABC
 * --------------------------
 * Filed    Type
 * -----    ----
 * f1       int(11) 
 * f2       varchar(100) 
 * f3       text
 * f4       double
 */

#define DB_NAME "ismg55_dev"
//#define DB_NAME "test"
#define DROP_TABLE "DROP TABLE ABC"
//InnoDB support transsion
#define CREATE_TABLE "CREATE TABLE `ABC` (`f1` int(11) NOT NULL auto_increment, `f2` varchar(100) default NULL, `f3` text, `f4` double NOT NULL DEFAULT '2009.1023', `f5` int(11) NULL DEFAULT '1', PRIMARY KEY  (`f1`)) ENGINE=InnoDB"

IDatabase* gpoMysql = APL_NULL;

class TimeCalcGuard
{
	char csMessage[128];
	apl_int_t ciDataCount;
	acl::CTimestamp coTimeBegin;
	acl::CTimestamp coTimeEnd;

	public:
	TimeCalcGuard( const char* apcMessage = APL_NULL, const int aoDataCount = 1 )
	{
		SetBegin(apcMessage, aoDataCount);
	}

	// Set new DataCount and return old DataCount
	int SetDataCount( const int aoDataCount )
	{
		apl_int_t liRet;

		liRet = ciDataCount;
		ciDataCount = aoDataCount;

		return liRet;
	}

	void SetBegin( const char* apcMessage = APL_NULL, const int aoDataCount = 1 )
	{
		ciDataCount = aoDataCount;
		coTimeBegin.Update( acl::CTimestamp::PRC_USEC );

		if( apcMessage != APL_NULL )
			apl_strncpy( csMessage, apcMessage, 128 );
	}

	void PrintCurrentResult()
	{
		double lfTimeDiff;

		coTimeEnd.Update(acl::CTimestamp::PRC_USEC);

		lfTimeDiff = coTimeEnd.Usec() - coTimeBegin.Usec();
		lfTimeDiff /= 1000000;;
		
		apl_errprintf( " %s - %lfs taken, %d data effected, average %lf data per sec. \n" , 
			csMessage, lfTimeDiff, ciDataCount, ciDataCount / lfTimeDiff  );
	}
};

struct CThread
{
    volatile bool   mbStarted;
    apl_thread_t    miThrId;
    apl_mutex_t     msMtx;
    apl_cond_t      msCond;

    CThread()
        :mbStarted(false)
    {
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_init, (&msMtx));
        RUN_AND_CHECK_RET_ERRNO(apl_cond_init, (&msCond));
    }

    virtual ~CThread()
    {
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_destroy, (&msMtx));
        RUN_AND_CHECK_RET_ERRNO(apl_cond_destroy, (&msCond));
    }

    void Start()
    {
        mbStarted = true;
        RUN_AND_CHECK_RET_ERRNO(apl_thread_create, (&miThrId, &CThread::_ThreadEntry, this, 0, 0));
    }

    void Join()
    {
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_lock, (&msMtx, APL_INT64_C(-1)));
        //wait for signal with timeout in case of the signal if early than wait
        while (mbStarted)
        {
            RUN_AND_CHECK_RET_ERRNO(apl_cond_wait, (&msCond, &msMtx, APL_INT64_C(-1)));
        }
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&msMtx));
    }

    virtual void Run() = 0;

    void OnExit()
    {
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_lock, (&msMtx, APL_INT64_C(-1)));
        mbStarted = false;
        RUN_AND_CHECK_RET_ERRNO(apl_cond_signal, (&msCond));
        RUN_AND_CHECK_RET_ERRNO(apl_mutex_unlock, (&msMtx));
    }

    static void* _ThreadEntry(void* apv_arg)
    {
        CThread* lpoThr = (CThread*)apv_arg;
        lpoThr->Run();
        lpoThr->OnExit();
        return APL_NULL;
    }
};

class CCursorThread : public CThread
{
public:
    CCursorThread(ICursor* apcCursor, apl_size_t auId)
        :mpcCursor(apcCursor)
        ,muId(auId)
    {
    }

    virtual ~CCursorThread() 
    {
        this->mpcCursor->Close();
    }

    virtual void Run()
    {
        apl_size_t luIdBase = muId * EXECUTE_COUNTS_PER_CURSOR + 1;
        apl_size_t luIdx;
        char lacBuf[128];

        apl_errprintf("cursor %d running\n", muId);
        for (luIdx = 0; luIdx < EXECUTE_COUNTS_PER_CURSOR; ++luIdx)
        {
            //apl_errprintf("cursor %d running\n", luIdBase);
            apl_memset(lacBuf, 0, sizeof(lacBuf));
            apl_snprintf(lacBuf, sizeof(lacBuf), 
                    "INSERT INTO ABC (F1, F2, F3, F4) VALUES (%d, 'test', 'test', 1.1)", luIdBase);
            this->CursorExecute(lacBuf);
#if 0
            //execute select every SELECT_INTERVAL
            if (luIdx % SELECT_INTERVAL == 0) 
            {
                apl_memset(lacBuf, 0, sizeof(lacBuf));
                apl_snprintf(lacBuf, sizeof(lacBuf), "SELECT * FROM ABC");
                this->CursorExecute(lacBuf);
            }
#endif
            ++luIdBase;
        }
        apl_errprintf("cursor %d exiting\n", muId);
    }

    apl_int_t CursorExecute(const char* apcSql)
    {
        apl_int_t liRet;
        liRet = this->mpcCursor->Execute(apcSql);

        if (liRet != 0) 
        {
            apl_errprintf("Execute: | %s | failed, err msg:\n%s\n!!\n", 
                    apcSql, this->mpcCursor->GetErrorMsg());
            CPPUNIT_ASSERT_MESSAGE("Failed!!",  0);
        }
        return liRet;
    }

private:
    ICursor* mpcCursor;
    apl_size_t muId;
};

class CMultiConn : public CThread
{
public:
    CMultiConn(IConnection* apoConn, apl_size_t auIndex, apl_size_t auCursorCount=COURSOR_COUNTS_PER_CONN)
        :mpoConn(apoConn)
        ,muIndex(auIndex) //which connection we are
        ,muCursorCount(auCursorCount)//how many cursors each conn
    {
        CPPUNIT_ASSERT_MESSAGE("CMultiConn failed", 
                apoConn != APL_NULL && muCursorCount != 0);
    }

    virtual ~CMultiConn() 
    {
    }

    virtual void Run()
    {
        apl_size_t luIdx = 0;
        //id dispatch to cursor
        apl_size_t luId = muIndex * muCursorCount;
        CCursorThread* lpoCursorThrd = APL_NULL;

        //apl_errprintf("conn %d running\n", muIndex);
        this->mpoConn->BeginTrans();
        //new muCursorCount threads and execute 
        for (luIdx=0; luIdx<muCursorCount; ++luIdx)
        {
            ICursor* lpoCursor = mpoConn->Cursor();
            CPPUNIT_ASSERT_MESSAGE("get cursor failed", lpoCursor != APL_NULL);

            lpoCursorThrd = new CCursorThread(lpoCursor, luId);
            
            CPPUNIT_ASSERT_MESSAGE("new CCursorThread failed", lpoCursorThrd != APL_NULL);
            this->moVetCursor.push_back(lpoCursorThrd);

            lpoCursorThrd->Start();
            ++luId;
        }

        //wait for the cursor done AND delete
        for(luIdx=0; luIdx<moVetCursor.size(); ++luIdx)
        {
            (moVetCursor[luIdx])->Join();
            delete moVetCursor[luIdx];
        }

        this->mpoConn->Commit();

        //apl_errprintf("releasing connection ...\n");
        gpoMysql->ReleaseConnection(this->mpoConn);
        //apl_errprintf("conn %d exiting\n", muIndex);
    }

private:
    IConnection* mpoConn;

    apl_size_t muIndex;
    apl_size_t muCursorCount;

    std::vector<CCursorThread*> moVetCursor;
};

class CTestMysql: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestMysql);
    CPPUNIT_TEST(TestMysqlDropTable);
    CPPUNIT_TEST(TestMysqlCreateTable);

	CPPUNIT_TEST(TestMysqlInsert);
    CPPUNIT_TEST(TestMysqlSelect);
    CPPUNIT_TEST(TestMysqlUpdate);

    //CPPUNIT_TEST(TestMysqlComplicate);
    //re-new the table
#if 0
    CPPUNIT_TEST(TestMysqlDropTable);
    CPPUNIT_TEST(TestMysqlCreateTable);
    CPPUNIT_TEST(TestMultiCursors);
    CPPUNIT_TEST(TestMysqlSelect);
#endif

    CPPUNIT_TEST(TestMysqlDropTable);
    CPPUNIT_TEST(TestMysqlCreateTable);
    CPPUNIT_TEST(TestParaBindingInsert);
    CPPUNIT_TEST(TestParaBindingInsertStrToInt);
    CPPUNIT_TEST(TestParaBindingSelect);
    CPPUNIT_TEST(TestParaBindingDuplicatedParam);
	
    //CPPUNIT_TEST(TestMySqlReconnect);

	// Performance Tests
    CPPUNIT_TEST(TestMysqlInsertPerformance);
    CPPUNIT_TEST(TestMysqlBindInsertPerformance);
    CPPUNIT_TEST(TestMysqlInsertPerformanceAllInOne);
	CPPUNIT_TEST(TestMysqlSelectPerformance);
	CPPUNIT_TEST(TestMysqlDeletePerformance);

    CPPUNIT_TEST_SUITE_END();
public:
    IConnection* mpoConn;
    ICursor* mpoCursor;

    void setUp(void) 
    {
        //gpoMysql = new CDataBase( "ismg55", "ismg55", "127.0.0.1", 5, DB_NAME, 3306);
        //gpoMysql = new CDataBase( "wangxg", "wangxg", "10.3.3.111", 5, "ismg55wxg", 3306);
        //gpoMysql = new CDataBase( "ismg55", "ismg55", "10.3.3.108", 5, "ismg55_dev", 8807);
        //gpoMysql = new CMySqlDataBase( "hezk", "hezk", "10.3.19.45", 5, DB_NAME, 3306);
	    gpoMysql = CreateDatabase(
            "lib=../src/mysql/.libs/libadl_mysql2.so,name=ismg55_dev,user=ismg55,pass=ismg55,host=10.3.3.108,port=8807,connpool=5");
        assert(gpoMysql != APL_NULL);

        if (gpoMysql -> Init() != 0)
        {
            apl_errprintf("Init failed: %s\n", gpoMysql->GetErrorMsg());
            assert(0);
        }

        //get conn
        mpoConn = gpoMysql->GetConnection();
        assert(mpoConn != APL_NULL);

        //get cursor
        mpoCursor = mpoConn->Cursor();
        assert(mpoCursor != APL_NULL);
    }

    void tearDown(void) 
    {
        mpoCursor->Close();
        gpoMysql->ReleaseConnection(mpoConn);
        delete gpoMysql;
        gpoMysql = APL_NULL;
    }

    void TestMysqlExecute(const char* apcSql, const char* apcErrMsg = APL_NULL, bool abAssert=true)
    {
        apl_int_t liResult;

        if (apcSql != APL_NULL)
            liResult = mpoCursor->Execute(apcSql);
        else 
            liResult = mpoCursor->Execute();

        if (liResult != 0)
        {
            if (apcErrMsg == APL_NULL)
                apl_errprintf("Execute failed, error msg:\n%s\nerror code: %d\n", 
                        mpoCursor->GetErrorMsg(), mpoCursor->GetErrorCode());
            else 
                apl_errprintf("Execute failed(%s), error msg:\n%s\nerror code: %d\n", 
                        apcErrMsg, mpoCursor->GetErrorMsg(), mpoCursor->GetErrorCode());

            if (abAssert)
                CPPUNIT_ASSERT(0);
        }
    }

    void TestMysqlDropTable(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        TestMysqlExecute(DROP_TABLE, "drop table failed", false);
    }

    void TestMysqlCreateTable(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        TestMysqlExecute(CREATE_TABLE);
    }

    void TestMysqlInsert(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_int_t liResult = 0;
        ICursor* lpoCursorSecond = APL_NULL;

        //case
        //1
        TestMysqlExecute("insert into ABC (f1, f2, f3, f4) VALUES (111, 'notrans', 'atonce', 1.1)");

        //2, transsion and commit
        CPPUNIT_ASSERT_MESSAGE("Beging transsion failed", 0 == mpoConn->BeginTrans());
        TestMysqlExecute(
                "insert into ABC (f1, f2, f3, f4) VALUES (222, 'trans', 'commit', 123456789012345.123456)", 
                "Transsion insert failed");
        TestMysqlExecute(
                "insert into ABC (f1, f2, f3, f4) VALUES (333, 'trans', 'commit', 1.12345678901234537)", 
                "Transsion insert failed");
        TestMysqlExecute(
                "insert into ABC (f1, f2, f3, f4) VALUES (444, 'trans', 'commit', 1.1)",
                "Transsion insert failed");

        CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Commit());
        
        //3, transsion and rollback
        CPPUNIT_ASSERT_MESSAGE("Beging transsion failed", 0 == mpoConn->BeginTrans());
        TestMysqlExecute(
                "insert into ABC (f1, f2, f3,f4) VALUES (555, 'trans', 'rollback', 1.1)", 
                "Transsion insert failed");
        TestMysqlExecute(
                "insert into ABC (f1, f2, f3, f4) VALUES (666, 'trans', 'rollback', 1.1)", 
                "Transsion insert failed");
        TestMysqlExecute(
                "insert into ABC (f1, f2, f3, f4) VALUES (777, 'trans', 'rollback', 1.1)", 
                "Transsion insert failed");
        CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Rollback());

        //4, get another cursor and try dupplicate insert which must failed!
        lpoCursorSecond = mpoConn->Cursor();
        liResult = lpoCursorSecond->Execute("insert into ABC (f1, f2, f3, f4) VALUES (444, 'notrans', 'atonce', 1.1)");
        CPPUNIT_ASSERT_MESSAGE("Duplicated execute insert failed", liResult != 0);
        apl_errprintf("Duplicated insert result(errmsg): (%d)%s\n", liResult, lpoCursorSecond->GetErrorMsg());
        apl_errprintf("Duplicated insert result(errno): (%d)%d\n", liResult, lpoCursorSecond->GetErrorCode());
        
        //end environment
        lpoCursorSecond->Close();
    }

    void PrintResultSet(ICursor* apoCursor)
    {
        apl_int_t liLen = -1;
        apl_int_t liIter;
        apl_int_t liFieldCount = 0;
        apl_int_t liRowCount = 0;
        const char* lpcField = APL_NULL;

        //how many fields we got
        liFieldCount = apoCursor->GetFieldNameCount();
        CPPUNIT_ASSERT_MESSAGE("Select failed, field name count error\n", liFieldCount != 0);
        apl_errprintf("%d fields fetched\n", liFieldCount);

        for (liIter = 0; liIter < liFieldCount; ++liIter)
        {
            lpcField = apoCursor->GetFieldName(liIter);
            CPPUNIT_ASSERT_MESSAGE("GetFieldName failed", lpcField != APL_NULL);
            apl_errprintf("Field:%s\t", lpcField);
        }
        apl_errprintf("\n\n");

        //how many rows we got
        liRowCount = apoCursor->GetRowCounts();
        CPPUNIT_ASSERT_MESSAGE("Select failed, row count error\n", liRowCount != 0);
        apl_errprintf("%d rows fetched\n", liRowCount);

        apl_int_t liIterField;
        char* lpcValue = APL_NULL;

        //there are lots of ways to get the result set
        apoCursor->SetPreFetchRows(100);
        while (apoCursor->FetchNext() == 0)
        {
            for (liIterField = 0; liIterField < liFieldCount; ++liIterField)
            {
                lpcValue = (char*)apoCursor->GetField(liIterField, &liLen);
                CPPUNIT_ASSERT_MESSAGE("GetField failed",  lpcValue != APL_NULL);
                apl_errprintf("|%s (Len:%d)|\t", lpcValue, liLen);
            }
            apl_errprintf("\n");
        }

    }

    void TestMysqlSelect(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        const char * SQLSTR1 = "select * from ABC order by f1";
        //const char * SQLSTR1 = "select count(*) from ABC";

        TestMysqlExecute(SQLSTR1, "select failed");

        PrintResultSet(this->mpoCursor);
        //end environment
    }


    void TestMysqlUpdate(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_int_t liRows = 0;
        const char* lpcUpdate = "  update ABC set f2='update', f3='oracle' where f1=333";

        TestMysqlExecute(lpcUpdate, "Update failed");
        //three rows should be updated (which insert in TestMysqlInsert()
        liRows = mpoCursor->GetAffectedRows();
        CPPUNIT_ASSERT_MESSAGE("GetField failed",  1 == liRows);
    }

    void TestMysqlComplicate(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_int_t liRows = 0;
        const char* lpcUpdate = "update abc set (f2, f3, f4)=(select f2, f3, f4 from abc where f1=444) where f1=111 or f1=222";

        TestMysqlExecute(lpcUpdate, "Update failed");
        //three rows should be updated (which insert in TestMysqlInsert()
        liRows = mpoCursor->GetAffectedRows();
        CPPUNIT_ASSERT_MESSAGE("GetField failed",  2 == liRows);
        apl_errprintf("the affected rows is %d\n", liRows);
    }

    void TestMultiCursors(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_size_t luIndex = 0;
        CMultiConn* lpcMultiConn = APL_NULL;
        IConnection* lpcConn = APL_NULL;
        std::vector<CMultiConn*> loVetConn;

        for (luIndex=0; luIndex<THREADS_FOR_GETTING_CONN; ++luIndex) 
        {
            //apl_errprintf("getting conn %d\n", luIndex);
            lpcConn = gpoMysql->GetConnection();
            CPPUNIT_ASSERT_MESSAGE("get conn failed", lpcConn != APL_NULL);
            lpcMultiConn = new CMultiConn(lpcConn, luIndex);
            CPPUNIT_ASSERT_MESSAGE("new CMultiConn failed", lpcMultiConn != APL_NULL);
            loVetConn.push_back(lpcMultiConn);

            lpcMultiConn->Start();
        }

        //wait for each conn and delete them
        for (luIndex=0; luIndex<loVetConn.size(); ++luIndex) 
        {
            (loVetConn[luIndex])->Join();
            //apl_errprintf("deleting conn %d\n", luIndex);
            delete (loVetConn[luIndex]);
        }
    }

    void TestParaBindingInsert(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

		const char* lpcSQL = 
        	"INSERT INTO ABC (f1, f2, f3, f4) values (:f1, :f2, :f3, :f4)";

        apl_int_t liIdx;
        apl_int_t liRet;

        apl_int32_t liField1;
        char lpcField2[100];
        char lpcField3[100];
        double ldField4;

		liRet = mpoCursor->Prepare( lpcSQL );
        CPPUNIT_ASSERT_MESSAGE("Parameter binding prepare failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f1", &liField1);
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f2", lpcField2, sizeof(lpcField2) );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f3", lpcField3, sizeof(lpcField3) );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f4", &ldField4 );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

        CPPUNIT_ASSERT_MESSAGE("Beging transsion failed", 0 == mpoConn->BeginTrans());

        for (liIdx=0; liIdx<BINDING_TEST_COUNTS; ++liIdx)
        {
            liField1 = 10000 + liIdx;
            apl_memset(lpcField2, 0, sizeof(lpcField2));
            apl_memset(lpcField3, 0, sizeof(lpcField3));
            apl_snprintf(lpcField2, sizeof(lpcField2), "param binding %d", liIdx);
            apl_snprintf(lpcField3, sizeof(lpcField3), "param binding %d", liIdx+1);
            ldField4 = 1.123456 + liIdx;

            TestMysqlExecute(APL_NULL, "Parameter binding execute failed");
        }

        CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Commit());
    }

    void TestParaBindingInsertStrToInt(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

		const char* lpcSQL = 
        	"INSERT INTO ABC (f2, f3, f4, f5) values (:f2, :f3, :f4, :f5)";

        apl_int_t liIdx;
        apl_int_t liRet;

        char lpcField2[100];
        char lpcField3[100];
        double ldField4;
        char lpcField5[100];

		liRet = mpoCursor->Prepare( lpcSQL );
        CPPUNIT_ASSERT_MESSAGE("Parameter binding prepare failed", liRet == 0);
		liRet = mpoCursor->BindParam( "f5", lpcField5, sizeof(lpcField5));
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f2", lpcField2, sizeof(lpcField2) );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f3", lpcField3, sizeof(lpcField3) );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f4", &ldField4 );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

        CPPUNIT_ASSERT_MESSAGE("Beging transsion failed", 0 == mpoConn->BeginTrans());

        for (liIdx=0; liIdx<BINDING_TEST_COUNTS; ++liIdx)
        {
            apl_memset(lpcField2, 0, sizeof(lpcField2));
            apl_memset(lpcField3, 0, sizeof(lpcField3));
            apl_memset(lpcField5, 0, sizeof(lpcField5));
            apl_snprintf(lpcField2, sizeof(lpcField2), "param binding %d", liIdx);
            apl_snprintf(lpcField3, sizeof(lpcField3), "param binding %d", liIdx+1);
            apl_snprintf(lpcField5, sizeof(lpcField5), "%d", liIdx);
            ldField4 = 1.123456 + liIdx;

            TestMysqlExecute(APL_NULL, "Parameter binding execute failed");
        }

        CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Commit());
    }

    void TestParaBindingSelect(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        const char* lpcSQL = 
            "SELECT * FROM ABC where f2 = :Content";
        apl_int_t liIdx;
        apl_int_t liRet;

        char lpcContent[100];

        liRet = mpoCursor->Prepare( lpcSQL );
        CPPUNIT_ASSERT_MESSAGE("Parameter binding prepare failed", liRet == 0);

		liRet = mpoCursor->BindParam( "Content", lpcContent, sizeof(lpcContent) );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

        for (liIdx=0; liIdx<BINDING_TEST_COUNTS; ++liIdx)
        {
            apl_memset(lpcContent, 0, sizeof(lpcContent));
            apl_snprintf(lpcContent, sizeof(lpcContent), "param binding %d", liIdx);
            TestMysqlExecute(APL_NULL, "Parameter binding select execute failed");

            PrintResultSet(this->mpoCursor);
        }
    }

    void TestParaBindingDuplicatedParam(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        const char* lpcSQL = 
            "SELECT * FROM ABC where f1 > :FieldId and f2 = :Content or f3 = :Content";
        apl_int_t liIdx;
        apl_int_t liRet;

        apl_int32_t liField1;
        char lpcContent[100];

        //the parameter count is still "2" here! Duplicate parameters would 
        //treat as 1 parameters. The duplicated parameters don't need describe 
        //with '< >' any more. 
        liRet = mpoCursor->Prepare( lpcSQL );

        CPPUNIT_ASSERT_MESSAGE("Parameter binding prepare failed", liRet == 0);

		liRet = mpoCursor->BindParam( "FieldId", &liField1 );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		liRet = mpoCursor->BindParam( "Content", lpcContent, sizeof(lpcContent) );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

        for (liIdx=0; liIdx<BINDING_TEST_COUNTS; ++liIdx)
        {
            liField1 = 10000 + liIdx;
            apl_memset(lpcContent, 0, sizeof(lpcContent));
            apl_snprintf(lpcContent, sizeof(lpcContent), "param binding %d", liIdx+1);
            TestMysqlExecute(APL_NULL, "Parameter binding select execute failed");

            PrintResultSet(this->mpoCursor);
        }
    }

	void TestMysqlInsertPerformance()
	{
		PRINT_TITLE_2( __func__ );

		char lsSQL[128];
		apl_int_t liIndex;
		
		apl_snprintf( lsSQL, sizeof(lsSQL), "TRUNCATE TABLE ABC " );
		TestMysqlExecute( lsSQL );
		
		TimeCalcGuard loGuard( __func__, PRFM_INSERT_COUNT );

		mpoConn->BeginTrans();
		for( liIndex = 0; liIndex < PRFM_INSERT_COUNT; ++liIndex )
		{
			apl_snprintf( lsSQL, sizeof( lsSQL ), 
				"INSERT INTO ABC( f1, f2, f3, f4 ) VALUES( %d, '%dfree', '%dfish', %d1.0 )", 
				liIndex + 1, liIndex, liIndex, liIndex );
			TestMysqlExecute( lsSQL );
		}
		mpoConn->Commit();

		loGuard.PrintCurrentResult();
	}


	void TestMysqlBindInsertPerformance()
	{
		PRINT_TITLE_2(__func__);
		const char* lpcSQL = 
        	"INSERT INTO ABC (f1, f2, f3, f4) values (:f1, :f2, :f3, :f4)";

        apl_int_t liIdx;
        apl_int_t liRet;

        apl_int32_t liField1;
        char lpcField2[100];
        char lpcField3[100];
        double ldField4;
		char lsSQL[128];

		apl_snprintf( lsSQL, sizeof(lsSQL), "TRUNCATE TABLE ABC " );
		TestMysqlExecute( lsSQL );

		liRet = mpoCursor->Prepare( lpcSQL );
        CPPUNIT_ASSERT_MESSAGE("Parameter binding prepare failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f1", &liField1);
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f2", lpcField2, sizeof(lpcField2) );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f3", lpcField3, sizeof(lpcField3) );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f4", &ldField4 );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		CPPUNIT_ASSERT_MESSAGE("Beging transsion failed", 0 == mpoConn->BeginTrans());
				
		TimeCalcGuard loGuard( __func__, PRFM_INSERT_COUNT );

		for (liIdx=0; liIdx<PRFM_BINDINSERT_COUNT; ++liIdx)
		{
			liField1 = 10000 + liIdx;
			apl_memset(lpcField2, 0, sizeof(lpcField2));
			apl_memset(lpcField3, 0, sizeof(lpcField3));
			apl_snprintf(lpcField2, sizeof(lpcField2), "param binding %d", liIdx);
			apl_snprintf(lpcField3, sizeof(lpcField3), "param binding %d", liIdx+1);
			ldField4 = 1.123456 + liIdx;
		
			TestMysqlExecute(APL_NULL, "Parameter binding execute failed");
		}

		CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Commit());
		loGuard.PrintCurrentResult();
	}

	void TestMysqlDeletePerformance()
	{
		PRINT_TITLE_2( __func__ );

		char lsSQL[128];
		apl_int_t liIndex;
		
		TimeCalcGuard loGuard( __func__, PRFM_DELETE_COUNT);

		mpoConn->BeginTrans();
		for( liIndex = 0; liIndex < PRFM_DELETE_COUNT; ++liIndex )
		{
			apl_snprintf( lsSQL, sizeof( lsSQL ), 
				"DELETE FROM ABC WHERE f1=%d", 
				10000 + liIndex);
			TestMysqlExecute( lsSQL );
		}
		mpoConn->Commit();

		loGuard.PrintCurrentResult();
	}

	void TestMysqlSelectPerformance()
	{
		PRINT_TITLE_2( __func__ );

		char lsSQL[128];
		
		TimeCalcGuard loGuard( __func__, PRFM_SELECT_COUNT);

		mpoConn->BeginTrans();

		apl_snprintf( lsSQL, sizeof( lsSQL ), "Select * FROM ABC" );
		TestMysqlExecute( lsSQL );

        mpoCursor->SetPreFetchRows(1);

        while (mpoCursor->FetchNext() == 0)
        {
		}

		mpoConn->Commit();

		loGuard.PrintCurrentResult();
	}

	void TestMySqlReconnect()
	{
		PRINT_TITLE_2( __func__ );
		
		char lsSQL[128];
		ICursor* lpoCursor;
		IConnection* lpoConn;

		lpoConn = gpoMysql -> GetConnection();
		CPPUNIT_ASSERT_MESSAGE("Can't get connection\n", mpoConn != APL_NULL);
		apl_errprintf("GetConnection OK\n");

		apl_errprintf( "Disconnect now, then press any key " );
		getchar();		

		lpoCursor = lpoConn -> Cursor();
		CPPUNIT_ASSERT_MESSAGE("Can't get cursor\n", mpoCursor != APL_NULL);
		apl_errprintf( "Cursor OK" );

		apl_snprintf( lsSQL, sizeof( lsSQL ), "Select * FROM ABC" );
		lpoCursor -> Execute( lsSQL );	
		PrintResultSet(lpoCursor);
		
		lpoCursor -> Close();
		gpoMysql -> ReleaseConnection(lpoConn);

		apl_errprintf( "Reconnect now, then press any key " );
		getchar();
		
		lpoConn = gpoMysql -> GetConnection();
		CPPUNIT_ASSERT_MESSAGE("Can't get connection\n", mpoConn != APL_NULL);
		lpoCursor = lpoConn -> Cursor();
		CPPUNIT_ASSERT_MESSAGE("Can't get cursor\n", mpoCursor != APL_NULL);

		apl_snprintf( lsSQL, sizeof( lsSQL ), "Select * FROM ABC" );
		lpoCursor -> Execute( lsSQL );	
		PrintResultSet(lpoCursor);

		lpoCursor -> Close();
		gpoMysql -> ReleaseConnection(lpoConn);
	}	

    void TestMysqlInsertPerformanceAllInOne()
	{
		PRINT_TITLE_2(__func__);

        std::string loString;
        apl_int_t liIdx;
        char lpcBuf[256];

        loString.assign("TRUNCATE TABLE ABC ");
		TestMysqlExecute(loString.c_str());

		TimeCalcGuard loGuard( __func__, PRFM_INSERT_COUNT );

        loString.assign("INSERT INTO ABC (f1, f2, f3, f4) values ");

		for (liIdx=0; liIdx<PRFM_BINDINSERT_COUNT; ++liIdx)
		{
			apl_memset(lpcBuf, 0, sizeof(lpcBuf));
			apl_snprintf(lpcBuf, sizeof(lpcBuf), 
                    " (%d, 'param binding %d', 'param binding %d', %f),", 
                    10000+liIdx, liIdx, liIdx+1, 1.123456+liIdx);
            loString.append(lpcBuf, lpcBuf+apl_strlen(lpcBuf));
		}

        //remove the last comma
        loString.resize(loString.size()-1);

        TestMysqlExecute(loString.c_str(), "AllInOne insert execute failed");

		loGuard.PrintCurrentResult();
	}

};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestMysql);

int main()
{
    RUN_ALL_TEST(__FILE__);
}


