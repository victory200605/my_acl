#include "Test.h"
#include "adl/Adl.h"
#include "adl/SQLiteDatabase.h"
#include "acl/DateTime.h"
#include "acl/TimeValue.h"
#include <vector>

using namespace adl;

#define BINDING_TEST_COUNTS 1000 

// Performance test counts
#define PRFM_INSERT_COUNT 10000
#define PRFM_BINDINSERT_COUNT 10000
#define PRFM_SELECT_COUNT 10000
#define PRFM_DELETE_COUNT 10000

class CTestSQLite: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestSQLite);
#if 1 
    CPPUNIT_TEST(TestDropTable);
    CPPUNIT_TEST(TestCreateTable);
	CPPUNIT_TEST(TestInsert);
    CPPUNIT_TEST(TestSelect);
    CPPUNIT_TEST(TestUpdate);
    
    CPPUNIT_TEST(TestParaBindingInsert);
    CPPUNIT_TEST(TestParaBindingInsertStrToInt);

	// Performance Tests
    CPPUNIT_TEST(TestInsertPerformance);
    CPPUNIT_TEST(TestBindInsertPerformance);
	CPPUNIT_TEST(TestSelectPerformance);
	CPPUNIT_TEST(TestDeletePerformance);
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    IDatabase* mpoDB;
    IConnection* mpoConn;
    ICursor* mpoCursor;

    void setUp(void) 
    {
        this->mpoDB = CreateDatabase("lib=../src/sqlite/.libs/libadl_sqlite.so,name=sqlite.db,pragma=synchronous=OFF|journal_mode=PERSIST");
        //this->mpoDB = new CSQLiteDatabase("sqlite.db");

        ACL_ASSERT(this->mpoDB != APL_NULL);
	
        if (this->mpoDB->Init() != 0)
        {
            apl_errprintf("Init failed: %s\n", this->mpoDB->GetErrorMsg());
            assert(0);
        }

        this->mpoConn = this->mpoDB->GetConnection();

        //PRAGMA


        ASSERT_MESSAGE(this->mpoConn != APL_NULL);

        this->mpoCursor = this->mpoConn->Cursor();
    }

    void tearDown(void) 
    {
        this->mpoDB->ReleaseConnection(this->mpoConn);
        this->mpoDB->Destroy();

        ACL_DELETE(this->mpoDB);
        ACL_DELETE(this->mpoCursor);
    }

    bool ExecuteSQL(const char* apcSql, char const* apcMessage)
    {
        apl_int_t liRetCode = 0;

        if (apcSql != APL_NULL)
            liRetCode = mpoCursor->Execute(apcSql);
        else 
            liRetCode = mpoCursor->Execute();

        if (liRetCode != 0)
        {
            apl_errprintf("Execute %s failed,%s (code=%d)\n", 
                apcMessage, mpoCursor->GetErrorMsg(), mpoCursor->GetErrorCode() );
            return false;
        }
        else
        {
            return true;
        }
    }

