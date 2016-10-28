#include "Test.h"
#include "adl/Adl.h"
#include "acl/Timestamp.h"
#include <vector>

using namespace adl;
/*
 * database: test, TABLE: ABC
 * --------------------------
 * Filed    Type
 * -----    ----
 * f1       int(11) 
 * f2       varchar(100) 
 * f2       varchar(100) 
 * f4       double
 */
#define THREADS_FOR_GETTING_CONN 3
#define COURSOR_COUNTS_PER_CONN 4
#define EXECUTE_COUNTS_PER_CURSOR 500
#define SELECT_INTERVAL 499

#define BINDING_TEST_COUNTS 1000

// Performance test counts
#define PRFM_INSERT_COUNT 10000
#define PRFM_BINDINSERT_COUNT 10000
#define PRFM_SELECT_COUNT 10000
#define PRFM_DELETE_COUNT 10000

#define DROP_TABLE "DROP TABLE ABC"
/*
    create table ABC (
            f1     NUMBER(10)                      not null,
            f2     VARCHAR(100)                    not null,
            f3     VARCHAR(100)                    not null,
            f4     BINARY_DOUBLE                   not null,
            constraint PK_ABC primary key (f1)
            );
*/

#define CREATE_TABLE "create table ABC (f1 NUMBER(11) not null, f2 VARCHAR(100) not null, f3 VARCHAR(100) not null, f4 FLOAT not null, f5 raw(20), constraint PK_ABC primary key (f1))"


IDatabase* gpoOracle = APL_NULL;
//CDataBase* gpoOracle = APL_NULL;

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

        //apl_errprintf("cursor %d running\n", muId);
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
        //apl_errprintf("cursor %d exiting\n", muId);
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
        gpoOracle->ReleaseConnection(this->mpoConn);
        //apl_errprintf("conn %d exiting\n", muIndex);
    }

private:
    IConnection* mpoConn;

    apl_size_t muIndex;
    apl_size_t muCursorCount;

    std::vector<CCursorThread*> moVetCursor;
};

class CTestOracle: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestOracle);
    CPPUNIT_TEST(TestOracleDropTable);
    CPPUNIT_TEST(TestOracleCreateTable);
    CPPUNIT_TEST(TestOracleInsert);
    CPPUNIT_TEST(TestOracleSelect);
    CPPUNIT_TEST(TestOracleUpdate);
    CPPUNIT_TEST(TestOracleComplicate);
    //re-new the table for multi cursors
    CPPUNIT_TEST(TestOracleDropTable);
    CPPUNIT_TEST(TestOracleCreateTable);
    CPPUNIT_TEST(TestMultiCursors);

    //re-new the table for parameters bindin
    CPPUNIT_TEST(TestOracleDropTable);
    CPPUNIT_TEST(TestOracleCreateTable);
    CPPUNIT_TEST(TestParaBindingInsert);
    CPPUNIT_TEST(TestParaBindingSelect);
    CPPUNIT_TEST(TestParaBindingDuplicatedParam);

	//CPPUNIT_TEST(TestOracleReconnect);

	//Preformance tests
    CPPUNIT_TEST(TestOracleInsertPerformance);
	CPPUNIT_TEST(TestOracleBindInsertPerformance);
    CPPUNIT_TEST(TestOracleSelectPerformance);
	CPPUNIT_TEST(TestOracleDeletePerformance);
#if 0
    CPPUNIT_TEST(TestBatchInsert);
#endif
    CPPUNIT_TEST_SUITE_END();
