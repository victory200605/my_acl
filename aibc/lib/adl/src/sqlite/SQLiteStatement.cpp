
#include "SQLiteStatement.h"

ADL_NAMESPACE_START

#define CXX_SQLITE_CLEAR_ERROR() \
    this->macLastMessage[0] = '\0'; \
    this->miLastErrcode = 0;
    
#define CXX_SQLITE_SET_LAST_MESSAGE(retcode) \
    apl_strncpy(this->macLastMessage, sqlite3_errmsg(this->moConn.GetDB() ), sizeof(this->macLastMessage) ); \
    if (retcode == -1) \
    { \
        this->miLastErrcode = sqlite3_errcode(this->moConn.GetDB() ); \
    } \
    else \
    { \
        this->miLastErrcode = retcode; \
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSQLiteGuard
{
public:
    CSQLiteGuard( CSQLiteConnection& aoConn )
        : moConn(aoConn)
    {
        sqlite3_mutex_enter(sqlite3_db_mutex(this->moConn.GetDB() ) );
    }

    ~CSQLiteGuard(void)
    {
        sqlite3_mutex_leave(sqlite3_db_mutex(this->moConn.GetDB() ) );
    }

private:
    CSQLiteConnection& moConn;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//Parameter binding
struct CParameterInt16 : public CSQLiteStatement::CParameter
{
    CParameterInt16( apl_int_t aiIndex, apl_int16_t* apiValue )
        : mpiValue(apiValue)
    {
        this->miIndex = aiIndex;
    }

    apl_int_t Bind( sqlite3_stmt* apoStmt )
    {
        apl_int_t liRetCode = sqlite3_bind_int(apoStmt, this->miIndex, *this->mpiValue);
        return liRetCode == SQLITE_OK ? 0 : -1;
    }

    apl_int16_t* mpiValue;
};

struct CParameterInt32 : public CSQLiteStatement::CParameter
{
    CParameterInt32( apl_int_t aiIndex, apl_int32_t* apiValue )
        : mpiValue(apiValue)
    {
        this->miIndex = aiIndex;
    }

    apl_int_t Bind( sqlite3_stmt* apoStmt )
    {
        apl_int_t liRetCode = sqlite3_bind_int(apoStmt, this->miIndex, *this->mpiValue);
        return liRetCode == SQLITE_OK ? 0 : -1;
    }

    apl_int32_t* mpiValue;
};

struct CParameterInt64 : public CSQLiteStatement::CParameter
{
    CParameterInt64( apl_int_t aiIndex, apl_int64_t* apiValue )
        : mpiValue(apiValue)
    {
        this->miIndex = aiIndex;
    }

    apl_int_t Bind( sqlite3_stmt* apoStmt )
    {
        apl_int_t liRetCode = sqlite3_bind_int64(apoStmt, this->miIndex, *this->mpiValue);
        return liRetCode == SQLITE_OK ? 0 : -1;
    }

    apl_int64_t* mpiValue;
};

struct CParameterUint32 : public CSQLiteStatement::CParameter
{
    CParameterUint32( apl_int_t aiIndex, apl_uint32_t* apiValue )
        : mpiValue(apiValue)
    {
        this->miIndex = aiIndex;
    }

    apl_int_t Bind( sqlite3_stmt* apoStmt )
    {
        apl_int_t liRetCode = sqlite3_bind_int64(apoStmt, this->miIndex, *this->mpiValue);
        return liRetCode == SQLITE_OK ? 0 : -1;
    }

    apl_uint32_t* mpiValue;
};

struct CParameterStr : public CSQLiteStatement::CParameter
{
    CParameterStr( apl_int_t aiIndex, char const* apcValue )
        : mpcValue(apcValue)
    {
        this->miIndex = aiIndex;
    }

    apl_int_t Bind( sqlite3_stmt* apoStmt )
    {
        apl_int_t liRetCode = sqlite3_bind_text(
                apoStmt, this->miIndex, this->mpcValue, apl_strlen(this->mpcValue), SQLITE_STATIC);
        return liRetCode == SQLITE_OK ? 0 : -1;
    }

    char const* mpcValue;
};

struct CParameterBlob : public CSQLiteStatement::CParameter
{
    CParameterBlob( apl_int_t aiIndex, void const* apvValue, apl_size_t auLen )
        : mpvValue(apvValue)
        , muLength(auLen)
    {
        this->miIndex = aiIndex;
    }

    apl_int_t Bind( sqlite3_stmt* apoStmt )
    {
        apl_int_t liRetCode = sqlite3_bind_blob(
            apoStmt, this->miIndex, this->mpvValue, this->muLength, SQLITE_STATIC);
        return liRetCode == SQLITE_OK ? 0 : -1;
    }

    void const* mpvValue;
    apl_size_t  muLength;
};

struct CParameterFloat : public CSQLiteStatement::CParameter
{
    CParameterFloat( apl_int_t aiIndex, float* apfValue )
        : mpfValue(apfValue)
    {
        this->miIndex = aiIndex;
    }

    apl_int_t Bind( sqlite3_stmt* apoStmt )
    {
        apl_int_t liRetCode = sqlite3_bind_double(apoStmt, this->miIndex, *this->mpfValue);
        return liRetCode == SQLITE_OK ? 0 : -1;
    }

    float* mpfValue;
};

struct CParameterDouble : public CSQLiteStatement::CParameter
{
    CParameterDouble( apl_int_t aiIndex, double* apdValue )
        : mpdValue(apdValue)
    {
        this->miIndex = aiIndex;
    }

    apl_int_t Bind( sqlite3_stmt* apoStmt )
    {
        apl_int_t liRetCode = sqlite3_bind_double(apoStmt, this->miIndex, *this->mpdValue);
        return liRetCode == SQLITE_OK ? 0 : -1;
    }

    double* mpdValue;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
CSQLiteStatement::CSQLiteStatement( CSQLiteConnection& aoConn )
    : moConn(aoConn)
    , mpoStmt(APL_NULL)
    , miPreFetchRows(0)
    , miAffectedRows(0)
    , mi64LastInsertRowID(-1)
{
    CXX_SQLITE_CLEAR_ERROR();
}

CSQLiteStatement::~CSQLiteStatement(void)
{
    this->Finalize();
}

void CSQLiteStatement::Close(void)
{
    delete this;
}

apl_int_t CSQLiteStatement::Prepare(const char* apcSQL)
{
    CXX_SQLITE_CLEAR_ERROR();

    apl_int_t liRetCode = 0;

    //Thread obtain exclusive use of the db connection
    CSQLiteGuard loGuard(this->moConn);
    
    //Release last statement resource
    this->Finalize();

    liRetCode = sqlite3_prepare_v2(this->moConn.GetDB(), apcSQL, apl_strlen(apcSQL), &this->mpoStmt, APL_NULL);
    if (liRetCode != SQLITE_OK)
    {
        CXX_SQLITE_SET_LAST_MESSAGE(liRetCode);

        return -1;
    }
    else
    {
        return 0;
    }
}

apl_int_t CSQLiteStatement::Execute(void)
{
    CXX_SQLITE_CLEAR_ERROR();
    
    //Thread obtain exclusive use of the db connection
    CSQLiteGuard loGuard(this->moConn);

    this->miPreFetchRows = 0;
    this->miAffectedRows = 0;

    sqlite3_clear_bindings(this->mpoStmt);
    sqlite3_reset(this->mpoStmt);

    //Bind or re-bind all parameter first
    if (this->BindAllParameter() != 0)
    {
        CXX_SQLITE_SET_LAST_MESSAGE(-1);

        return -1;
    }
    
    return this->Step();
}

apl_int_t CSQLiteStatement::Step(void)
{
    apl_int_t liRetCode = 0;

    liRetCode = sqlite3_step(this->mpoStmt);
    if (liRetCode == SQLITE_ROW)
    {
        this->miPreFetchRows = 1;

        return 0;
    }
    else if (liRetCode == SQLITE_DONE)
    {
        this->miPreFetchRows = -1;

        this->miAffectedRows = sqlite3_changes(this->moConn.GetDB() );

        this->mi64LastInsertRowID = sqlite3_last_insert_rowid(this->moConn.GetDB() );
        
        return 0;
    }
    else
    {
        CXX_SQLITE_SET_LAST_MESSAGE(liRetCode);

        return -1;
    }
}

apl_int_t CSQLiteStatement::Execute(const char* apcSQL)
{
    CXX_SQLITE_CLEAR_ERROR();

    apl_int_t liRetCode = 0;

    //Thread obtain exclusive use of the db connection
    CSQLiteGuard loGuard(this->moConn);
    
    this->Finalize();

    this->miPreFetchRows = 0;
    this->miAffectedRows = 0;

    liRetCode = sqlite3_prepare_v2(this->moConn.GetDB(), apcSQL, apl_strlen(apcSQL), &this->mpoStmt, APL_NULL);
    if (liRetCode != SQLITE_OK)
    {
        CXX_SQLITE_SET_LAST_MESSAGE(liRetCode);

        return -1;
    }
    
    return this->Step();
}

apl_int_t CSQLiteStatement::Finalize(void)
{
    CXX_SQLITE_CLEAR_ERROR();
    
    apl_int_t liRetCode = 0;
    
    if (this->mpoStmt != APL_NULL)
    {
        if ( (liRetCode = sqlite3_finalize(this->mpoStmt) ) != SQLITE_OK)
        {
            CXX_SQLITE_SET_LAST_MESSAGE(liRetCode);
        }
    }
    
    //Clear all binded parameteres
    apl_size_t luCount = this->moParameteres.size();

    if (luCount > 0)
    {
        for (apl_size_t luN = 0; luN < luCount; luN++)
        {
            ACL_DELETE(this->moParameteres[luN]);
        }

        this->moParameteres.clear();
    }

    return liRetCode;
}

const char* CSQLiteStatement::GetField(apl_size_t auField, apl_int_t* apiLen)
{
    CXX_SQLITE_CLEAR_ERROR();

    if (apiLen != APL_NULL)
    {
        *apiLen = sqlite3_column_bytes(this->mpoStmt, auField);
    }
    
    return (char const*)sqlite3_column_text(this->mpoStmt, auField);
}

const char* CSQLiteStatement::GetField(const char* apcFieldName, apl_int_t* apiLen)
{
    CXX_SQLITE_CLEAR_ERROR();

    apl_int_t liIndex = this->GetFieldIndex(apcFieldName);

    if (apiLen != APL_NULL)
    {
        *apiLen = sqlite3_column_bytes(this->mpoStmt, liIndex);
    }
    
    return (char const*)sqlite3_column_text(this->mpoStmt, liIndex);
}

apl_size_t CSQLiteStatement::GetFieldNameCount(void)
{
    return sqlite3_column_count(this->mpoStmt);
}

const char* CSQLiteStatement::GetFieldName(apl_size_t auIdx)
{
    return sqlite3_column_name(this->mpoStmt, auIdx);
}

apl_int_t CSQLiteStatement::GetFieldIndex(const char* apcName)
{
    apl_size_t luCount = this->GetFieldNameCount();

    for (apl_size_t luN = 0; luN < luCount; luN++)
    {
        char const* lpcTemp = this->GetFieldName(luN);
        if (lpcTemp != APL_NULL && apl_strcmp(lpcTemp, apcName) == 0)
        {
            return luN;
        }
    }

    return -1;
}

apl_int_t CSQLiteStatement::GetRowCounts(void)
{
    this->miLastErrcode = -1;
    apl_strncpy(this->macLastMessage, "Not support.", sizeof(this->macLastMessage) );

    return -1;
}

apl_int_t CSQLiteStatement::GetAffectedRows(void)
{
    return this->miAffectedRows;
}

apl_int_t CSQLiteStatement::BindParam(const char* apcParamName, apl_int16_t* apiParam)
{
    CParameter* lpoParameter = APL_NULL;
    apl_int_t liIndex = this->GetParameterIndex(apcParamName);
    if (liIndex <= 0)
    {
        return -1;
    }

    ACL_NEW_ASSERT(lpoParameter, CParameterInt16(liIndex, apiParam) );
    this->moParameteres.push_back(lpoParameter);

    return 0;
}

apl_int_t CSQLiteStatement::BindParam(const char* apcParamName, apl_int32_t* apiParam)
{
    CParameter* lpoParameter = APL_NULL;
    apl_int_t liIndex = this->GetParameterIndex(apcParamName);
    if (liIndex <= 0)
    {
        return -1;
    }

    ACL_NEW_ASSERT(lpoParameter, CParameterInt32(liIndex, apiParam) );
    this->moParameteres.push_back(lpoParameter);

    return 0;
}

apl_int_t CSQLiteStatement::BindParam(const char* apcParamName, apl_int64_t* apiParam)
{
    CParameter* lpoParameter = APL_NULL;
    apl_int_t liIndex = this->GetParameterIndex(apcParamName);
    if (liIndex <= 0)
    {
        return -1;
    }

    ACL_NEW_ASSERT(lpoParameter, CParameterInt64(liIndex, apiParam) );
    this->moParameteres.push_back(lpoParameter);

    return 0;
}


apl_int_t CSQLiteStatement::BindParam(const char* apcParamName, apl_uint32_t* apuParam)
{
    CParameter* lpoParameter = APL_NULL;
    apl_int_t liIndex = this->GetParameterIndex(apcParamName);
    if (liIndex <= 0)
    {
        return -1;
    }

    ACL_NEW_ASSERT(lpoParameter, CParameterUint32(liIndex, apuParam) );
    this->moParameteres.push_back(lpoParameter);

    return 0;
}

apl_int_t CSQLiteStatement::BindParam(const char* apcParamName, const char* apcParam, apl_size_t auLen)
{
    CParameter* lpoParameter = APL_NULL;
    apl_int_t liIndex = this->GetParameterIndex(apcParamName);
    if (liIndex <= 0)
    {
        return -1;
    }

    ACL_NEW_ASSERT(lpoParameter, CParameterStr(liIndex, apcParam) );
    this->moParameteres.push_back(lpoParameter);

    return 0;
}

apl_int_t CSQLiteStatement::BindParam(const char* apcParamName, const void* apcParam, apl_size_t auLen, bool abIsBinary)
{
    CParameter* lpoParameter = APL_NULL;
    apl_int_t liIndex = this->GetParameterIndex(apcParamName);
    if (liIndex <= 0)
    {
        return -1;
    }

    if (abIsBinary)
    {
        ACL_NEW_ASSERT(lpoParameter, CParameterBlob(liIndex, apcParam, auLen) );
    }
    else
    {
        ACL_NEW_ASSERT(lpoParameter, CParameterStr(liIndex, (char const*)apcParam) );
    }

    this->moParameteres.push_back(lpoParameter);

    return 0;
}

apl_int_t CSQLiteStatement::BindParam(const char* apcParamName, float* apfParam)
{
    CParameter* lpoParameter = APL_NULL;
    apl_int_t liIndex = this->GetParameterIndex(apcParamName);
    if (liIndex <= 0)
    {
        return -1;
    }

    ACL_NEW_ASSERT(lpoParameter, CParameterFloat(liIndex, apfParam) );
    this->moParameteres.push_back(lpoParameter);

    return 0;
}

apl_int_t CSQLiteStatement::BindParam(const char* apcParamName, double* apdParam)
{
    CParameter* lpoParameter = APL_NULL;
    apl_int_t liIndex = this->GetParameterIndex(apcParamName);
    if (liIndex <= 0)
    {
        return -1;
    }

    ACL_NEW_ASSERT(lpoParameter, CParameterDouble(liIndex, apdParam) );
    this->moParameteres.push_back(lpoParameter);

    return 0;
}

apl_int_t CSQLiteStatement::GetParameterIndex( const char* apcParamName )
{
    char lacTemp[64];

    apl_snprintf(lacTemp, sizeof(lacTemp), ":%s", apcParamName);
    
    return sqlite3_bind_parameter_index(this->mpoStmt, lacTemp);
}

apl_int_t CSQLiteStatement::BindAllParameter(void)
{
    apl_size_t luCount = this->moParameteres.size();

    if (luCount > 0)
    {
        for (apl_size_t luN = 0; luN < luCount; luN++)
        {
            if (this->moParameteres[luN]->Bind(this->mpoStmt) != 0)
            {
                return -1;
            }
        }
    }

    return 0;
}

void CSQLiteStatement::SetPreFetchRows(apl_size_t auRows)
{
    //Ignore it
}

apl_int_t CSQLiteStatement::FetchNext(void)
{
    if (this->miPreFetchRows > 0)
    {
        this->miPreFetchRows = 0;
        return 0;
    }
    else if (this->miPreFetchRows < 0)
    {
        return -1;
    }
    else
    {
        CXX_SQLITE_CLEAR_ERROR();

        //Thread obtain exclusive use of the db connection
        CSQLiteGuard loGuard(this->moConn);
        
        apl_int_t liRetCode = sqlite3_step(this->mpoStmt);
        if (liRetCode == SQLITE_ROW)
        {
            return 0;
        }
        else if (liRetCode != SQLITE_DONE)
        {
            CXX_SQLITE_SET_LAST_MESSAGE(liRetCode);

            return -1;
        }
        else
        {
            this->miPreFetchRows = -1;
            
            return -1;
        }
    }
}

const char* CSQLiteStatement::GetErrorMsg(void)
{
    return this->macLastMessage;
}

apl_int_t CSQLiteStatement::GetErrorCode(void)
{
    return this->miLastErrcode;
}

apl_int_t CSQLiteStatement::SetCharacters(const char* apoCharacterSet)
{
    this->miLastErrcode = -1;
    apl_strncpy(this->macLastMessage, "Not support.", sizeof(this->macLastMessage) );

    //Ignore
    return -1;
}

apl_int64_t CSQLiteStatement::GetLastInsertRowID(void)
{
    return this->mi64LastInsertRowID;
}

ADL_NAMESPACE_END

