
#ifndef AIBC_GFQ_MASTERCONFIG_H
#define AIBC_GFQ_MASTERCONFIG_H

#include "gfq2/Utility.h"
#include "../config/ConfigFactory.h"
#include "acl/Singleton.h"
#include "acl/Regex.h"
#include "acl/Synch.h"

AIBC_GFQ_NAMESPACE_START

struct CMasterConfig
{
    //main
    std::string moLocalAddress;
    apl_int_t   miLocalPort;
    std::string moGroupAddress;
    apl_int_t   miGroupPort;
    apl_int_t   miKeepalivedTimedout;
    apl_int_t   miKeepalivedRetryTimes;
    apl_int_t   miMaxConnectionCount;
    apl_int_t   miReadBufferSize;
    apl_int_t   miWriteBufferSize;
    apl_int_t   miAcceptorPPSize;
    apl_int_t   miConnectionIdleTimedout;
    apl_int_t   miAllocateWeighting;
    std::string moAllocateScheduling;

    //log service
    std::string moDefaultName;
    std::string moLevelFileName;
    std::string moDefaultBackupDir;
    apl_int_t   miDefaultBufferSize;
    apl_int_t   miDefaultBackupSize;
    apl_int_t   miDefaultBackupTimeInterval;
    
    //table log handler
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
    typedef std::vector<CLogHandler> LogHandlerTableType;
    LogHandlerTableType moLogHandlers;
    
    //queue info table
    struct CQueueInfo
    {
        std::string moName;
        apl_size_t  muCapacity;
        std::string moType;
        std::string moBindNodeGroup;
    };
    struct CQueueInfoRegex : public CQueueInfo
    {
        acl::CRegex moRegex;
    };
    typedef std::vector<CQueueInfoRegex> QueueInfoTableType;
    QueueInfoTableType moQueueInfos;
    
    //method
    CMasterConfig(void);
    
    ~CMasterConfig(void);

    apl_int_t Initialize( char const* apcCmd );

    void Close(void);
    
    apl_int_t Update(void);

    bool FindQueueInfo( char const* apcQueueName, CQueueInfo& aoQueueInfo );

    bool GetValue( char const* apcField, char const* apcKey, std::string& aoVal );

    bool GetValue( char const* apcField, char const* apcKey, apl_int64_t& aiVal );

    bool GetValue( char const* apcField, char const* apcKey, apl_int32_t& aiVal );

    bool IsFieldExisted( char const* apcField );

    void PrintAll(void);

private:
    acl::CLock moLock;

    IGFQConfig* mpoConfigImpl;
};

AIBC_GFQ_NAMESPACE_END

#endif//AIBC_GFQ_MASTERCONFIG_H