public:
    IConnection* mpoConn;
    ICursor* mpoCursor;

    void setUp(void) 
    {
        //gpoOracle = new COracleDataBase("hezk", "hezk", "127001", 5);
        //gpoOracle = new CDataBase("ismg29", "ismg29", "103355", 5);
        //gpoOracle = new CDataBase("ptop_dyf", "ptop_dyf", "oracle", 5);
	    gpoOracle = CreateDatabase(
            "lib=../src/oracle/.libs/libadl_oracle2.so,user=ptop_dyf,pass=ptop_dyf,host=oracle,connpool=5");

        assert(gpoOracle != APL_NULL);

        if (gpoOracle -> Init() != 0)
        {
            apl_errprintf("Init failed: %s\n", gpoOracle->GetErrorMsg());
            assert(0);
        }

        //get conn
        mpoConn = gpoOracle->GetConnection();
        assert(mpoConn != APL_NULL);

        //get cursor
        mpoCursor = mpoConn->Cursor();
        assert(mpoCursor != APL_NULL);
    }

    void tearDown(void) 
    {
        if (mpoCursor != APL_NULL) 
        {
            mpoCursor->Close();
        }

        if (gpoOracle != APL_NULL)
        {
            if (mpoConn != APL_NULL)
            {
                gpoOracle->ReleaseConnection(mpoConn);
            }

            delete gpoOracle;
        }

        gpoOracle = APL_NULL;
    }

    void TestOracleExecute(const char* apcSql, const char* apcErrMsg = APL_NULL, bool abAssert=true)
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

    void TestOracleDropTable(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        TestOracleExecute(DROP_TABLE, "drop table failed", false);
    }

    void TestOracleCreateTable(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        TestOracleExecute(CREATE_TABLE);
    }

    void TestOracleInsert(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_int_t liResult = 0;
        ICursor* lpoCursorSecond = APL_NULL;

        //case
        //1
        TestOracleExecute("insert into ABC (f1, f2, f3, f4) VALUES (111, 'notrans', 'atonce', 1.1)");

        //2, transsion and commit
        CPPUNIT_ASSERT_MESSAGE("Beging transsion failed", 0 == mpoConn->BeginTrans());
        TestOracleExecute(
                "insert into ABC (f1, f2, f3, f4) VALUES (222, 'trans', 'commit', 123456789012345.123456)", 
                "Transsion insert failed");
        TestOracleExecute(
                "insert into ABC (f1, f2, f3, f4) VALUES (333, 'trans', 'commit', 1.12345678901234537)", 
                "Transsion insert failed");
        TestOracleExecute(
                "insert into ABC (f1, f2, f3, f4) VALUES (444, 'trans', 'commit', 1.1)",
                "Transsion insert failed");

        CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Commit());
        
        //3, transsion and rollback
        CPPUNIT_ASSERT_MESSAGE("Beging transsion failed", 0 == mpoConn->BeginTrans());
        TestOracleExecute(
                "insert into ABC (f1, f2, f3,f4) VALUES (555, 'trans', 'rollback', 1.1)", 
                "Transsion insert failed");
        TestOracleExecute(
                "insert into ABC (f1, f2, f3, f4) VALUES (666, 'trans', 'rollback', 1.1)", 
                "Transsion insert failed");
        TestOracleExecute(
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
        //CPPUNIT_ASSERT_MESSAGE("Select failed, row count error\n", liRowCount != 0);
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

    void TestOracleSelect(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        const char * SQLSTR1 = "select * from ABC order by f1";
        //const char * SQLSTR1 = "select count(*) from ABC";

        TestOracleExecute(SQLSTR1, "select failed");

        PrintResultSet(this->mpoCursor);
        //end environment
    }

    void TestOracleUpdate(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_int_t liRows = 0;
        const char* lpcUpdate = "  update ABC set f2='update', f3='oracle' where f1=333";

        TestOracleExecute(lpcUpdate, "Update failed");
        //three rows should be updated (which insert in TestOracleInsert()
        liRows = mpoCursor->GetAffectedRows();
        CPPUNIT_ASSERT_MESSAGE("GetField failed",  1 == liRows);
    }

    void TestOracleComplicate(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_int_t liRows = 0;
        const char* lpcUpdate = "update abc set (f2, f3, f4)=(select f2, f3, f4 from abc where f1=444) where f1=111 or f1=222";

        TestOracleExecute(lpcUpdate, "Update failed");
        //three rows should be updated (which insert in TestOracleInsert()
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
            lpcConn = gpoOracle->GetConnection();
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
        	"INSERT INTO ABC (f1, f2, f3, f4, f5) values (:f1, :f2, :f3, :f4, :f5)";

        apl_int_t liIdx;
        apl_int_t liRet;

        apl_int32_t liField1;
        char lpcField2[100];
        char lpcField3[100];
        double ldField4;
        char lpcField5[5] = {'0', '1', '2', '3', '4'};

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

		liRet = mpoCursor->BindParam( "f5", &lpcField5, sizeof(lpcField5), true);
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

            TestOracleExecute(APL_NULL, "Parameter binding execute failed");
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

        liRet = mpoCursor->Prepare(lpcSQL);
        CPPUNIT_ASSERT_MESSAGE("Parameter binding prepare failed", liRet == 0);

		liRet = mpoCursor->BindParam( "Content", lpcContent, sizeof(lpcContent) );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

        for (liIdx=0; liIdx<BINDING_TEST_COUNTS; ++liIdx)
        {
            apl_memset(lpcContent, 0, sizeof(lpcContent));
            apl_snprintf(lpcContent, sizeof(lpcContent), "param binding %d", liIdx);
            TestOracleExecute(APL_NULL, "Parameter binding select execute failed");

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

		liRet = mpoCursor->BindParam( "FieldId", &liField1);
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

		liRet = mpoCursor->BindParam( "Content", lpcContent, sizeof(lpcContent) );
		CPPUNIT_ASSERT_MESSAGE("Parameter binding failed", liRet == 0);

        for (liIdx=0; liIdx<BINDING_TEST_COUNTS; ++liIdx)
        {
            liField1 = 10000 + liIdx;
            apl_memset(lpcContent, 0, sizeof(lpcContent));
            apl_snprintf(lpcContent, sizeof(lpcContent), "param binding %d", liIdx+1);
            TestOracleExecute(APL_NULL, "Parameter binding select execute failed");

            PrintResultSet(this->mpoCursor);
        }
    }

	void TestOracleInsertPerformance()
	{   
		PRINT_TITLE_2( __func__ );
	
		char lsSQL[128];
		apl_int_t liIndex;
	
		apl_snprintf( lsSQL, sizeof(lsSQL), "TRUNCATE TABLE ABC " );
		TestOracleExecute( lsSQL );
	
		TimeCalcGuard loGuard( __func__, PRFM_INSERT_COUNT );
	
		mpoConn->BeginTrans();
		for( liIndex = 0; liIndex < PRFM_INSERT_COUNT; ++liIndex )
		{	   
			apl_snprintf( lsSQL, sizeof( lsSQL ), 
				"INSERT INTO ABC( f1, f2, f3, f4 ) VALUES( %d, '%dfree', '%dfish', %d1.0 )",
				liIndex + 1, liIndex, liIndex, liIndex );
			TestOracleExecute( lsSQL );
		}

		mpoConn->Commit();
	
		loGuard.PrintCurrentResult();
	}

    void TestOracleBindInsertPerformance(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

		char lsSQL[128] = {0};
		const char* lpcSQL = 
        	"INSERT INTO ABC (f1, f2, f3, f4) values (:f1, :f2, :f3, :f4)";

		apl_snprintf( lsSQL, sizeof(lsSQL), "TRUNCATE TABLE ABC " );
		TestOracleExecute( lsSQL );

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

		TimeCalcGuard loGuard( __func__, PRFM_INSERT_COUNT );

        for (liIdx=0; liIdx<PRFM_INSERT_COUNT; ++liIdx)
        {
            liField1 = 10000 + liIdx;
            apl_memset(lpcField2, 0, sizeof(lpcField2));
            apl_memset(lpcField3, 0, sizeof(lpcField3));
            apl_snprintf(lpcField2, sizeof(lpcField2), "param binding %d", liIdx);
            apl_snprintf(lpcField3, sizeof(lpcField3), "param binding %d", liIdx+1);
            ldField4 = 1.123456 + liIdx;

            TestOracleExecute(APL_NULL, "Parameter binding execute failed");
        }

        CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Commit());
		loGuard.PrintCurrentResult();
	}

	void TestOracleDeletePerformance()
	{   
		PRINT_TITLE_2( __func__ );
	
		char lsSQL[128];
		apl_int_t liIndex;
	
		apl_snprintf( lsSQL, sizeof(lsSQL), "TRUNCATE TABLE ABC " );
		TestOracleExecute( lsSQL );
	
		TimeCalcGuard loGuard( __func__, PRFM_INSERT_COUNT );
	
		mpoConn->BeginTrans();
		for( liIndex = 0; liIndex < PRFM_DELETE_COUNT; ++liIndex )
		{	   
			apl_snprintf( lsSQL, sizeof( lsSQL ), 
				"DELETE FROM ABC WHERE f1=%d",
				liIndex + 10000);
			TestOracleExecute( lsSQL );
		}

		mpoConn->Commit();
	
		loGuard.PrintCurrentResult();
	}

	void TestOracleSelectPerformance()
	{   
		PRINT_TITLE_2( __func__ );

		char lsSQL[128];
		
		TimeCalcGuard loGuard( __func__, PRFM_SELECT_COUNT);

		mpoConn->BeginTrans();

		apl_snprintf( lsSQL, sizeof( lsSQL ), "Select * FROM ABC" );
		TestOracleExecute( lsSQL );

        mpoCursor->SetPreFetchRows(0);

        while (mpoCursor->FetchNext() == 0)
        {
		}

		mpoConn->Commit();

		loGuard.PrintCurrentResult();
	}

	void TestOracleReconnect()
	{
		PRINT_TITLE_2( __func__ );
		
		char lsSQL[128];
		ICursor* lpoCursor;
		IConnection* lpoConn;

		lpoConn = gpoOracle -> GetConnection();
		CPPUNIT_ASSERT_MESSAGE("Can't get connection\n", mpoConn != APL_NULL);
		apl_errprintf("GetConnection OK\n");

		apl_errprintf( "Disconnect now, then press any key \n" );
		getchar();		

		lpoCursor = lpoConn -> Cursor();
		CPPUNIT_ASSERT_MESSAGE("Can't get cursor\n", mpoCursor != APL_NULL);
		apl_errprintf( "Cursor OK" );

		apl_snprintf( lsSQL, sizeof( lsSQL ), "Select * FROM ABC" );
		lpoCursor -> Execute( lsSQL );	
		PrintResultSet(lpoCursor);
		
		lpoCursor -> Close();
		gpoOracle -> ReleaseConnection(lpoConn);

		apl_errprintf( "Reconnect now, then press any key " );
		getchar();
		
		lpoConn = gpoOracle -> GetConnection();
		CPPUNIT_ASSERT_MESSAGE("Can't get connection\n", mpoConn != APL_NULL);
		lpoCursor = lpoConn -> Cursor();
		CPPUNIT_ASSERT_MESSAGE("Can't get cursor\n", mpoCursor != APL_NULL);

		apl_snprintf( lsSQL, sizeof( lsSQL ), "Select * FROM ABC" );
		lpoCursor -> Execute( lsSQL );	
		PrintResultSet(lpoCursor);

		lpoCursor -> Close();
		gpoOracle -> ReleaseConnection(lpoConn);
	}	

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestOracle);

int main()
{
    RUN_ALL_TEST(__FILE__);
}



