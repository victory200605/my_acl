
#include "DBOperator.h"

CDBWrapper::CDBWrapper( adl::IDatabase* apoDb )
    : mpoDb(apoDb)
    , miErrorCode(0)
{
}

CDBWrapper::~CDBWrapper(void)
{
    for (apl_size_t luN = 0; luN < this->moBindParameteres.size(); luN++)
    {
        ACL_DELETE(this->moBindParameteres[luN]);
    }
}

apl_ssize_t CDBWrapper::ExecuteNonQuery( char const* apcSQL )
{
    apl_ssize_t liResult = -1;
    adl::IConnection* lpoConn = APL_NULL;
    adl::ICursor* lpoCursor = APL_NULL;
    
    this->ClearLastErrorMessage();

    do
    {
        //Get db connection
        if ( (lpoConn = this->mpoDb->GetConnection() ) == APL_NULL)
        {
            this->SetMessage(this->mpoDb->GetErrorCode(), this->mpoDb->GetErrorMsg() );

            break;
        }

        //Get statment execute cursor
        if ( (lpoCursor = lpoConn->Cursor() ) == APL_NULL)
        {
            this->SetMessage(lpoConn->GetErrorCode(), lpoConn->GetErrorMsg() );

            break;
        }

        //Prepare SQL statement
        if (lpoCursor->Prepare(apcSQL) != 0)
        {
            this->SetMessage(lpoCursor->GetErrorCode(), lpoCursor->GetErrorMsg() );

            break;
        }

        //Bind all parameter
        if (this->moBindParameteres.size() > 0)
        {
            for (apl_size_t luN = 0; luN < this->moBindParameteres.size(); luN++)
            {
                CBindParameter* lpoParameter = this->moBindParameteres[luN];
                if (lpoCursor->BindParam(
                    lpoParameter->moName.c_str(),
                    lpoParameter->moValue.c_str(),
                    lpoParameter->moValue.length(),
                    true) != 0)
                {
                    this->SetMessage(lpoCursor->GetErrorCode(), lpoCursor->GetErrorMsg() );

                    break;
                }
            }
        }

        //Catch a db error and break it
        if (this->GetErrorCode() != 0)
        {
            break;
        }

        if (lpoCursor->Execute() != 0)
        {
            this->SetMessage(lpoCursor->GetErrorCode(), lpoCursor->GetErrorMsg() );

            break;
        }

        liResult = lpoCursor->GetAffectedRows();
    }
    while(false);

    //Release all resource
    if (lpoCursor != APL_NULL)
    {
        lpoCursor->Close();
    }

    if (lpoConn != APL_NULL)
    {
        this->mpoDb->ReleaseConnection(lpoConn);
    }

    return liResult;
}

apl_ssize_t CDBWrapper::ExecuteAndQuery( char const* apcSQL, std::vector<RowType>& aoResultset )
{
    apl_ssize_t liResult = -1;
    apl_size_t  luFieldCount = 0;
    adl::IConnection* lpoConn = APL_NULL;
    adl::ICursor* lpoCursor = APL_NULL;
    
    aoResultset.clear();
    this->ClearLastErrorMessage();

    do
    {
        //Get db connection
        if ( (lpoConn = this->mpoDb->GetConnection() ) == APL_NULL)
        {
            this->SetMessage(this->mpoDb->GetErrorCode(), this->mpoDb->GetErrorMsg() );

            break;
        }

        //Get statment execute cursor
        if ( (lpoCursor = lpoConn->Cursor() ) == APL_NULL)
        {
            this->SetMessage(lpoConn->GetErrorCode(), lpoConn->GetErrorMsg() );

            break;
        }

        if (lpoCursor->Execute(apcSQL) != 0)
        {
            this->SetMessage(lpoCursor->GetErrorCode(), lpoCursor->GetErrorMsg() );

            break;
        }

        luFieldCount = lpoCursor->GetFieldNameCount();
        
        //Get select resultset
        while(lpoCursor->FetchNext() == 0)
        {
            aoResultset.push_back(std::vector<std::string>() );
            RowType& loRow = aoResultset.back();

            loRow.resize(luFieldCount);

            char const* lpcValue = APL_NULL;
            apl_int_t   liLen = 0;
            for (apl_size_t luN = 0; luN < luFieldCount; luN++)
            {
                lpcValue = lpoCursor->GetField(luN, &liLen);

                loRow[luN].assign(lpcValue, liLen);
            }
        }

        liResult = aoResultset.size();
    }
    while(false);

    //Release all resource
    if (lpoCursor != APL_NULL)
    {
        lpoCursor->Close();
    }

    if (lpoConn != APL_NULL)
    {
        this->mpoDb->ReleaseConnection(lpoConn);
    }

    return liResult;
}

