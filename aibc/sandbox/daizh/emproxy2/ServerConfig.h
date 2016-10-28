
#ifndef AIMC_EMPROXY_CONFIG_H
#define AIBC_EMPROXY_CONFIG_H

#include "aaf/ConfigService.h"

struct CServerConfig : public aaf::CConfigFileHandler
{
    CServerConfig(void);

    //main
    std::string moLocalAddress;
    apl_int_t   miLocalPort;
    apl_int_t   miMaxConnection;
    apl_int_t   miReadBufferSize;
    apl_int_t   miWriteBufferSize;
    apl_int_t   miAcceptorProcessorPoolSize;
    apl_int_t   miHeartbeatInterval;
    apl_int_t   miDefaultTimedout;

    //Remote pop server processor
    struct CPOPProcessor
    {
        std::string moHostName;
        apl_int_t   miQueueSize;
        apl_int_t   miThreadCount;
    };
    typedef std::vector<CPOPProcessor> ProcessorListType;
    ProcessorListType moPOPProcessores;

    apl_int_t BindAll(void);

    void PrintAll(void);
};

void SetConfigFilePath( char const* apcPath );

#endif//AIMC_EMPROXY_CONFIG_H