#define TestExecute(sql, message, enable) \
    if (enable) \
    { \
        ASSERT_MESSAGE(ExecuteSQL(sql, message) == true);\
    } \
    else \
    { \
        ExecuteSQL(sql, message); \
    }

    void TestDropTable(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        
        TestExecute("DROP table abc", "drop table", false);
    }

    void TestCreateTable(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        TestExecute(
            "CREATE table abc("
            "   f1 integer primary key not null,"
            "   f2 varchar(20) default null,"
            "   f3 varchar(20) default null,"
            "   f4 double not null"
            ")",
            "create table", 
            true );
    }
    
    void TestInsert(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_int_t liResult = 0;
        
        //case
        //1
        TestExecute("insert into abc (f1, f2, f3, f4) VALUES (111, 'notrans', 'insert', 1.1)", "insert", true);

        //2, transsion and commit
        CPPUNIT_ASSERT_MESSAGE("Beging transsion failed", 0 == this->mpoConn->BeginTrans());
        TestExecute(
            "insert into abc (f1, f2, f3, f4) VALUES (222, 'trans', 'commit', 123456789012345.123456)", 
            "Transsion insert",
            true);
        TestExecute(
            "insert into abc (f1, f2, f3, f4) VALUES (333, 'trans', 'commit', 1.12345678901234537)", 
            "Transsion insert", 
            true);
        TestExecute(
            "insert into abc (f1, f2, f3, f4) VALUES (444, 'trans', 'commit', 1.1)",
            "Transsion insert",
            true);
        CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Commit());
        
        //3, transsion and rollback
        CPPUNIT_ASSERT_MESSAGE("Beging transsion failed", 0 == mpoConn->BeginTrans());
        TestExecute(
            "insert into abc (f1, f2, f3,f4) VALUES (555, 'trans', 'rollback', 1.1)", 
            "Transsion insert",
            true );
        TestExecute(
            "insert into abc (f1, f2, f3, f4) VALUES (666, 'trans', 'rollback', 1.1)", 
            "Transsion insert",
            true );
        TestExecute(
            "insert into abc (f1, f2, f3, f4) VALUES (777, 'trans', 'rollback', 1.1)", 
            "Transsion insert",
            true );
        CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Rollback());

        //4, get another cursor and try dupplicate insert which must failed!
        ICursor* lpoCursorSecond = mpoConn->Cursor();
        liResult = lpoCursorSecond->Execute("insert into abc (f1, f2, f3, f4) VALUES (444, 'notrans', 'atonce', 1.1)");
        CPPUNIT_ASSERT_MESSAGE("Duplicated execute insert failed", liResult != 0);
        apl_errprintf("Duplicated insert result(errmsg): (%d)%s\n", liResult, lpoCursorSecond->GetErrorMsg());
        apl_errprintf("Duplicated insert result(errno): (%d)%d\n", liResult, lpoCursorSecond->GetErrorCode());
        
        //end environment
    }

    void PrintResultSet(ICursor* apoCursor)
    {
        apl_int_t liLen = -1;
        apl_int_t liIter;
        apl_int_t liFieldCount = 0;
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
        apl_errprintf("\n");

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

    void TestSelect(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        //case
        TestExecute("select * from abc order by f1", "select", true);

        PrintResultSet(this->mpoCursor);
        
        //end environment
    }


    void TestUpdate(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_int_t liRows = 0;
        
        TestExecute(
            "update abc set f2='update', f3='oracle' where f1=333", 
            "Update",
            true);

        liRows = mpoCursor->GetAffectedRows();

        CPPUNIT_ASSERT_MESSAGE("GetField failed",  1 == liRows);
    }

    void TestComplicate(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

        apl_int_t liRows = 0;
        const char* lpcUpdate = "update abc set (f2, f3, f4)=(select f2, f3, f4 from abc where f1=444) where f1=111 or f1=222";

        TestExecute(lpcUpdate, "Update", true);
        
        //three rows should be updated (which insert in TestMysqlInsert()
        liRows = mpoCursor->GetAffectedRows();
        CPPUNIT_ASSERT_MESSAGE("GetField failed",  2 == liRows);
        apl_errprintf("the affected rows is %d\n", liRows);
    }

    void TestParaBindingInsert(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

		const char* lpcSQL = 
        	"INSERT INTO abc (f1, f2, f3, f4) values (:f1, :f2, :f3, :f4)";

        apl_int_t liIdx;
        apl_int_t liRet;

        apl_int32_t liField1;
        char lpcField2[100];
        char lpcField3[100];
        double ldField4;

        this->TestDropTable();
        this->TestCreateTable();

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

            TestExecute(APL_NULL, "Parameter binding execute", true);
        }

        CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Commit());

        //TestExecute("select * from abc", "select", true);
        //PrintResultSet(this->mpoCursor);
    }

    void TestParaBindingInsertStrToInt(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);

		const char* lpcSQL = 
        	"INSERT INTO abc (f1,f2, f3, f4) values (:f1, :f2, :f3, :f4)";

        apl_int_t liIdx;
        apl_int_t liRet;

        char lpcField1[100];
        char lpcField2[100];
        char lpcField3[100];
        double ldField4;
        
        this->TestDropTable();
        this->TestCreateTable();

		liRet = mpoCursor->Prepare( lpcSQL );
        CPPUNIT_ASSERT_MESSAGE("Parameter binding prepare failed", liRet == 0);

		liRet = mpoCursor->BindParam( "f1", lpcField1, sizeof(lpcField1) );
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
            apl_memset(lpcField1, 0, sizeof(lpcField1));
            apl_memset(lpcField2, 0, sizeof(lpcField2));
            apl_memset(lpcField3, 0, sizeof(lpcField3));
            apl_snprintf(lpcField1, sizeof(lpcField1), "%d", liIdx);
            apl_snprintf(lpcField2, sizeof(lpcField2), "param binding %d", liIdx);
            apl_snprintf(lpcField3, sizeof(lpcField3), "param binding %d", liIdx+1);
            ldField4 = 1.123456 + liIdx;

            TestExecute(APL_NULL, "Parameter binding execute", true);
        }

        CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Commit());
        
        //TestExecute("select * from abc", "select", true);
        //PrintResultSet(this->mpoCursor);
    }

    void TestParaBindingSelect(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        const char* lpcSQL = 
            "SELECT * FROM abc where f2 = :Content";
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
            TestExecute(APL_NULL, "Parameter binding select execute", true);

            PrintResultSet(this->mpoCursor);
        }
    }

    void TestParaBindingDuplicatedParam(void)
    {
        //start case environment
        PRINT_TITLE_2(__func__);
        const char* lpcSQL = 
            "SELECT * FROM abc where f1 > :FieldId and f2 = :Content or f3 = :Content";
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
            TestExecute(APL_NULL, "Parameter binding select execute", true);

            PrintResultSet(this->mpoCursor);
        }
    }

	void TestInsertPerformance()
	{
		PRINT_TITLE_2( __func__ );

		char lsSQL[128];
		apl_int_t liIndex;

        this->TestDropTable();
        this->TestCreateTable();
		
        START_LOOP(PRFM_INSERT_COUNT)
        {
            //mpoConn->BeginTrans();
            liIndex = __liN;

            apl_snprintf( lsSQL, sizeof( lsSQL ), 
                "INSERT INTO abc( f1, f2, f3, f4 ) VALUES( %d, '%dfree', '%dfish', %d1.0 )", 
                liIndex + 1, liIndex, liIndex, liIndex );
            
            TestExecute(lsSQL, "insert", true);
            //mpoConn->Commit();
        }
        END_LOOP();
	}


	void TestBindInsertPerformance()
	{
		PRINT_TITLE_2(__func__);
		const char* lpcSQL = 
        	"INSERT INTO abc (f1, f2, f3, f4) values (:f1, :f2, :f3, :f4)";

        apl_int_t liIdx;
        apl_int_t liRet;

        apl_int32_t liField1;
        char lpcField2[100];
        char lpcField3[100];
        double ldField4;

        this->TestDropTable();
        this->TestCreateTable();

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

        START_LOOP(PRFM_BINDINSERT_COUNT)
        {
            //CPPUNIT_ASSERT_MESSAGE("Beging transsion failed", 0 == mpoConn->BeginTrans());
            liIdx = __liN;
            //for (liIdx=0; liIdx<PRFM_BINDINSERT_COUNT; ++liIdx)
            {
                liField1 = 10000 + liIdx;
                apl_memset(lpcField2, 0, sizeof(lpcField2));
                apl_memset(lpcField3, 0, sizeof(lpcField3));
                apl_snprintf(lpcField2, sizeof(lpcField2), "param binding %d", liIdx);
                apl_snprintf(lpcField3, sizeof(lpcField3), "param binding %d", liIdx+1);
                ldField4 = 1.123456 + liIdx;
            
                TestExecute(APL_NULL, "Parameter binding execute", true);
            }

            //CPPUNIT_ASSERT_MESSAGE("End transsion failed", 0 == mpoConn->Commit());
        }
        END_LOOP();
	}

	void TestDeletePerformance()
	{
		PRINT_TITLE_2( __func__ );

		char lsSQL[128];
		apl_int_t liIndex;
		
        START_LOOP(1)
        {
            mpoConn->BeginTrans();
            for( liIndex = 0; liIndex < PRFM_DELETE_COUNT; ++liIndex )
            {
                apl_snprintf( lsSQL, sizeof( lsSQL ), 
                    "DELETE FROM abc WHERE f1=%d", 
                    10000 + liIndex);
                TestExecute(lsSQL, "delete", true);
            }
            mpoConn->Commit();
        }
        END_LOOP();
	}

	void TestSelectPerformance()
	{
		PRINT_TITLE_2( __func__ );

		char lsSQL[128];
		
        START_LOOP(1)
        {
            mpoConn->BeginTrans();

            apl_snprintf( lsSQL, sizeof( lsSQL ), "Select * FROM abc" );
            TestExecute(lsSQL, "select", true);

            mpoCursor->SetPreFetchRows(1);

            while (mpoCursor->FetchNext() == 0)
            {
            }

            mpoConn->Commit();
        }
        END_LOOP();
	}
};


CPPUNIT_TEST_SUITE_REGISTRATION(CTestSQLite);

int main()
{
    RUN_ALL_TEST(__FILE__);
}


