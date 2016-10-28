
#include "ServerConfig.h"
#include "Utility.h"

std::string goConfigFilePath = "../config/EMProxy.ini";

CServerConfig::CServerConfig(void)
    : aaf::CConfigFileHandler(goConfigFilePath.c_str() )
{
}

apl_int_t CServerConfig::BindAll(void)
{
    AAF_CONFIG_BIND(moLocalAddress, "main", "local_ip_address");
    AAF_CONFIG_BIND(miLocalPort, "main", "local_port");
    AAF_CONFIG_BIND_D(miMaxConnection, "main", "max_connection", 128);
    AAF_CONFIG_BIND_D(miReadBufferSize, "main", "read_buffer_size", 40860);
    AAF_CONFIG_BIND_D(miWriteBufferSize, "main", "write_buffer_size", 1024);
    AAF_CONFIG_BIND_D(miAcceptorProcessorPoolSize, "main", "acceptor_processor_pool_Size", 8);
    AAF_CONFIG_BIND_D(miHeartbeatInterval, "main", "heartbeat_interval", 10);
    AAF_CONFIG_BIND_D(miDefaultTimedout, "main", "default_timedout", 10);
    
    AAF_CONFIG_BIND_TABLE_START(moPOPProcessores, "pop_processor")
        AAF_CONFIG_BIND_ROW(moPOPProcessores, moHostName, "host_name");
        AAF_CONFIG_BIND_ROW(moPOPProcessores, miQueueSize, "queue_size");
        AAF_CONFIG_BIND_ROW(moPOPProcessores, miThreadCount, "thread_count");
    AAF_CONFIG_BIND_TABLE_END();
    
    return 0;
}

void CServerConfig::PrintAll(void)
{
    {
        EMP_LOG_INFO("%s", "-------------------------------------------------------------------------------------");
        EMP_LOG_INFO("|-%-40s :%-40s|", "local ip address", this->moLocalAddress.c_str() );
        EMP_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "local port", this->miLocalPort);
        EMP_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "max connection count", this->miMaxConnection);
        EMP_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "acceptor processor pool size", this->miAcceptorProcessorPoolSize);
        EMP_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "read buffer size", this->miReadBufferSize);
        EMP_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "write buffer size", this->miWriteBufferSize);
        EMP_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "heartbeat time interval", this->miHeartbeatInterval);
        EMP_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "default timedout", this->miDefaultTimedout);
        EMP_LOG_INFO("%s", "-------------------------------------------------------------------------------------");
    }
    EMP_LOG_INFO("%s", "-------------------------------------------------------------------------------------");
    EMP_LOG_INFO("|%-41s|%-20s|%-20s|", "host_name", "queue_size", "thread_count");
    EMP_LOG_INFO("%s", "-------------------------------------------------------------------------------------");
    for (std::vector<CPOPProcessor>::iterator loIter = this->moPOPProcessores.begin();
         loIter != this->moPOPProcessores.end(); ++loIter)
    {
        EMP_LOG_INFO("|%-41s|%-20"APL_PRIdINT"|%-20"APL_PRIdINT"|", 
            loIter->moHostName.c_str(),
            loIter->miQueueSize,
            loIter->miThreadCount )
    }
    EMP_LOG_INFO("%s", "-------------------------------------------------------------------------------------");
}

void SetConfigFilePath( char const* apcPath )
{
    goConfigFilePath = apcPath;
}

