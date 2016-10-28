
#ifndef AAF_LOGSERVICE_H
#define AAF_LOGSERVICE_H

#include "aaf/Service.h"
#include "acl/FileLog.h"
#include "acl/Synch.h"
#include "acl/Singleton.h"
#include "acl/ThreadManager.h"
#include "acl/MsgQueue.h"
#include "acl/SockHandle.h"
#include "acl/EventHandler.h"
#include "acl/TimerQueueAdapter.h"
#include "acl/stl/string.h"
#include "acl/stl/map.h"
#include "acl/stl/vector.h"

AAF_NAMESPACE_START

class CLogService;

/**
 * Log handler interface, all log implement will derive from it 
 */
class CLogHandler
{
public:
    CLogHandler(void);

	virtual ~CLogHandler(void);

//virtual hook method
    virtual apl_int_t Init( CLogService* apoService, char const* apcName ) = 0;
    
    virtual apl_int_t Destroy(void) = 0;
    
    virtual apl_int_t HandleWrite( acl::ELLType aeLevel, char const* apcBuffer, apl_size_t auLen ) = 0;
    
    virtual apl_int_t HandleTimeout( acl::CTimestamp const& aoCurrentTime, void const* apvAct = NULL ) = 0;
    
    virtual apl_int_t HandleFlush(void) = 0;

//attribute operator
    void SetBufferSize( apl_size_t auBufferSize );

    apl_size_t GetBufferSize(void);
    
    acl::CMemoryBlock& GetBuffer(void);

    void SetBackupSize( apl_size_t auBackupSize );

    apl_size_t GetBackupSize(void);

    void SetBackupTimeInterval( acl::CTimeValue const& aoInterval );

    acl::CTimeValue const& GetBackupTimeInterval(void);

    void SetBackupDir( char const* apcBackupDir );

    char const* GetBackupDir(void);

    void SetTriggerLevel( apl_int_t aiLevel );

    apl_int_t GetTriggerLevel(void);

private:
    apl_size_t muBackupSize;

    acl::CTimeValue moBackupTimeInterval;

    std::string moBackupDir;

    apl_int_t miTriggerLevel;

    acl::CMemoryBlock moBuffer;
};

/**
 * File log Handler, provide log writting to file service
 */
class CLogFileHandler : public CLogHandler, public acl::IEventHandler
{
public:
	CLogFileHandler(void);

	virtual ~CLogFileHandler(void);

    virtual apl_int_t Init( CLogService* apoService, char const* apcName );
    
    virtual apl_int_t Destroy(void);
    
    virtual apl_int_t HandleWrite( acl::ELLType aeLevel, char const* apcBuffer, apl_size_t auLen );
    
    virtual apl_int_t HandleTimeout( acl::CTimestamp const& aoCurrentTime, void const* apvAct = NULL );
   
    virtual apl_int_t HandleFlush(void);

protected:
    apl_ssize_t Write( char const* apcBuffer, apl_size_t auLen );

    apl_int_t Backup( acl::CTimestamp const& aoTimestamp );

    apl_int_t FlushAll(void);

private:
    acl::CTimestamp moLastBackupTime;

    std::string   moName;

    CLogService* mpoService;

    apl_int_t miTimerID;

    acl::CLock moLock;
    
    acl::CFileLog moFileLog;
};

/**
 * Provide log writting to terminal service
 */
class CLogTerminalHandler : public CLogHandler
{
public:
	virtual ~CLogTerminalHandler(void);

    virtual apl_int_t Init( CLogService* apoService, char const* apcName );
    
    virtual apl_int_t Destroy(void);
    
    virtual apl_int_t HandleWrite( acl::ELLType aeLevel, char const* apcBuffer, apl_size_t auLen );
    
    virtual apl_int_t HandleTimeout( acl::CTimestamp const& aoCurrentTime, void const* apvAct = NULL );
   
    virtual apl_int_t HandleFlush(void);

private:
    CLogService* mpoService;
};

/**
 * Provide log service to application, functions include dynamic log level changing, 
 * std error redirecting
 */
