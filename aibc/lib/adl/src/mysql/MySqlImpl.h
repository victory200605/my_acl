///////////////////////////////////////////////////////////
//  MySqlImpl.h
//  Implementation class of MYSQL DB 
//  Created on:      19-10-2009 14:27:59
//  Original author: hezk
///////////////////////////////////////////////////////////

#if !defined(ADL_MYSQL_IMPL_H)
#define ADL_MYSQL_IMPL_H

#include "adl/Adl.h"
#include "MySqlParamBind.h"
#include "../ResultSet.h"

#include "acl/Timestamp.h"
#include "acl/Synch.h"
#include <vector>
#include <string>
#include "mysql/mysql.h"

ADL_NAMESPACE_START


//remember what we last execute
//apl_int_t miLastExecute;
#define LAST_EXECUTE_NOTHING    (0)
#define LAST_EXECUTE_COMMON     (1)
#define LAST_EXECUTE_STMT       (2)

#define DB_CONNECTION_OK  	 (0)

#define DB_CONNECTION_FAIL	(-1)

#define DB_MYSQL_MAX_CONNECTION  (64)


class CMySqlConnection;

class CMySqlCursor : public ICursor
{

public:

    friend class CMySqlConnection;

	CMySqlCursor();

	virtual ~CMySqlCursor();
    
    /** 
     * Close the cursor itself.
     *
     * This function shall close the cursor itself. The cursor should
     * not be used any more after this call.
     * 
     */
    virtual void Close();

    /** 
     * Execute a prepared SQL command (with parameters binding).
     *
     * This function shall try to execute the SQL command which 
     * prepared by Prepare() before (with parameters binding).
     *
     * @param[in]   abAutoFetch  When it is set to true, the 
     *                           result set would be fetched by ADL
     *                           automatically after execute. 
     *                           Otherwise, it would not.
     *
     * @retval 0            End transaction success.
     * @retval !=0          End transaction failed.
     *
     */
	virtual apl_int_t Execute();

    /** 
     * Execute a SQL command.
     *
     * This function shall try to execute the SQL command. 
     *
     * If the command returning result set, e.g. SELECT, this 
     * function would either fetch the result set or not based 
     * on the abAutoFetch flag. By default, the result set would 
     * be fetched when executing. If the caller specify false to
     * abAutoFetch, then the result set could be fetched by a later
     * call of Fetch*() manually.
     *
     * If the execute failed, GetErrorCode() and GetErrorMsg()
     * would return a error code and error message about the 
     * error.
     * 
     * @param[in]   apcSQL       The Sql to execute.
     * @param[in]   abAutoFetch  When it is set to true, the 
     *                           result set would be fetched by ADL
     *                           automatically after execute. 
     *                           Otherwise, it would not.
     *
     * @retval 0            Execute success.
     * @retval !=0          Execute failed.
     *
     */
	virtual apl_int_t Execute(const char* apcSQL);

    /** 
     * Get result of current row from the fetched result set by field index.
     *
     * This function shall try to get result of the auField-th field of 
     * current row from the fetched result set which fetched by the last 
     * execution of Execute(apcSQL, abAutoFetch). Note that, the result 
     * set would only be fetched when abAutoFetch is set to true.
     *
     * Optionaly, if apiLen is not NULL, the length of the field's content
     * in bytes would be return by this pointer. Though usually the result
     * string would terminate with '\0'.
     *
     * Currently, only string fromat is supported for the result returning,
     * and the result is read-only.
     * 
     * @param[in]   auField      Specify the index of field in current row.
     * @param[out]  apiLen       The length in bytes of the result.
     *
     * @retval APL_NULL     Get field success.
     * @retval !=APL_NULL   Get field failed.
     *
     */
	virtual const char* GetField(apl_size_t auField, apl_int_t* apiLen);

    /** 
     * Get result of current row from the fetched result set by field name.
     *
     * This function shall try to get result of apcFieldName field of 
     * current row from the fetched result set which fetched by the last 
     * execution of Execute(apcSQL, abAutoFetch). Note that, the result 
     * set would only be fetched when abAutoFetch is set to true.
     *
     * Optionaly, if apiLen is not NULL, the length of the field's content
     * in bytes would be return by this pointer. Though usually the result
     * string would terminate with '\0'.
     *
     * Currently, only string fromat is supported for the result returning,
     * and the result is read-only.
     * 
     * @param[in]   apcFieldName    Specify the name of field in current row.
     * @param[out]  apiLen          The length in bytes of the result.
     *
     * @retval APL_NULL     Get field success.
     * @retval !=APL_NULL   Get field failed.
     *
     */
	virtual const char* GetField(const char* apcFieldName, apl_int_t* apiLen);