void CDBWrapper::BindParameter( char const* apcName, char const* apcValue, apl_size_t auLen )
{
    CBindParameter* lpoParameter = APL_NULL;
    ACL_NEW_ASSERT(lpoParameter, CBindParameter);

    lpoParameter->moName = apcName;
    lpoParameter->moValue.assign(apcValue, auLen);

    this->moBindParameteres.push_back(lpoParameter);
}


void CDBWrapper::SetMessage( apl_int_t aiCode, char const* apcMessage )
{
    this->miErrorCode = aiCode;
    this->moErrorMessage = apcMessage;
}

void CDBWrapper::ClearLastErrorMessage(void)
{
    this->miErrorCode = 0;
    this->moErrorMessage.clear();
}

apl_int_t CDBWrapper::GetErrorCode(void)
{
    return this->miErrorCode;
}

char const* CDBWrapper::GetErrorMsg(void)
{
    return this->moErrorMessage.c_str();
}

////////////////////////////////////////////////////////////////////////////////////////
CDBCondition::CDBCondition(void)
{
    this->moCond.reserve(128);
}

CDBCondition::CDBCondition( char const* apcName, char const* apcValue )
{
    this->moCond = apcName;
    this->moCond += "='";
    this->moCond += apcValue;
    this->moCond += '\'';
}

CDBCondition::CDBCondition( char const* apcName, apl_intmax_t aiValue )
{
    char lacTemp[64];
    apl_snprintf(lacTemp, sizeof(lacTemp), "%"APL_PRIdMAX, aiValue);

    this->moCond = apcName;
    this->moCond += '=';
    this->moCond += lacTemp;
}

CDBCondition::CDBCondition( char const* apcName, double adValue )
{
    char lacTemp[64];
    apl_snprintf(lacTemp, sizeof(lacTemp), "%lf", adValue);

    this->moCond = apcName;
    this->moCond += '=';
    this->moCond += lacTemp;
}

char const* CDBCondition::GetCStr(void) const
{
    return this->moCond.c_str();
}

std::string const& CDBCondition::GetStr(void) const
{
    return this->moCond;
}

CDBCondition operator || ( CDBCondition const& aoLhs, CDBCondition const& aoRhs )
{
    CDBCondition loCond;

    loCond.moCond = '(';
    loCond.moCond += aoLhs.moCond;
    loCond.moCond += " or ";
    loCond.moCond += aoRhs.moCond;
    loCond.moCond += ')';

    return loCond;
}

