#ifndef ADL_SQLITE_CONNECTION_H
#define ADL_SQLITE_CONNECTION_H

#include "adl/Adl.h"
#include "sqlite3.h"

ADL_NAMESPACE_START

class CSQLiteConnection : public IConnection
{
public:
    CSQLiteConnection(void);

    virtual ~CSQLiteConnection(void);
    
    /** 
     * @brief Open SQLite database connection
     *
     * This function shall try to open sqlite database by specified flags
     * 
     * @param apcName sqlite database name
     * @param aiFlags open flags
     * 
     * @returns if success return 0, otherwise -1 shall be return and GetErrorMsg()/GetErroCode() to indicate the error
     */
    apl_int_t Open( char const* apcName, apl_int_t aiFlags );

    /** 
     * @brief Close SQLite database and release all resource, after this call all behavior on Connection is undefined
     */
    void Close(void);

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
     * @brief To execute PRAGMA statement
     * 
     * The PRAGMA statement is a SQL extension specific to SQLite 
     * and used to modify the operation of the SQLite library 
     * or to query the SQLite library for internal (non-table) data
     * 
     * @param apcName PRAGMA command name
     * 
     * @returns if success return 0, otherwise -1 shall be return and GetErrorMsg()/GetErrorCode() to indicate the error 
     */
    apl_int_t PragmaCommand( char const* apcCmd );

    /** 
     * Get a cursor handler from connection.
     *
     * This function shall try to get a cursor from connection. All the 
     * Database operation/query is based on cursor. 
     * Multi-cursors could get from the same connection, but the 
     * cursors are not thread-safety in all database. To see 
     * thread-safety level of a database, use GetThreadSafety() to get 
     * the level.
     *
     * @retval APL_NULL     Can't get a valid cursor handler.
     * @retval !=APL_NULL   Pointer to the cursor handler.
     *
     */
	virtual ICursor* Cursor(void);

    /** 
     * Begin a transaction.
     *
     * This function shall try to begin a transaction on a connection.
     * Node that, transaction is based on *connection* in ADL (Not
     * based on cursor), the caller should keep in mind that when 
     * multi-cursors sharing the same connection.
     *
     * @retval 0            Begin transaction success.
     * @retval !=0          Begin transaction failed.
     *
     */
    virtual apl_int_t BeginTrans(void);

    /** 
     * End a transaction and commit.
     *
     * This function shall try to end a transaction on a connection 
     * and commit. All the operations since the call of BeginTrans() 
     * would commit. 
     *
     * @retval 0            Commit and end transaction success.
     * @retval !=0          Commit failed.
     *
     */
    virtual apl_int_t Commit(void);

    /** 
     * End a transaction and rollback.
     *
     * This function shall try to end a transaction on a connection 
     * and rollback. All the operations since the call of BeginTrans() 
     * would cancle.
     *
     * @retval 0            Rollback and end transaction success.
     * @retval !=0          Rollback failed.
     *
     */
    virtual apl_int_t Rollback(void);

    /** 
     * Set characterset for current cursor.
     * 
     * It would affect the other cursors which get from the same connection.
     *
     * FIXME, currently this is only available for mysql. Because oracle's 
     * characterset behavior could be changed by env variable NLS_LANG.
     *
     * Calling this function is equal to the following 3 statements in mysql:
     *      SET character_set_client = apcCharacterSet;
     *      SET character_set_results = apoCharacterSet;
     *      SET character_set_connection = apoCharacterSet;
     *
     * @param[in]   apoCharacterSet The character set which want to set.
     *
     * @retval =0           Set success.
     * @retval !=0          Set failed.
     *
     */
    virtual apl_int_t SetCharacters(const char* apoCharacterSet);

    /** 
     * Get the error message of the last DB operation.
     *
     * This function shall try to get the error message of the last
     * DataBase operation.
     *
     * @retval          Error message in string format.
     *
     */
    virtual const char* GetErrorMsg(void);

    /** 
     * Get the error code of the last DB operation.
     *
     * This function shall try to get the error code of the last
     * DataBase operation. 
     *
     * @retval          Error code in apl_int_t format.
     *
     */
    virtual apl_int_t GetErrorCode(void);

    sqlite3* GetDB(void);

private:
    sqlite3* mpoDB;
    
    char macLastMessage[256];

    apl_int_t miLastErrcode;
};

ADL_NAMESPACE_END

#endif//ADL_SQLITE_CONNECTION_H