    /** 
     * Get the field counts of result set.
     *
     * This function shall try to get the field counts from the fetched 
     * result set which fetched by the last execution of 
     * Execute(apcSQL, abAutoFetch). Note that, the result set would only 
     * be fetched when abAutoFetch is set to true.
     *
     * @retval          The counts of the field in result set.
     *
     */
	virtual apl_size_t GetFieldNameCount();

    /** 
     * Get the field name by specify the auIdx(index).
     *
     * This function shall try to get the field name by specify the field 
     * index(auIdx) from the fetched result set which fetched by the last 
     * execution of Execute(apcSQL, abAutoFetch). Note that, the result 
     * set would only be fetched when abAutoFetch is set to true.
     *
     * @param[in]   auIdx   The field index.
     *
     * @retval APL_NULL     Get field name failed.
     * @retval !=APL_NULL   Field name corresponding to auIdx.
     *
     */
	virtual const char* GetFieldName(apl_size_t auIdx);

    /** 
     * Get the field index by specify the apcName(field name).
     *
     * This function shall try to get the field index by specify the field 
     * name(apcName) from the fetched result set which fetched by the last 
     * execution of Execute(apcSQL, abAutoFetch). Note that, the result 
     * set would only be fetched when abAutoFetch is set to true.
     *
     * @param[in]   apcName The field name.
     *
     * @retval >=0     Field index corresponding to apcName.
     * @retval =-1     Get field index failed.
     *
     */
    virtual apl_int_t GetFieldIndex(const char* apcName);

    /** 
     * Get the counts of row in result set.
     *
     * This function shall return the row counts of result set 
     * which generated by the last execute.
     *
     * @retval          Rows in result set.
     *
     */
    virtual apl_int_t GetRowCounts();   

    /** 
     * Get the error message of the last DB operation.
     *
     * This function shall try to get the error message of the last
     * DataBase operation. e.g. Execute(), Prepare(), FetchAll().
     *
     * @retval          Error message in string format.
     *
     */
    virtual const char* GetErrorMsg();

    /** 
     * Get the error code of the last DB operation.
     *
     * This function shall try to get the error code of the last
     * DataBase operation. e.g. Execute(), Prepare(), FetchAll().
     *
     * @retval          Error code in apl_int_t format.
     *
     */
    virtual apl_int_t GetErrorCode();

    /** 
     * Get the affected rows of the last execution.
     *
     * This function shall try to get the affected rows of the last
     * DataBase execution which issued by Execute(...). e.g. 
     * INSERT, DELETE, UPDATE ... 
     *
     * @retval          The affected rows by last execution.
     *
     */
    virtual apl_int_t GetAffectedRows();

    /** 
     * Binding parameters with SQL in a printf-like format.
     *
     * This function shall try to bind parameters with SQL in a 
     * printf-like format.
     *
     * @param[in]   auParamCounts   Counts of parameters embed in apcSQLStmt.
     * @param[in]   apcSQLStmt  SQL with parameters format embed. 
     * @param[in]   ...             auParamCounts count parameters should 
     *                              follow.
     *
     * @retval 0            Prepare success.
     * @retval !=0          Prepare failed.
     *
     */

    virtual apl_int_t Prepare(const char* apcSQLStmt);
            
    virtual apl_int_t BindParam(const char* apcParamName, apl_int16_t* apiParam);

    virtual apl_int_t BindParam(const char* apcParamName, apl_int32_t* apiParam);

    virtual apl_int_t BindParam(const char* apcParamName, apl_int64_t* apiParam);

    virtual apl_int_t BindParam(const char* apcParamName, apl_uint32_t* apuParam);

    virtual apl_int_t BindParam(const char* apcParamName, const char* apcParam, apl_size_t auLen);

    virtual apl_int_t BindParam(const char* apcParamName, const void* apcParam, apl_size_t auLen, bool abIsBinary);

    virtual apl_int_t BindParam(const char* apcParamName, float* apfParam);

    virtual apl_int_t BindParam(const char* apcParamName, double* apdParam);


