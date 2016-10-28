
#ifndef COUNTSERVER_LOGSERVER_H
#define COUNTSERVER_LOGSERVER_H

#include "acl/Reactor.h"
#include "acl/SockHandle.h"
#include "acl/ThreadManager.h"
#include "acl/MemoryBlock.h"
#include "acl/FileLog.h"

class CLogServer : public acl::IEventHandler
{
public:
    static CLogServer* Instance(void);
    
    static void Release();

    ~CLogServer(void);
    
    apl_int_t Startup( acl::CReactor& aoReactor, char const* apcFileName );
    
    void Close(void);
    
    apl_int_t Write( acl::ELLType aeLevel, char const* apcFormat, ... );
        
    apl_int_t WriteNonBlock( acl::ELLType aeLevel, char const* apcFormat, ... );
    
    apl_int_t WriteNonBlockHex( acl::ELLType aeLevel, char const* apcPtr, apl_size_t auLength );
    
    void SetMaxTime( acl::CTimeValue const& aoTime );
    
    void SetMaxLogSize( apl_size_t auSize );
    
    void SetBackupDir( char const* apcPath );
    
    void SetCacheSize( apl_size_t auSize );
    
    void SetProcessName( char const* apcProcessName );
    
    void SetLevel( apl_int_t aiLevel );
    
    char const* GetProcessName(void);
    
    apl_int_t GetLevel(void);
    
    static const char* UStringToHex(
        char const* aspBuff,
        apl_size_t aiSize,
        acl::CMemoryBlock& aoOut,
        apl_size_t aiColumn = 4,
        apl_size_t aiGroup = 4 );
    
protected:
    CLogServer(void);
        
    static void* Svc(void*);
    
    static void* LogLevelSvc(void*);
    
    //Hex log
    static char* CharToHex( unsigned char acCh, char* apcRet, apl_size_t auSize );

    virtual apl_int_t HandleOutput( apl_handle_t aiHandle );

private:
    static CLogServer* cpoInstance;

private:
    bool mbIsClosed;
    
    acl::CReactor* mpoReactor;
    apl_int_t      miEvents;
    
    acl::CSockHandlePair moHandlePair;
        
    acl::CThreadManager moThreadManager;
    
    acl::CMemoryBlock moBuffer;

    acl::CTimeValue moMaxLogTime;
        
    apl_size_t muMaxLogSize;
    
    char macLogFileName[APL_NAME_MAX];
    
    char macProcessName[APL_NAME_MAX];
    
    char macBackupDir[APL_PATH_MAX];
    
    apl_int_t  miLogLevel;
        
    acl::CFileLog moLog;
        
    acl::CAnyLogFormat::DefaultFormatType moFormat;
};

//////////////////////////////////////////////////////////////////////////////
#define CNTSRV_LOG_ERROR_NB( ... ) \
    CLogServer::Instance()->WriteNonBlock(acl::LOG_LVL_ERROR, __VA_ARGS__);

#define CNTSRV_LOG_WARN_NB( ... ) \
    CLogServer::Instance()->WriteNonBlock(acl::LOG_LVL_WARN, __VA_ARGS__);
        
#define CNTSRV_LOG_DEBUG_NB( ... ) \
    CLogServer::Instance()->WriteNonBlock(acl::LOG_LVL_DEBUG, __VA_ARGS__);
        
#define CNTSRV_LOG_INFO_NB( ... ) \
    CLogServer::Instance()->WriteNonBlock(acl::LOG_LVL_INFO, __VA_ARGS__);

#define CNTSRV_LOG_TRACE_NB( ... ) \
    CLogServer::Instance()->WriteNonBlock(acl::LOG_LVL_TRACE, __VA_ARGS__);
        
#define CNTSRV_LOG_ERROR( ... ) \
    CLogServer::Instance()->Write(acl::LOG_LVL_ERROR, __VA_ARGS__);

#define CNTSRV_LOG_DEBUG( ... ) \
    CLogServer::Instance()->Write(acl::LOG_LVL_DEBUG, __VA_ARGS__);
        
#define CNTSRV_LOG_INFO( ... ) \
    CLogServer::Instance()->Write(acl::LOG_LVL_INFO, __VA_ARGS__);

#define CNTSRV_LOG_TRACE( ... ) \
    CLogServer::Instance()->Write(acl::LOG_LVL_TRACE, __VA_ARGS__);
        
#endif //COUNTSERVER_LOGSERVER_H