CDBCondition operator && ( CDBCondition const& aoLhs, CDBCondition const& aoRhs )
{
    CDBCondition loCond;

    loCond.moCond = '(';
    loCond.moCond += aoLhs.moCond;
    loCond.moCond += " and ";
    loCond.moCond += aoRhs.moCond;
    loCond.moCond += ')';

    return loCond;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CDBWhereExpression::CDBWhereExpression(void)
{
    this->moWhere.reserve(128);
}

void CDBWhereExpression::AddCondition( char const* apcName, char const* apcValue )
{
    this->AddCondition(apcName, apcValue, true);
}
    
void CDBWhereExpression::AddCondition( char const* apcName, apl_intmax_t aiValue )
{
    char lacTemp[64];

    apl_snprintf(lacTemp, sizeof(lacTemp), "%"APL_PRIdMAX, aiValue);

    this->AddCondition(apcName, lacTemp, false);
}
    
void CDBWhereExpression::AddCondition( char const* apcName, double adValue )
{
    char lacTemp[64];

    apl_snprintf(lacTemp, sizeof(lacTemp), "%lf", adValue);

    this->AddCondition(apcName, lacTemp, false);
}
    
void CDBWhereExpression::AddCondition( char const* apcName, char const* apcValue, bool abIsText )
{
    if (this->moWhere.length() > 0)
    {
        this->moWhere += " and ";
    }
    else
    {
        this->moWhere = " WHERE ";
    }
    
    if (abIsText)
    {
        this->moWhere += apcName;
        this->moWhere += "=\'";
        this->moWhere += apcValue;
        this->moWhere += '\'';
    }
    else
    {
        this->moWhere += apcName;
        this->moWhere += '=';
        this->moWhere += apcValue;
    }
}

void CDBWhereExpression::SetCondition( CDBCondition const& aoCond )
{
    this->moWhere = " WHERE ";
    this->moWhere += aoCond.GetStr();
}
    
std::string const& CDBWhereExpression::GetWhere(void) const
{
    return this->moWhere;
}

////////////////////////////////////////////////////////////////////////////////////////////
CDBCreateOperator::CDBCreateOperator( adl::IDatabase* apoDb, char const* apcTableName )
    : CDBWrapper(apoDb)
    , moTableName(apcTableName)
{
}

void CDBCreateOperator::AddField(
    char const* apcName,
    char const* apcType,
    bool abIsNotNull,
    bool abIsPrimary, 
    bool abIsAutoIncrement )
{
    if (this->moFields.length() > 0)
    {
        this->moFields += ',';
    }

    this->moFields += apcName;
    this->moFields += ' ';
    this->moFields += apcType;

    if (abIsNotNull)
    {
        this->moFields += " NOT NULL";
    }
    
    if (abIsPrimary)
    {
        this->moFields += " PRIMARY KEY";
    }
    
    if (abIsAutoIncrement)
    {
        this->moFields += " AUTOINCREMENT"; 
    }
}

void CDBCreateOperator::AddIndex( char const* apcColumn, bool abIsDesc )
{
    if (this->moIndexs.length() > 0)
    {
        this->moIndexs += ',';
    }

    this->moIndexs += apcColumn;

    if (abIsDesc)
    {
        this->moIndexs += " DESC";
    }
}

void CDBCreateOperator::AddUniqueIndex( char const* apcColumn, bool abIsDesc )
{
    if (this->moUniqueIndexs.length() > 0)
    {
        this->moUniqueIndexs += ',';
    }

    this->moUniqueIndexs += apcColumn;

    if (abIsDesc)
    {
        this->moUniqueIndexs += " DESC";
    }
}

apl_ssize_t CDBCreateOperator::Execute(void)
{
    std::string loSQL;
    
    ////Reserve SQL string length
    loSQL.reserve(1024);

    loSQL = "CREATE TABLE ";
    loSQL += this->moTableName;
    loSQL += '(';
    loSQL += this->moFields;
    loSQL += ')';

    if (this->moIndexs.length() > 0)
    {
        loSQL += ";CREATE INDEX index_";
        loSQL += this->moTableName;
        loSQL += " ON ";
        loSQL += this->moTableName;
        loSQL += '(';
        loSQL += this->moIndexs;
        loSQL += ')';
    }

    if (this->moUniqueIndexs.length() > 0)
    {
        loSQL += ";CREATE UNIQUE INDEX unique_index_";
        loSQL += this->moTableName;
        loSQL += " ON ";
        loSQL += this->moTableName;
        loSQL += '(';
        loSQL += this->moUniqueIndexs;
        loSQL += ')';
    }

    return this->ExecuteNonQuery(loSQL.c_str() );
}

////////////////////////////////////////////////////////////////////////////////////////////
CDBDropOperator::CDBDropOperator( adl::IDatabase* apoDb, char const* apcTableName )
    : CDBWrapper(apoDb)
    , moTableName(apcTableName)
{
}

apl_ssize_t CDBDropOperator::Execute(void)
{
    std::string loSQL;
    
    ////Reserve SQL string length
    loSQL.reserve(1024);

    loSQL = "DROP TABLE ";
    loSQL += this->moTableName;

    return this->ExecuteNonQuery(loSQL.c_str() );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDBSelectOperator::CDBSelectOperator( adl::IDatabase* apoDb, char const* apcTableName )
    : CDBWrapper(apoDb)
    , moTableName(apcTableName)
    , muFieldCount(0)
{
    this->moFields.reserve(128);
    this->moFields = '*';
}

void CDBSelectOperator::AddField( char const* apcFieldName )
{
    if (this->muFieldCount > 0)
    {
        this->moFields += ",";
        this->moFields += apcFieldName;
    }
    else
    {
        this->moFields = apcFieldName;
    }

    this->muFieldCount++;
}

apl_ssize_t CDBSelectOperator::Execute(void)
{
    std::string loSQL;
    apl_ssize_t liResult = -1;
    
    this->moResultset.clear();

    ////Reserve SQL string length
    loSQL.reserve(1024);

    loSQL = "SELECT ";
    loSQL += this->moFields;
    loSQL += " FROM ";
    loSQL += this->moTableName;
    loSQL += this->GetWhere();

    liResult = this->ExecuteAndQuery(loSQL.c_str(), this->moResultset);

    return liResult;
}

apl_size_t CDBSelectOperator::GetRowCount(void)
{
    return this->moResultset.size();
}

std::string const& CDBSelectOperator::GetField( apl_size_t auRow, apl_size_t auColumn ) const
{
    return this->moResultset[auRow][auColumn];
}

///////////////////////////////////////////////////////////////////////////////////////////
CDBInsertOperator::CDBInsertOperator( adl::IDatabase* apoDb, char const* apcTableName )
    : CDBWrapper(apoDb)
    , moTableName(apcTableName)
{
    this->moFields.reserve(128);
    this->moValues.reserve(128);
}

CDBInsertOperator::~CDBInsertOperator(void)
{
}

void CDBInsertOperator::AddField( char const* apcName, char const* apcValue )
{
    this->AddField(apcName, apcValue, true);
}

void CDBInsertOperator::AddField( char const* apcName, char const* apcValue, apl_size_t auLen )
{
    if (this->moFields.length() > 0)
    {
        this->moFields += ',';
        this->moFields += apcName;
        
        this->moValues += ",:";
        this->moValues += apcName;
    }
    else
    {
        this->moFields = apcName;
        
        this->moValues = ":";
        this->moValues += apcName;
    }

    this->BindParameter(apcName, apcValue, auLen);
}

void CDBInsertOperator::AddField( char const* apcName, apl_intmax_t aiValue )
{
    char lacTemp[64];

    apl_snprintf(lacTemp, sizeof(lacTemp), "%"APL_PRIdMAX, aiValue);

    this->AddField(apcName, lacTemp, false);
}

void CDBInsertOperator::AddField( char const* apcName, double adValue )
{
    char lacTemp[64];

    apl_snprintf(lacTemp, sizeof(lacTemp), "%lf", adValue);

    this->AddField(apcName, lacTemp, false);
}

void CDBInsertOperator::AddField( char const* apcName, char const* apcValue, bool abIsText )
{
    if (this->moFields.length() > 0)
    {
        this->moFields += ',';
        this->moValues += ',';
    }

    this->moFields += apcName;
 
    if (abIsText)
    {
        this->moValues += '\'';
        this->moValues += apcValue;
        this->moValues += '\'';
    }
    else
    {
        this->moValues += apcValue;
    }
}

apl_ssize_t CDBInsertOperator::Execute(void)
{
    std::string loSQL;
    apl_ssize_t liResult = -1;

    ////Reserve SQL string length
    loSQL.reserve(1024);

    loSQL = "INSERT INTO ";
    loSQL += this->moTableName;
    loSQL += '(';
    loSQL += this->moFields;
    loSQL += ") Values(";
    loSQL += this->moValues;
    loSQL += ')';

    liResult = this->ExecuteNonQuery(loSQL.c_str() );

    return liResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////
CDBUpdateOperator::CDBUpdateOperator( adl::IDatabase* apoDb, char const* apcTableName )
    : CDBWrapper(apoDb)
    , moTableName(apcTableName)
{
}

void CDBUpdateOperator::AddField( char const* apcName, char const* apcValue )
{
    this->AddField(apcName, apcValue, true);
}

void CDBUpdateOperator::AddField( char const* apcName, char const* apcValue, apl_size_t auLen )
{
    if (this->moSet.length() > 0)
    {
        this->moSet += ',';
    }

    this->moSet += apcName;
    this->moSet += "=:";
    this->moSet += apcName;

    this->BindParameter(apcName, apcValue, auLen);
}

void CDBUpdateOperator::AddField( char const* apcName, apl_intmax_t aiValue )
{
    char lacTemp[64];

    apl_snprintf(lacTemp, sizeof(lacTemp), "%"APL_PRIdMAX, aiValue);

    this->AddField(apcName, lacTemp, false);
}

void CDBUpdateOperator::AddField( char const* apcName, double adValue )
{
    char lacTemp[64];

    apl_snprintf(lacTemp, sizeof(lacTemp), "%lf", adValue);

    this->AddField(apcName, lacTemp, false);
}

void CDBUpdateOperator::AddField( char const* apcName, char const* apcValue, bool abIsText )
{
    if (this->moSet.length() > 0)
    {
        this->moSet += ',';
    }
    
    this->moSet += apcName;

    if (abIsText)
    {
        this->moSet += "=\'";
        this->moSet += apcValue;
        this->moSet += '\'';
    }
    else
    {
        this->moSet += apcValue;
    }
}

apl_ssize_t CDBUpdateOperator::Execute(void)
{
    std::string loSQL;
    apl_ssize_t liResult = -1;

    ////Reserve SQL string length
    loSQL.reserve(1024);

    loSQL = "UPDATE ";
    loSQL += this->moTableName;
    loSQL += " SET ";
    loSQL += this->moSet;
    loSQL += ' ';
    loSQL += this->GetWhere();

    liResult = this->ExecuteNonQuery(loSQL.c_str() );

    return liResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
CDBDeletOperator::CDBDeletOperator( adl::IDatabase* apoDb, char const* apcTableName )
    : CDBWrapper(apoDb)
    , moTableName(apcTableName)
{
}

apl_ssize_t CDBDeletOperator::Execute(void)
{
    std::string loSQL;
    apl_ssize_t liResult = -1;

    ////Reserve SQL string length
    loSQL.reserve(1024);

    loSQL = "DELETE FROM ";
    loSQL += this->moTableName;
    loSQL += ' ';
    loSQL += this->GetWhere();

    liResult = this->ExecuteNonQuery(loSQL.c_str() );

    return liResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//Unit Test

#if 0

#define START_LOOP( count ) \
{ \
    acl::CTimestamp __loStart, __loEnd; \
    __loStart.Update(acl::CTimestamp::PRC_USEC); \
    apl_int_t __liCount = count; \
    for( apl_int_t __liN = 0; __liN < __liCount; __liN++ )

#define END_LOOP() \
    __loEnd.Update(acl::CTimestamp::PRC_USEC); \
    double __ldUseTime = double( __loEnd.Msec() - __loStart.Msec() ) / 1000; \
    printf("Use time=%lf, %lf/pre\n", __ldUseTime, double(__liCount) / __ldUseTime); \
}

#include "acl/Timestamp.h"
#include "acl/Timestamp.h"
#include "acl/FileStream.h"
#include "acl/MemoryBlock.h"
#include "adl/SQLiteDatabase.h"

#define LOOP_COUNT 10

void TestCreate( adl::IDatabase* apoDb )
{
    CDBCreateOperator loCmd(apoDb, "abc");

    loCmd.AddField("field1", "integer", true, true);
    loCmd.AddField("field2", "varchar(20)");
    loCmd.AddField("field3", "text");
    loCmd.AddField("field4", "double");

    if (loCmd.Execute() < 0)
    {
        apl_errprintf("Create table fail, %s\n", loCmd.GetErrorMsg() );
    }
}

void TestDrop( adl::IDatabase* apoDb )
{
    CDBDropOperator loCmd(apoDb, "abc");

    if (loCmd.Execute() < 0)
    {
        apl_errprintf("Drop table fail, %s\n", loCmd.GetErrorMsg() );
    }
}

void TestInsert( adl::IDatabase* apoDb )
{
    acl::CMemoryBlock loBlob;
    acl::CFileStream  loFile;

    loFile.Open("DBOperator.h", APL_O_RDONLY);

    loBlob.Resize(loFile.GetFileSize() );

    loFile.Read(loBlob.GetWritePtr(), loBlob.GetSize() );
    
    loBlob.SetWritePtr(loBlob.GetSize() );

    START_LOOP(LOOP_COUNT)
    {
        CDBInsertOperator loCmd(apoDb, "abc");

        loCmd.AddField("field1", (apl_intmax_t)__liN);
        loCmd.AddField("field2", "value 1");
        loCmd.AddField("field3",  loBlob.GetReadPtr(), loBlob.GetLength() );
        loCmd.AddField("field4", __liN + 0.1);
        
        if (loCmd.Execute() <= 0)
        {
            apl_errprintf("Insert table fail, %s\n", loCmd.GetErrorMsg() );
        }
    }
    END_LOOP();
}

void TestSelect( adl::IDatabase* apoDb )
{
    START_LOOP(LOOP_COUNT)
    {
        CDBSelectOperator loCmd(apoDb, "abc");

        loCmd.AddField("field2");
        loCmd.AddField("field3");
        loCmd.AddField("field4");

        loCmd.AddCondition("field1", (apl_intmax_t)__liN );

        apl_ssize_t liResult = loCmd.Execute();
        if (liResult <= 0)
        {
            apl_errprintf("Select fail, %s\n", loCmd.GetErrorMsg() );
        }

        //for (apl_ssize_t liN = 0; liN < liResult; liN++)
        //{
        //    printf("%s|%s|%s\n", 
        //        loCmd.GetField(liN,0).c_str(),
        //        loCmd.GetField(liN,2).c_str(),
        //        loCmd.GetField(liN,2).c_str() );
        //}
    }
    END_LOOP();
}

void TestSelect1( adl::IDatabase* apoDb )
{
    CDBSelectOperator loCmd(apoDb, "abc");

    loCmd.AddField("field2");
    loCmd.AddField("field3");
    loCmd.AddField("field4");

    loCmd.SetCondition( CDBCondition("field1", (apl_intmax_t)1) && CDBCondition("field2", "value 1") );

    apl_ssize_t liResult = loCmd.Execute();
    if (liResult <= 0)
    {
        apl_errprintf("Select fail, %s\n", loCmd.GetErrorMsg() );
    }

    printf("%s|%s|%s\n", 
        loCmd.GetField(0,0).c_str(),
        loCmd.GetField(0,2).c_str(),
        loCmd.GetField(0,2).c_str() );
}

void TestUpdate( adl::IDatabase* apoDb )
{
    acl::CMemoryBlock loBlob;
    acl::CFileStream  loFile;

    loFile.Open("DBOperator.h", APL_O_RDONLY);

    loBlob.Resize(loFile.GetFileSize() );

    loFile.Read(loBlob.GetWritePtr(), loBlob.GetSize() );
    
    loBlob.SetWritePtr(loBlob.GetSize() );

    START_LOOP(LOOP_COUNT)
    {
        CDBUpdateOperator loCmd(apoDb, "abc");

        loCmd.AddField("field2", "value 1");
        loCmd.AddField("field3",  loBlob.GetReadPtr(), loBlob.GetLength() );
        
        loCmd.AddCondition("field1", (apl_intmax_t)__liN);

        if (loCmd.Execute() <= 0)
        {
            apl_errprintf("Update table fail, %s\n", loCmd.GetErrorMsg() );
        }
    }
    END_LOOP();
}

void TestDelete( adl::IDatabase* apoDb )
{
    START_LOOP(LOOP_COUNT)
    {
        CDBDeletOperator loCmd(apoDb, "abc");

        loCmd.AddCondition("field1", (apl_intmax_t)__liN);

        if (loCmd.Execute() <= 0)
        {
            apl_errprintf("Delete fail, %s\n", loCmd.GetErrorMsg() );
        }
    }
    END_LOOP();
}

int main(int argc, char* argv[] )
{
    adl::CSQLiteDatabase loSQLite("sqlite.db");

    loSQLite.PragmaCommand("synchronous=OFF");
    loSQLite.PragmaCommand("journal_mode=PERSIST");

    if (loSQLite.Init() != 0)
    {
        apl_errprintf("Init sqlite fail, %s\n", loSQLite.GetErrorMsg() );
        return -1;
    }

    TestDrop(&loSQLite);
    TestCreate(&loSQLite);
    TestInsert(&loSQLite);
    TestSelect1(&loSQLite);
    TestSelect(&loSQLite);
    TestUpdate(&loSQLite);
    TestDelete(&loSQLite);

    loSQLite.Destroy();
}

#endif