    /** 
     * Set the pre-fetch row counts for Fetch() .
     *
     * This function would affect the behaviour of Fetch().
     *
     * By default, the pre-fetch row counts of fetching a result set are *ALL*
     * if this function is not involved. This function could specified the
     * pre-fetch row counts for Fetch(). That's say, when this function is 
     * involved, ADL would try to fetch at most auRows rows to pre-fetch buffer 
     * if there is no row in pre-fetch buffer any more, otherwise, ADL would 
     * fetch the whole result set in to pre-fetch buffer.
     *
     * @param[in]   auRows  The counts of pre-fetch rows.
     *
     */
    virtual void SetPreFetchRows(apl_size_t auRows);

    /** 
     * Fetch next row from result set.
     * 
     * After the call of this function, get the result with GetField series
     * function.
     *
     * SetPreFetchRows(apl_size_t auRows) would affect the behaviour of this
     * function. By default, it would pre-fetch the whole result in to buffer. 
     * If the pre-fetch counts is SET, it would try to fetch the specified rows 
     * when there is no rows in buffer any more.
     *
     * When the result set is known as very huge, specified the pre-fetch rows
     * with SetPreFetchRows(apl_size_t auRows) is always recommended.
     *
     *
     * @retval =0           Fetch success.
     * @retval !=0          Fetch failed.
     *
     */
    virtual apl_int_t FetchNext();

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
     *      SET character_set_results = apcCharacterSet;
     *      SET character_set_connection = apcCharacterSet;
     *
     * @param[in]   apcCharacterSet The character set which want to set.
     *
     * @retval =0           Set success.
     * @retval !=0          Set failed.
     *
     */
    virtual apl_int_t SetCharacters(const char* apcCharacterSet);

private:

    //for internal use
    ////////////////////////////////////////

    apl_int_t FetchMany(apl_size_t auRows);

    apl_int_t MoveToRow(apl_size_t luIndex);

    apl_int_t MoveToNextRow();
    
    apl_int_t MoveToPrevRow();
    
    apl_int_t AddFieldName(const char* apcField);

    void ClearFieldNames();

    void AddResult(void* apvPtr, apl_size_t auLen, apl_size_t auRow);

    void ClearMysqlResultStruct();

    void ClearResults();

    void SetErrorMsg(const char* apcMsg, apl_int_t aiErrCode);

    void ClearErrorMsg();

    void SetAffectedRows(apl_size_t auRows);

    void SetRowCounts(apl_size_t auRows);

    void ClearCursor();

    apl_size_t GetCurrentRow();

    void SetCurrentRow(apl_size_t auRow);

    void SetMysqlVConn(MYSQL* apoMysqlConn);

    MYSQL_STMT* GetMysqlStmt();

    void SetMysqlStmt(MYSQL_STMT* apoMysqlStmt);

    MYSQL_BIND* GetMysqlBind();

    void SetMysqlBind(MYSQL_BIND* apoMysqlBind);

    CParamBind* GetParamBind();

    // 0 means fetch all
    apl_int_t StoreResult(apl_size_t auFetchRows=0);

    apl_int_t StoreResultStmt(apl_size_t auFetchRows=0);

private:

    std::string moStatment;

    MYSQL* mpoMysqlConn;

    //result set of last call
    CResultSet moCurResultSet;

    //the latest error msg of exexute
    std::string moErrorMsg;

    apl_int_t miErrorCode;

    apl_size_t muAffectedRows;

    //for parse parameters binding
    CMySqlParamBind* mpoParamBind;

    MYSQL_STMT* mpoStmt;

    //bind parameters for output, for stmt only
    MYSQL_BIND* mpoBindOutput;

    //buffer for the len of output parameters
    apl_uint_t* mpuBindLenOutput; 

    //buffer for the content of output parameters
    std::vector<char*> moBindContentOutputVt; 

    //remember what we last execute
    apl_int_t miLastExecute;

    MYSQL_RES* mpoMysqlRes;

    //pre-fetch size
    apl_size_t muPreFetchRows;

    //row counts in last execute which return result set
    apl_size_t muRowCounts;
};


////////////////////////////////////////////////////////////////////////////////

#define EXECUTE_MODE_DONOT_FETCH        (0b00000000)
#define EXECUTE_MODE_DONOT_FETCH_STMT   (0b00000001)
#define EXECUTE_MODE_AUTO_FETCH         (0b00000010)
#define EXECUTE_MODE_AUTO_FETCH_STMT    (0b00000011)
#define EXECUTE_MODE_STMT_MASK          (0b00000001)
#define EXECUTE_MODE_FETCH_MASK         (0b00000010)

