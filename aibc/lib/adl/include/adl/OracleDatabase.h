#ifndef ADL_ORACLE_DATABASE_H
#define ADL_ORACLE_DATABASE_H

#include "adl/Adl.h"
#include "acl/Timestamp.h"

ADL_NAMESPACE_START

class COracleDatabase : public IDatabase
{
public:
	COracleDatabase(
        const char* apcUser, 
        const char* apcPasswd, 
        const char* apcHostStr, 
        apl_int_t aiMaxConn );

    ~COracleDatabase(void);

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
    class COracleDatabaseImpl* mpoImpl;
};

ADL_NAMESPACE_END

#endif//ADL_ORACLE_DATABASE_H

