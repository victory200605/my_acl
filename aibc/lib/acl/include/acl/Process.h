/**
 * @file Process.h
 */

#ifndef ACL_PROCESS_H
#define ACL_PROCESS_H

#include "acl/Utility.h"
#include "acl/TimeValue.h"
#include "acl/IOHandle.h"
#include "acl/stl/vector.h"

ACL_NAMESPACE_START

///////////////////////////////////////////// ProcessOption ////////////////////////////////////

/**
 * class CProcessOption
 */
class CProcessOption
{
public:
    static const apl_size_t MAX_CMDLINE_LEN = 1024;   ///<command line length
    static const apl_size_t MAX_ENVBUFFER_LEN = 1024; ///<environment variable buffer length
    static const apl_size_t MAX_ARG_SIZE = 100;       ///<argument size
    static const apl_size_t MAX_ENV_SIZE = 100;       ///<environment variable size
    
public:

    /**
     * @brief A constructor.
     */
    CProcessOption(void);
    
    /**
     * @brief A destructor.
     */
    ~CProcessOption(void);
    
    /**
     * @brief Set command line.
     *
     * @param [in] apcFormat - the command line format
     * @param [in] ... - variable argument list
     *
     * retval 0 Upon successful completion.
     */
    apl_int_t SetCmdLine( char const* apcFormat, ... );
    
    /**
     * @brief Get command line.
     *
     * @return The command line.
     */
    char* const* GetCmdLine(void) const;
    
    /**
     * @brief Set process name.
     *
     * @param [in] apcProcName - the specified process name
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t SetProcName( char const* apcProcName );
    
    /**
     * @brief Get process name.
     *
     * @return The process name.
     */
    char const* GetProcName(void) const;
    
    /**
     * @brief Redirect stdin/stdout/stderr handle. 
     *
     * @param [in] aiStdIn - the stdin handle
     * @param [in] aiStdOut - the stdout handle
     * @param [in] aiStdErr - the stderr handle
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t SetHandle(
        apl_handle_t aiStdIn,
        apl_handle_t aiStdOut = ACL_INVALID_HANDLE,
        apl_handle_t aiStdErr = ACL_INVALID_HANDLE );
        
    /**
     * @brief Get stdin handle.
     *
     * @return The stdin handle.
     */
    apl_handle_t GetStdIn(void) const;
    
    /**
     * @brief Get stdout handle.
     *
     * @return The stdout handle.
     */
    apl_handle_t GetStdOut(void) const;
    
    /**
     * @brief Get stderr handle.
     *
     * @return The stderr handle.
     */
    apl_handle_t GetStdErr(void) const;
    
    /**
     * @brief Pass handle to the child process.
     *
     * @param [in] aiHandle - the handle to pass
     */
    void PassHandle( apl_handle_t aiHandle );
    
    /**
     * @brief Set environment variable.
     *
     * @param [in] apcFormat - the environment format
     * @param [in] ... - the variable argument list
     *
     * @retval 0 Upon successful completion. 
     */
    apl_int_t SetEnv( char const* apcFormat, ... );
    
    /**
     * @brief Get environment variable array.
     *
     * @return The environment variable array.
     */
    char* const* GetEnv(void) const;
    
    /**
     * @brief Set working directory.
     *
     * @param [in] apcDirectory - the working directory
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t SetWorkingDir( char const* apcDirectory );
    
    /**
     * @brief Get working directory.
     *
     * @return The working directory.
     */
    char const* GetWorkingDir(void) const;
    
    /**
     * @brief Set passed handle and command line.
     */
    void Prepare(void);
    
private:
    char* mpcCommandLine;
    
    char** mppcArgv;
    
    apl_size_t muArgvIndex;
    
    char* mpcEnvBuffer;
    
    apl_size_t muEnvBufferIndex;
    
    char** mppcEnv;
    
    apl_size_t muEnvIndex;
    
    char* mpcWorkingDir;
    
    char* mpcProcessName;

    apl_handle_t miStdIn;
    apl_handle_t miStdOut;
    apl_handle_t miStdErr;
    
    std::vector<apl_handle_t> moHandleSet;
};

/////////////////////////////////////////////// Process //////////////////////////////////////////

/**
 * class CProcess
 */
class CProcess
{
public:

    /**
     * @brief Process constructor.
     */
    CProcess(void);

    /** 
     * @brief Process destructor
     */
    virtual ~CProcess(void);
    
    /**
     * @brief Create process.
     *
     * @param [in] apoOption - the process option
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t Spawn( CProcessOption& apoOption );
    
    /**
     * @brief Wait for a child process to stop or terminate.
     *
     * @param [out] apiExitCode - the child process status information
     * @param [in] aoTimeout - timeout
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval >0 The process ID of the child process
     * @retval -1 Otherwise, -1 shall be returned.
     */
    apl_int_t Wait( apl_int_t* apiExitCode,  CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
    
    /**
     * @brief Send a signal to a child process. 
     *
     * @param [in] aiSignal - the signal to send
     * 
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t Kill( apl_int_t aiSignal ) const;
    
    /**
     * @brief Get the child process ID.
     *
     * @return The process ID of child process.
     */
    apl_pid_t GetChildID(void);
    
    /**
     * @brief Get the process ID.
     *
     * @return The process ID of the calling process.
     */
    static apl_pid_t GetPid(void);
    
    /**
     * @brief Get the parent process ID.
     *
     * @return The parent process ID.
     */
    static apl_pid_t GetPpid(void);
    
    /**
     * @brief A virtual member.
     * The code in parent process.
     */
    virtual void Parent(void);
    
    /**
     * @brief A virtual member.
     * The code in chile process.
     */
    virtual void Child(void);

private:
    apl_pid_t miChildID;
    
    apl_handle_t miHandle;
};

ACL_NAMESPACE_END

#endif //ACL_PROCESS_H