class CMySqlConnection : public IConnection
{

    friend class CMySqlDatabaseImpl;

    friend class CMySqlCursor;

public:

	CMySqlConnection();

	virtual ~CMySqlConnection();

    /** 
     * Get a cursor handler from connection.
     *
     * This function shall try to get a cursor from connection. All the 
     * Database operation/query is based on cursor. 
     * Multi-cursors could get from the same connection, but the 
     * connection are not thread-safety in MySql database interface.
     *
     * @retval APL_NULL     Can't get a valid cursor handler.
     * @retval !=APL_NULL   Pointer to the cursor handler.
     *
     */
	virtual ICursor* Cursor();

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
    virtual apl_int_t BeginTrans();

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
    virtual apl_int_t Commit();

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
    virtual apl_int_t Rollback();
    
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
     *      SET character_set_results = apcCharacterSet;
     *      SET character_set_connection = apcCharacterSet;
     *
     * @param[in]   apcCharacterSet The character set which want to set.
     *
     * @retval =0           Set success.
     * @retval !=0          Set failed.
     *
     */
    virtual apl_int_t SetCharacters(const char* apcCharacterSet);

    void ClearErrorMsg();

    void SetErrorMsg(const char* apcMsg, apl_int_t aiErrCode);

    /** 
     * Get the error message of the last DB operation.
     *
     * This function shall try to get the error message of the last
     * DataBase operation. 
     *
     * @retval          Error message in string format.
     *
     */
    virtual const char* GetErrorMsg();

    /** 
     * Get the error code of the last DB operation.
     *
     * This function shall try to get the error code of the last
     * DataBase operation.
     *
     * @retval          Error code in apl_int_t format.
     *
     */
    virtual apl_int_t GetErrorCode();

protected:
    //internal use
    void SetMysqlConn(MYSQL* apoMysql);

    MYSQL* GetMysqlConn();

    MYSQL* mpoMysqlConn;

    //CMySqlCursor* mpoMysqlCursor;
    acl::CLock moLock;

    std::string moErrMsg;

    apl_int_t miErrCode;
};

//////////////////////////////////////////////////////////////////////////////

struct CMYSQL
{
    CMySqlConnection* mpoConn;

    MYSQL* mpoMysql;

    bool mbConnected;

    bool mbOccupied;
};

typedef std::vector<struct CMYSQL> CMYSQL_LIST;



class CMySqlDatabaseImpl
{

public:
	CMySqlDatabaseImpl(
            const char* apcUser, 
            const char* apcPasswd, 
            const char* apcHostStr, 
            apl_int_t aiMaxConn,
            const char* apcDBName,
            apl_int_t aiPort);

	~CMySqlDatabaseImpl();

	apl_int_t Init();

	apl_int_t InitConnPool(
            const char* apcUser, 
            const char* apcPasswd, 
            const char* apcHostStr, 
            apl_int_t aiMinConn, 
            apl_int_t aiMaxConn, 
            const char* apcDBName,
            apl_int_t aiPort);

	void Destroy();

    IConnection* GetConnection(
            acl::CTimeValue const& aoTimeout = acl::CTimeValue::MAXTIME);

    void ReleaseConnection(IConnection* apoIConn);

    apl_int_t AddMysqlNode();

    void ClearErrorMsg();

    void SetErrorMsg(const char* apcMsg, apl_int_t aiErrCode);

    /** 
     * Get the error message of the last DB operation.
     *
     * This function shall try to get the error message of the last
     * DataBase operation. 
     *
     * @retval          Error message in string format.
     *
     */
    const char* GetErrorMsg();

    /** 
     * Get the error code of the last DB operation.
     *
     * This function shall try to get the error code of the last
     * DataBase operation.
     *
     * @retval          Error code in apl_int_t format.
     *
     */
    apl_int_t GetErrorCode();

protected:

    apl_int_t MysqlConnect(CMYSQL* apoCMysql);

    CMYSQL_LIST moMysqlList; //connection pool

    acl::CLock moLock;

    std::string moUser;

    std::string moPasswd;

    std::string moHost;

    apl_int_t miMinConnection;

    apl_int_t miMaxConnection;

    std::string moDBName;

	apl_int_t  miPort;

    std::string moErrMsg;

    apl_int_t miErrCode;
};


ADL_NAMESPACE_END


#endif // !defined(ADL_MYSQL_IMPL_H)