class CLogService : public IService
{
    typedef std::map<std::string, CLogHandler*> HandlerMapType;
    typedef std::vector<CLogHandler*> HandlerListType;

public:
    enum ETriggerType
    {
        TRIGGER_SELF = 0x01,
        TRIGGER_ALL  = 0x02,
        TRIGGER_ANY  = 0x04
    };

public:
    CLogService(void);
    
    virtual ~CLogService(void);
    
    virtual apl_int_t Startup( char const* apcConfigFileName );
    
    virtual apl_int_t Restart(void);
    
    virtual apl_int_t Shutdown(void);
    
    apl_int_t Write( char const* apcName, acl::ELLType aeLevel, char const* apcBuffer, apl_size_t auLen );
    
    template<typename FormaterType>
    apl_int_t Write( char const* apcName, FormaterType const& aoFormater )
    {
        return this->Write(apcName, aoFormater.GetLevel(), aoFormater.GetOutput(), aoFormater.GetLength() );
    }
    
    apl_int_t AddHandler( 
        char const* apcName, 
        CLogHandler* apoHandler, 
        apl_int_t aiTriggerLevel = -1, 
        apl_int_t aiTriggerType = TRIGGER_SELF|TRIGGER_ALL,
        apl_size_t auBufferSize = 0,
        apl_size_t auBackupSize = 1024*1024*5,
        acl::CTimeValue const& aoBackupTimeInterval = 900,
        char const* apcBackupDir = "LogBak" );
    
    apl_int_t AddHandler( 
        char const* apcName, 
        char const* apcType, 
        apl_int_t aiTriggerLevel = -1, 
        apl_int_t aiTriggerType = TRIGGER_SELF|TRIGGER_ALL,
        apl_size_t auBufferSize = 0,
        apl_size_t auBackupSize = 1024*1024*5,
        acl::CTimeValue const& aoBackupTimeInterval = 900,
        char const* apcBackupDir = "LogBak" );

    bool IsHandlerExisted( char const* apcName );
    
    void RemoveHandler( char const* apcName );

    void SetLevelFileName( char const* apcName );
    
    char const* GetLevelFileName(void);
    
    void SetLevel( apl_int_t aiLevel );
    
    apl_int_t GetLevel(void);
    
    void SetStdErrLevel( apl_int_t aiLevel );
    
    acl::ELLType GetStdErrLevel(void);
    
    void RedirectStdErr( char const* apcName );

    // Set default
    void SetDefaultName( char const* apcName );

    void SetDefaultBufferSize( apl_size_t auBufferSize );

    void SetDefaultBackupSize( apl_size_t auBackupSize );

    void SetDefaultBackupTimeInterval( acl::CTimeValue const& aoBackupTimeInterval );

    void SetDefaultBackupDir( char const* apcBackupDir );
    
    void SetDefaultTriggerType( apl_int_t aiTriggerType );
    
    acl::CTimerQueueAdapter& GetTimerQueueAdapter(void);

protected:
    static void* StdErrorSvc(void*);
    
    static void* LogLevelSvc(void*);

private:
    bool mbIsShutdown;

    std::map<std::string, CLogHandler*> moHandlers;
    
    std::vector<CLogHandler*> moAllHandlers;
    
    std::vector<CLogHandler*> moAnyHandlers;
    
    acl::CSockHandlePair moStdErrPair;
   
    acl::CThreadManager moThreadManager;

    std::string moStdErrName;
    
    // Level control file name
    std::string moLevelFileName;

    apl_int_t  miLogLevel;
    
    acl::ELLType meStdErrLevel;
    
    // Default variable
    std::string moDefaultName;
    apl_size_t muDefaultBufferSize;
    apl_size_t muDefaultBackupSize;
    acl::CTimeValue moDefaultBackupTimeInterval;
    std::string moDefaultBackupDir;
    apl_int_t miDefaultTriggerType;

    acl::CRWLock moLock;

    acl::CTimerQueueAdapter moTimerQueueAdapter;
};

AAF_NAMESPACE_END

