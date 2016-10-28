
#include "acl/SString.h"
#include "acl/stl/map.h"
#include "adl/SQLiteDatabase.h"
#include "SQLiteConnection.h"
#include "sqlite3.h"

ADL_NAMESPACE_START

class CSQLiteDatabaseImpl
{
public:
    CSQLiteDatabaseImpl( char const* apcName, apl_int_t aiFlags );

    ~CSQLiteDatabaseImpl(void);

    void PragmaCommand( char const* apcCmd );

	apl_int_t Init(void);

	void Destroy(void);

    IConnection* GetConnection(
        acl::CTimeValue const& aoTimeout = acl::CTimeValue::MAXTIME);
    
    void ReleaseConnection(IConnection* apoConn);

    apl_int_t ExecuteNonQuery( char const* apcSQL, apl_int64_t* api64RowID );
    
    const char* GetErrorMsg(void);

    apl_int_t GetErrorCode(void);

    apl_int_t GetThreadSafety(void);

private:
    std::string moName;

    apl_int_t miFlags;

    std::vector<std::string> moPragmaCommands;

    std::string moErrorMessage;

    apl_int_t miLastErrcode;
    
    CSQLiteConnection* mpoConn;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
CSQLiteDatabaseImpl::CSQLiteDatabaseImpl( char const* apcName, apl_int_t aiFlags )
    : moName(apcName)
    , miFlags(0)
    , miLastErrcode(0)
    , mpoConn(APL_NULL)
{
    if (ACL_BIT_ENABLED(aiFlags, CSQLiteDatabase::OPEN_READONLY) )
    {
        ACL_SET_BITS(this->miFlags, SQLITE_OPEN_READONLY);
    }
    else if (ACL_BIT_ENABLED(aiFlags, CSQLiteDatabase::OPEN_READWRITE) )
    {
        ACL_SET_BITS(this->miFlags, SQLITE_OPEN_READWRITE);
    }

    if (ACL_BIT_ENABLED(aiFlags, CSQLiteDatabase::OPEN_CREATE) )
    {
        ACL_SET_BITS(this->miFlags, SQLITE_OPEN_CREATE);
    }
    
    if (ACL_BIT_ENABLED(aiFlags, CSQLiteDatabase::OPEN_NOMUTEX) )
    {
        ACL_SET_BITS(this->miFlags, SQLITE_OPEN_NOMUTEX);
    }
    else if (ACL_BIT_ENABLED(aiFlags, CSQLiteDatabase::OPEN_FULLMUTEX) )
    {
        ACL_SET_BITS(this->miFlags, SQLITE_OPEN_FULLMUTEX);
    }
}

CSQLiteDatabaseImpl::~CSQLiteDatabaseImpl(void)
{
    this->Destroy();
}

void CSQLiteDatabaseImpl::PragmaCommand( char const* apcCmd )
{
    this->moPragmaCommands.push_back(apcCmd);
}
	
apl_int_t CSQLiteDatabaseImpl::Init(void)
{
    apl_int_t liRetCode = 0;

    ACL_NEW_ASSERT(this->mpoConn, CSQLiteConnection);

    //Init only one connection for connection pool
    if ( (liRetCode = this->mpoConn->Open(this->moName.c_str(), this->miFlags) ) == 0)
    {
        //PRAGMA statement on new connection
        for (std::vector<std::string>::iterator loIter = this->moPragmaCommands.begin();
            loIter != this->moPragmaCommands.end(); ++loIter)
        {
            if ( (liRetCode = this->mpoConn->PragmaCommand(loIter->c_str() ) ) != 0)
            {
                break;
            }
        }
    }

    if (liRetCode != 0)
    {
        this->moErrorMessage = this->mpoConn->GetErrorMsg();
        this->miLastErrcode = this->mpoConn->GetErrorCode();

        ACL_DELETE(this->mpoConn);
        
        return -1;
    }
    else
    {
        return 0;
    }
}

void CSQLiteDatabaseImpl::Destroy(void)
{
    if (this->mpoConn != APL_NULL)
    {
        this->mpoConn->Close();

        ACL_DELETE(this->mpoConn);
    }
}

IConnection* CSQLiteDatabaseImpl::GetConnection( acl::CTimeValue const& aoTimeout )
{
    return this->mpoConn;
}

void CSQLiteDatabaseImpl::ReleaseConnection( IConnection* apoConn )
{
    //Do nothing
}

apl_int_t CSQLiteDatabaseImpl::ExecuteNonQuery( char const* apcSQL, apl_int64_t* api64RowID )
{
    if (this->mpoConn->ExecuteNonQuery(apcSQL, api64RowID) != 0)
    {
        this->moErrorMessage = this->mpoConn->GetErrorMsg();
        this->miLastErrcode = this->mpoConn->GetErrorCode();

        return -1;
    }
    else
    {
        return 0;
    }
}
    
const char* CSQLiteDatabaseImpl::GetErrorMsg(void)
{
    return this->moErrorMessage.c_str();
}

apl_int_t CSQLiteDatabaseImpl::GetErrorCode(void)
{
    return this->miLastErrcode;
}

apl_int_t CSQLiteDatabaseImpl::GetThreadSafety(void)
{
    apl_int_t liResult = sqlite3_threadsafe();

    switch(liResult)
    {
        case 0:
            //With -DSQLITE_THREADSAFE=0 the threading mode is single-thread
            return THREAD_SAFETY_NO;
        case 1:
            //With -DSQLITE_THREADSAFE=1 the threading mode is Serialized
            return THREAD_SAFETY_CONNECTION;
        case 2:
            //With -DSQLITE_THREADSAFE=1 the threading mode is Multi-thread
            return THREAD_SAFETY_MODULE;
        default:
            return THREAD_SAFETY_NO;
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CSQLiteDatabase::CSQLiteDatabase( char const* apcName, apl_int_t aiFlags )
    : mpoImpl(APL_NULL)
{
    ACL_NEW_ASSERT(this->mpoImpl, CSQLiteDatabaseImpl(apcName, aiFlags) );
}

CSQLiteDatabase::~CSQLiteDatabase(void)
{
    ACL_DELETE(this->mpoImpl);
}

void CSQLiteDatabase::PragmaCommand( char const* apcCmd )
{
    this->mpoImpl->PragmaCommand(apcCmd);
}
	
apl_int_t CSQLiteDatabase::Init(void)
{
    return this->mpoImpl->Init();
}

void CSQLiteDatabase::Destroy(void)
{
    return this->mpoImpl->Destroy();
}

IConnection* CSQLiteDatabase::GetConnection( acl::CTimeValue const& aoTimeout )
{
    return this->mpoImpl->GetConnection(aoTimeout);
}

void CSQLiteDatabase::ReleaseConnection( IConnection* apoConn )
{
    this->mpoImpl->ReleaseConnection(apoConn);
}
    
apl_int_t CSQLiteDatabase::ExecuteNonQuery( char const* apcSQL, apl_int64_t* api64RowID )
{
    return this->mpoImpl->ExecuteNonQuery(apcSQL, api64RowID);
}

const char* CSQLiteDatabase::GetErrorMsg(void)
{
    return this->mpoImpl->GetErrorMsg();
}

apl_int_t CSQLiteDatabase::GetErrorCode(void)
{
    return this->mpoImpl->GetErrorCode();
}

apl_int_t CSQLiteDatabase::GetThreadSafety(void)
{
    return this->mpoImpl->GetThreadSafety();
}

////////////////////////////////////////////////////////////////////////////////////////
//Define for dynamic load database type
extern "C"
{
    IDatabase* CreateDatabase0( std::map<std::string, std::string>& aoParameteres );
}

IDatabase* CreateDatabase0( std::map<std::string, std::string>& aoParameteres )
{
    CSQLiteDatabase* lpoDatabase = APL_NULL;
    std::string loName;
    apl_int_t   liFlags = 0;

    //Get database name
    std::map<std::string, std::string>::iterator loIter = aoParameteres.find("name");
    if (loIter == aoParameteres.end() )
    {
        apl_errprintf("Create database fail,unknown db name, example:name=myname\n");

        return APL_NULL;
    }

    loName = loIter->second;

    //Get database flags
    if ( (loIter = aoParameteres.find("flag") ) != aoParameteres.end() )
    {
        acl::CSpliter loSpliter("|");

        loSpliter.Parse(loIter->second.c_str() );

        for (apl_size_t luN = 0; luN < loSpliter.GetSize(); luN++)
        {
            if (apl_strcasecmp(loSpliter.GetField(luN), "readonly") == 0)
            {
                ACL_SET_BITS(liFlags, CSQLiteDatabase::OPEN_READONLY);
            }
            else if (apl_strcasecmp(loSpliter.GetField(luN), "readwrite") == 0)
            {
                ACL_SET_BITS(liFlags, CSQLiteDatabase::OPEN_READWRITE);
            }
            else if (apl_strcasecmp(loSpliter.GetField(luN), "create") == 0)
            {
                ACL_SET_BITS(liFlags, CSQLiteDatabase::OPEN_CREATE);
            }
            else if (apl_strcasecmp(loSpliter.GetField(luN), "nomutex") == 0)
            {
                ACL_SET_BITS(liFlags, CSQLiteDatabase::OPEN_NOMUTEX);
            }
            else if (apl_strcasecmp(loSpliter.GetField(luN), "fullmutex") == 0)
            {
                ACL_SET_BITS(liFlags, CSQLiteDatabase::OPEN_FULLMUTEX);
            }
        }
    }

    ACL_NEW_ASSERT(lpoDatabase, CSQLiteDatabase(loName.c_str(), liFlags) );
    
    //Get database pragma command
    if ( (loIter = aoParameteres.find("pragma") ) != aoParameteres.end() )
    {
        acl::CSpliter loSpliter("|");

        loSpliter.Parse(loIter->second.c_str() );

        for (apl_size_t luN = 0; luN < loSpliter.GetSize(); luN++)
        {
            lpoDatabase->PragmaCommand(loSpliter.GetField(luN) );
        }
    }
    
    return lpoDatabase;
}

ADL_NAMESPACE_END

