
#ifndef AAF_LOGSERVER_H
#define AAF_LOGSERVER_H

#include "aaf/Server.h"
#include "acl/FileLog.h"
#include "acl/Reactor.h"
#include "acl/Synch.h"
#include "acl/Singleton.h"
#include "acl/ThreadManager.h"

#include "acl/SockHandle.h"

AAF_NAMESPACE_START

class CLogServer : public IServer, public acl::IEventHandler
{
public:
    CLogServer(void);
    
    virtual ~CLogServer(void);
    
    virtual apl_int_t Startup( char const* apcConfigFileName );
    
    virtual apl_int_t Restart(void);
    
    virtual apl_int_t Shutdown(void);
    
    apl_int_t Write( acl::ELLType aeLevel, char const* apcFormat, ... );
        
    apl_int_t WriteHex( acl::ELLType aeLevel, char const* apcPtr, apl_size_t auLength );
        
    apl_int_t WriteNonBlock( acl::ELLType aeLevel, char const* apcFormat, ... );
    
    apl_int_t WriteNonBlockHex( acl::ELLType aeLevel, char const* apcPtr, apl_size_t auLength );

    void SetFileName( char const* apcFileName );
    
    void SetLevelFileName( char const* apcFileName );
    
    void SetLevel( apl_int_t aiLevel );
    
    void SetStdErrLevel( apl_int_t aiLevel );
    
    char const* GetFileName(void);
    
    char const* GetLevelFileName(void);
    
    apl_int_t GetLevel(void);
    
    acl::ELLType GetStdErrLevel(void);
    
    void SetReactor( acl::CReactor* apoReactor );

protected:
    static void* Svc(void*);
    
    static void* LogLevelSvc(void*);
    
    void CheckBackup(void);
    
    virtual apl_int_t HandleOutput( apl_handle_t aiHandle );

private:
    bool mbIsShutdown;
    
    acl::CReactor* mpoReactor;
    apl_int_t      miEvents;
    
    acl::CSockHandlePair moHandlePair;
    
    acl::CSockHandlePair moStdErrPair;
    
    acl::CPollee moPollee;
        
    acl::CThreadManager moThreadManager;
    
    acl::CMemoryBlock moBuffer;

    acl::CTimeValue moMaxBakInterval;
        
    apl_size_t muMaxFileSize;
    
    char macFileName[APL_NAME_MAX];
    
    char macLevelFileName[APL_NAME_MAX];

    char macBackupDir[APL_PATH_MAX];
    
    acl::CTimestamp moLastBakTime;
    
    apl_int_t  miLogLevel;
    
    acl::ELLType meStdErrLevel;
        
    acl::CFileLog moLog;
        
    acl::CAnyLogFormat::DefaultFormatType moFormat;
    
    acl::CLock moLock;
};

AAF_NAMESPACE_END

//////////////////////////////////////////////////////////////////////////////
//non-block
#define AAF_LOG_ERROR_NB( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_ERROR) \
        acl::Instance<aaf::CLogServer>()->WriteNonBlock(acl::LOG_LVL_ERROR, __VA_ARGS__);

#define AAF_LOG_WARN_NB( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_WARN) \
        acl::Instance<aaf::CLogServer>()->WriteNonBlock(acl::LOG_LVL_WARN, __VA_ARGS__);
        
#define AAF_LOG_DEBUG_NB( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_DEBUG) \
        acl::Instance<aaf::CLogServer>()->WriteNonBlock(acl::LOG_LVL_DEBUG, __VA_ARGS__);
        
#define AAF_LOG_INFO_NB( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_INFO) \
        acl::Instance<aaf::CLogServer>()->WriteNonBlock(acl::LOG_LVL_INFO, __VA_ARGS__);

#define AAF_LOG_TRACE_NB( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_TRACE) \
        acl::Instance<aaf::CLogServer>()->WriteNonBlock(acl::LOG_LVL_TRACE, __VA_ARGS__);

//non-block and hex
#define AAF_LOG_ERROR_NB_HEX( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_ERROR) \
        acl::Instance<aaf::CLogServer>()->WriteNonBlockHex(acl::LOG_LVL_ERROR, __VA_ARGS__);

#define AAF_LOG_WARN_NB_HEX( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_WARN) \
        acl::Instance<aaf::CLogServer>()->WriteNonBlockHex(acl::LOG_LVL_WARN, __VA_ARGS__);
        
#define AAF_LOG_DEBUG_NB_HEX( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_DEBUG) \
        acl::Instance<aaf::CLogServer>()->WriteNonBlockHex(acl::LOG_LVL_DEBUG, __VA_ARGS__);
        
#define AAF_LOG_INFO_NB_HEX( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_INFO) \
        acl::Instance<aaf::CLogServer>()->WriteNonBlockHex(acl::LOG_LVL_INFO, __VA_ARGS__);

#define AAF_LOG_TRACE_NB_HEX( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_TRACE) \
        acl::Instance<aaf::CLogServer>()->WriteNonBlockHex(acl::LOG_LVL_TRACE, __VA_ARGS__);
        
//block
#define AAF_LOG_ERROR( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_ERROR) \
        acl::Instance<aaf::CLogServer>()->Write(acl::LOG_LVL_ERROR, __VA_ARGS__);

#define AAF_LOG_DEBUG( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_DEBUG) \
        acl::Instance<aaf::CLogServer>()->Write(acl::LOG_LVL_DEBUG, __VA_ARGS__);
        
#define AAF_LOG_INFO( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_INFO) \
        acl::Instance<aaf::CLogServer>()->Write(acl::LOG_LVL_INFO, __VA_ARGS__);

#define AAF_LOG_TRACE( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_TRACE) \
        acl::Instance<aaf::CLogServer>()->Write(acl::LOG_LVL_TRACE, __VA_ARGS__);

//block and hex
#define AAF_LOG_ERROR_HEX( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_ERROR) \
        acl::Instance<aaf::CLogServer>()->WriteHex(acl::LOG_LVL_ERROR, __VA_ARGS__);

#define AAF_LOG_DEBUG_HEX( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_DEBUG) \
        acl::Instance<aaf::CLogServer>()->WriteHex(acl::LOG_LVL_DEBUG, __VA_ARGS__);
        
#define AAF_LOG_INFO_HEX( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_INFO) \
        acl::Instance<aaf::CLogServer>()->WriteHex(acl::LOG_LVL_INFO, __VA_ARGS__);

#define AAF_LOG_TRACE_HEX( ... ) \
    if (acl::Instance<aaf::CLogServer>()->GetLevel() >= acl::LOG_LVL_TRACE) \
        acl::Instance<aaf::CLogServer>()->WriteHex(acl::LOG_LVL_TRACE, __VA_ARGS__);

#endif//AAF_LOGSERVER_H
