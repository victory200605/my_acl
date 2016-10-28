
#include "SQLiteStatement.h"
#include "SQLiteConnection.h"

ADL_NAMESPACE_START

CSQLiteConnection::CSQLiteConnection(void)
    : mpoDB(APL_NULL)
{
}

CSQLiteConnection::~CSQLiteConnection(void)
{
    //Release all resource
    this->Close();
}

apl_int_t CSQLiteConnection::Open( char const* apcName, apl_int_t aiFlags )
{
    apl_int_t liRetCode = 0;
    
    //Release all resource first
    this->Close();

    if (aiFlags == 0)
    {
        aiFlags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
    }

    liRetCode = sqlite3_open_v2(apcName, &this->mpoDB, aiFlags, APL_NULL);

    if (liRetCode != SQLITE_OK && this->mpoDB != APL_NULL)
    {
        apl_strncpy(this->macLastMessage, sqlite3_errmsg(this->mpoDB), sizeof(this->macLastMessage) );

        this->miLastErrcode = liRetCode;

        this->Close();
    }
    
    return liRetCode == SQLITE_OK ? 0 : -1;
}

void CSQLiteConnection::Close(void)
{
    if (this->mpoDB != APL_NULL)
    {
        //Close sqlite
        sqlite3_close(this->mpoDB);

        //Reset DB pointer
        this->mpoDB = APL_NULL;
    }
}

apl_int_t CSQLiteConnection::ExecuteNonQuery( char const* apcSQL, apl_int64_t* api64RowID )
{
    apl_int_t liRetCode = -1;
    char* lpcErrMsg = APL_NULL;

    if (api64RowID == APL_NULL)
    {
        if (sqlite3_exec(this->mpoDB, apcSQL, APL_NULL, APL_NULL, &lpcErrMsg) == SQLITE_OK)
        {
            liRetCode = 0;
        }
    }
    else
    {
        sqlite3_mutex_enter(sqlite3_db_mutex(this->mpoDB) );
        
        if (sqlite3_exec(this->mpoDB, apcSQL, APL_NULL, APL_NULL, &lpcErrMsg) == SQLITE_OK)
        {
            *api64RowID = sqlite3_last_insert_rowid(this->mpoDB);
            liRetCode = 0;
        }
        
        sqlite3_mutex_leave(sqlite3_db_mutex(this->mpoDB) );
    }
    
    if (lpcErrMsg != APL_NULL)
    {
        apl_strncpy(this->macLastMessage, lpcErrMsg, sizeof(this->macLastMessage) );
        this->miLastErrcode = liRetCode;
        
        sqlite3_free(lpcErrMsg);
    }

    return liRetCode;
}

apl_int_t CSQLiteConnection::PragmaCommand( char const* apcCmd )
{
    char lacTemp[1024];

    apl_snprintf(lacTemp, sizeof(lacTemp), "PRAGMA %s", apcCmd);

    return this->ExecuteNonQuery(lacTemp);
}

ICursor* CSQLiteConnection::Cursor(void)
{
    ICursor* lpoCursor = APL_NULL;

    ACL_NEW_ASSERT(lpoCursor, CSQLiteStatement(*this) );

    return lpoCursor;
}

apl_int_t CSQLiteConnection::BeginTrans(void)
{
    return this->ExecuteNonQuery("BEGIN TRANSACTION");
}

apl_int_t CSQLiteConnection::Commit(void)
{
    return this->ExecuteNonQuery("COMMIT TRANSACTION");
}

apl_int_t CSQLiteConnection::Rollback(void)
{
    return this->ExecuteNonQuery("ROLLBACK TRANSACTION");
}

apl_int_t CSQLiteConnection::SetCharacters(const char* apoCharacterSet)
{
    this->miLastErrcode = -1;
    apl_strncpy(this->macLastMessage, "Not support.", sizeof(this->macLastMessage) );

    //Ignore
    return -1;
}

const char* CSQLiteConnection::GetErrorMsg(void)
{
    return this->macLastMessage;
}

apl_int_t CSQLiteConnection::GetErrorCode(void)
{
    return this->miLastErrcode;
}

sqlite3* CSQLiteConnection::GetDB(void)
{
    return this->mpoDB;
}

ADL_NAMESPACE_END

