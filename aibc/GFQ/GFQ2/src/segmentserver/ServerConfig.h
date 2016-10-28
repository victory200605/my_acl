
#ifndef AIBC_GFQ_MASTERCONFIG_H
#define AIBC_GFQ_MASTERCONFIG_H

#include "gfq2/Utility.h"
#include "../config/ConfigFactory.h"
#include "acl/Singleton.h"

AIBC_GFQ_NAMESPACE_START

struct CServerConfig
{
    //main
    std::string moNodeName;
    std::string moMasterAddress;
    apl_int_t   miMasterPort;
    std::string moLocalAddress;
    apl_int_t   miLocalPort;
    apl_int_t   miMaxConnectionCount;
    apl_int_t   miReadBufferSize;
    apl_int_t   miWriteBufferSize;
    apl_int_t   miAcceptorPPSize;
    apl_int_t   miConnectorPPSize;
    apl_int_t   miHeartbeatInterval;
    apl_int_t   miConnectionIdleTimedout;
    apl_int_t   miSegmentInfoSyncInterval;
    apl_int_t   miStatInfoSyncInterval;
    std::string moNodeGroup;
    apl_int_t   miNodePriority;

    //segment storage
    std::string moStoragePath;
    std::string moStorageType;
    std::string moStorageBufferPoolType;
    std::string moStorageBufferPoolSize;

    //log service
    std::string moDefaultName;
    std::string moLevelFileName;
    std::string moDefaultBackupDir;
    apl_int_t   miDefaultBufferSize;
    apl_int_t   miDefaultBackupSize;
    apl_int_t   miDefaultBackupTimeInterval;
    
    //table scp log handler
    struct CLogHandler
    {
        std::string moName;
        std::string moType;
        std::string moBackupDir;
        apl_int_t   miBufferSize;
        apl_int_t   miBackupSize;
        apl_int_t   miBackupTimeInterval;
        apl_int_t   miTriggerLevel;
        std::string moTriggerType;
    };
    std::vector<CLogHandler> moLogHandlers;
    typedef std::vector<CLogHandler> LogHandlerTableType;
    
    //method
    friend class acl::TSingleton<CServerConfig>;

    apl_int_t Initialize( char const* apcCmd );

    void Close(void);
    
    apl_int_t Update(void);

    bool GetValue( char const* apcField, char const* apcKey, std::string& aoVal );

    bool GetValue( char const* apcField, char const* apcKey, apl_int64_t& aiVal );

    bool GetValue( char const* apcField, char const* apcKey, apl_int32_t& aiVal );

    bool IsFieldExisted( char const* apcField );

    void PrintAll(void);

private:
    CServerConfig(void);
    
    ~CServerConfig(void);

private:
    IGFQConfig* mpoConfigImpl;
};

AIBC_GFQ_NAMESPACE_END

#endif//AIBC_GFQ_MASTERCONFIG_H

