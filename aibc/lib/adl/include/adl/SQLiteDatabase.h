#ifndef ADL_SQLITEI_DATABASE_H
#define ADL_SQLITEI_DATABASE_H

#include "adl/Adl.h"
#include "acl/Timestamp.h"
#include "acl/stl/vector.h"
#include "acl/stl/string.h"

ADL_NAMESPACE_START

class CSQLiteDatabase : public IDatabase
{
public:
    enum
    {
        //The database is opened in read-only mode.
        //If the database does not already exist, an error is returned.
        OPEN_READONLY     = 0x01,

        //The database is opened for reading and writing if possible, 
        //or reading only if the file is write protected by the operating system. 
        //In either case the database must already exist, otherwise an error is returned.
        OPEN_READWRITE    = 0x02,

        //The database is opened and is creates it if it does not already exist.
        OPEN_CREATE       = 0x04,

        //The database connection opens in the multi-thread threading mode 
        //as long as the single-thread mode has not been set at compile-time or start-time.
        //In this mode, SQLite can be safely used by multiple threads provided 
        //that no single database connection is used simultaneously in two or more threads.
        OPEN_NOMUTEX      = 0x08,

        //The database connection opens in the serialized threading mode 
        //unless single-thread was previously selected at compile-time or start-time.
        //In serialized mode, SQLite can be safely used by multiple threads with no restriction.
        OPEN_FULLMUTEX    = 0x10,
    };

public:
    CSQLiteDatabase( char const* apcName, apl_int_t aiFlags = OPEN_READWRITE|OPEN_CREATE|OPEN_FULLMUTEX );

    ~CSQLiteDatabase(void);

    /** 
     * The PRAGMA statement is a SQL extension specific to SQLite 
     * and used to modify the operation of the SQLite library or to query the SQLite library for internal (non-table) data. 
     * 
     * @param apcCmd PRAGMA command
     */
    void PragmaCommand( char const* apcCmd );

    /** 
     * Initialize the SQLite Database.
     *
     * This function shall try to init the SQLite Database ENV.
     * It *MUST* be callled before all the other operations.
     *
     * @retval 0     Initialize success.
     * @retval !=0   Initialize failed.
     *
     */
	apl_int_t Init(void);

    /** 
     * Destroy the SQLite Database.
     *
     * Usually, there is no need to call this routine because the 
     * de-constructer of the xxxBase class would have the same effect. It's 
     * provided for caller who want to re-init with Init().
     *
     * This function shall destroy the SQLite Database.
     *
     */
	void Destroy(void);

    /** 
     * Get a SQLite Database connection.
     *
     * This function shall get a SQLite Database connection handler and
     * All the later operations are based on this handler. This might be a 
     * virtual connection, but the caller should not care about it.
     *
     * @param[in]   aoTimeout     The MAX wait time untill getting a valid 
     *                            connection handler. Default wait forever.
     *
     * @retval APL_NULL     Can't get a valid handler in the specified time.
     * @retval !=APL_NULL   Pointer to the connection handler.
     *
     */
    IConnection* GetConnection(
        acl::CTimeValue const& aoTimeout = acl::CTimeValue::MAXTIME);

    /** 
     * Release a SQLite Database connection.
     *
     * This function shall release a SQLite Database connection handler. 
     * All the later operations on a released handler is undefined. 
     *
     * @param[in]   apoIConn      The connection which get by GetConnection() 
     *                            before.
     *
     */
    void ReleaseConnection(IConnection* apoConn);
    
    /** 
     * @brief To execute SQL statement
     *
     * This function allows an application to run multiple statements of SQL and return last insert rowid
     * 
     * @param apcSQL SQL statement
     * @param api64RowID Last insert RowID
     * 
     * @returns if success return 0, otherwise -1 shall be return
     */
    apl_int_t ExecuteNonQuery( char const* apcSQL, apl_int64_t* api64RowID = APL_NULL );

    /** 
     * Get the error message of the last DB operation.
     *
     * This function shall try to get the error message of the last
     * Database operation.
     *
     * @retval          Error message in string format.
     *
     */
    const char* GetErrorMsg(void);

    /** 
     * Get the error code of the last DB operation.
     *
     * This function shall try to get the error code of the last
     * Database operation.
     *
     * @retval          Error code in apl_int_t format.
     *
     */
    apl_int_t GetErrorCode(void);

    /** 
     * Get the thread_safety level of the module.
     *
     * This function shall return the thread-safety level of the module.
     *
     * @retval THREAD_SAFETY_NO          Threads may not share the module.
     * @retval THREAD_SAFETY_MODULE      Threads may share the module, but not 
     *                                   connections.
     * @retval THREAD_SAFETY_CONNECTION  Threads may share the module and 
     *                                   connections.
     * @retval THREAD_SAFETY_CURSOR      Threads may share the module, 
     *                                   connections and cursors.
     *
     */
    apl_int_t GetThreadSafety(void);

private:
    class CSQLiteDatabaseImpl* mpoImpl;
};


/** 
 * @brief This abstract class define some method specially for SQLite
 */
class ISQLiteCursor : public ICursor
{
public:
    virtual ~ISQLiteCursor(void) {}

    /** 
     * Get the last insert row id of the last INSERT statment execution.
     *
     * This function shall try to get the last insert row id of the last
     * DataBase execution which issued by Execute(...). e.g. 
     * INSERT 
     *
     * @retval The last insert row id by last execution.
     *
     */
    virtual apl_int64_t GetLastInsertRowID(void) = 0;
};

ADL_NAMESPACE_END

#endif//ADL_SQLITE_DATABASE_H