//////////////////////////////////////////////////////////////////////////////
#define AAF_LOG_WRITE( name, level, ... ) \
    if (acl::Instance<aaf::CLogService>()->GetLevel() >= level) \
        acl::Instance<aaf::CLogService>()->Write(name, acl::CDefaultFormatter<4096>(level, __VA_ARGS__) );

#define AAF_LOG_WRITE_HEX( name, level, ... ) \
    if (acl::Instance<aaf::CLogService>()->GetLevel() >= level) \
        acl::Instance<aaf::CLogService>()->Write(name, acl::CDefaultFormatter<4096>(level, "\n%s", acl::CHexFormatter(__VA_ARGS__).GetOutput() ) );

#define AAF_LOG_ERROR( ... ) AAF_LOG_WRITE(NULL, acl::LOG_LVL_ERROR, __VA_ARGS__);
#define AAF_LOG_WARN( ... ) AAF_LOG_WRITE(NULL, acl::LOG_LVL_WARN, __VA_ARGS__);
#define AAF_LOG_DEBUG( ... ) AAF_LOG_WRITE(NULL, acl::LOG_LVL_DEBUG, __VA_ARGS__);
#define AAF_LOG_INFO( ... ) AAF_LOG_WRITE(NULL, acl::LOG_LVL_INFO, __VA_ARGS__);
#define AAF_LOG_TRACE( ... ) AAF_LOG_WRITE(NULL, acl::LOG_LVL_TRACE, __VA_ARGS__);

#define AAF_LOG_ERROR_EX( name, ... ) AAF_LOG_WRITE(name, acl::LOG_LVL_ERROR, __VA_ARGS__);
#define AAF_LOG_WARN_EX( name, ... ) AAF_LOG_WRITE(name, acl::LOG_LVL_WARN, __VA_ARGS__);
#define AAF_LOG_DEBUG_EX( name, ... ) AAF_LOG_WRITE(name, acl::LOG_LVL_DEBUG, __VA_ARGS__);
#define AAF_LOG_INFO_EX( name, ... ) AAF_LOG_WRITE(name, acl::LOG_LVL_INFO, __VA_ARGS__);
#define AAF_LOG_TRACE_EX( name, ... ) AAF_LOG_WRITE(name, acl::LOG_LVL_TRACE, __VA_ARGS__);

//hex
#define AAF_LOG_ERROR_HEX( ... ) AAF_LOG_WRITE_HEX(NULL, acl::LOG_LVL_ERROR, __VA_ARGS__);
#define AAF_LOG_WARN_HEX( ... ) AAF_LOG_WRITE_HEX(NULL, acl::LOG_LVL_WARN, __VA_ARGS__);
#define AAF_LOG_DEBUG_HEX( ... ) AAF_LOG_WRITE_HEX(NULL, acl::LOG_LVL_DEBUG, __VA_ARGS__);
#define AAF_LOG_INFO_HEX( ... ) AAF_LOG_WRITE_HEX(NULL, acl::LOG_LVL_INFO, __VA_ARGS__);
#define AAF_LOG_TRACE_HEX( ... ) AAF_LOG_WRITE_HEX(NULL, acl::LOG_LVL_TRACE, __VA_ARGS__);

#define AAF_LOG_ERROR_HEXEX( name, ... ) AAF_LOG_WRITE_HEX(name, acl::LOG_LVL_ERROR, __VA_ARGS__);
#define AAF_LOG_WARN_HEXEX( name, ... ) AAF_LOG_WRITE_HEX(name, acl::LOG_LVL_WARN, __VA_ARGS__);
#define AAF_LOG_DEBUG_HEXEX( name, ... ) AAF_LOG_WRITE_HEX(name, acl::LOG_LVL_DEBUG, __VA_ARGS__);
#define AAF_LOG_INFO_HEXEX( name, ... ) AAF_LOG_WRITE_HEX(name, acl::LOG_LVL_INFO, __VA_ARGS__);
#define AAF_LOG_TRACE_HEXEX( name, ... ) AAF_LOG_WRITE_HEX(name, acl::LOG_LVL_TRACE, __VA_ARGS__);

#endif//AAF_LOGSERVICE_H
