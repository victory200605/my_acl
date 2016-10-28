
#include "ServerConfig.h"
#include "aaf/LogService.h"
#include "../config/FileConfig.h"

AIBC_GFQ_NAMESPACE_START

void UpdateHandler(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////
CServerConfig::CServerConfig(void)
    : mpoConfigImpl(APL_NULL)
{
}

CServerConfig::~CServerConfig(void)
{
    this->Close();
}

apl_int_t CServerConfig::Initialize( char const* apcCmd )
{
    if ((this->mpoConfigImpl = CConfigFactory::Create(apcCmd) ) == APL_NULL)
    {
        return -1;
    }

    this->mpoConfigImpl->SetUpdateHandler(UpdateHandler);

    return this->Update();
}

void CServerConfig::Close(void)
{
    CConfigFactory::Destroy(this->mpoConfigImpl);
}

apl_int_t CServerConfig::Update(void)
{
    AAF_CONFIG_BIND(moNodeName, "main", "name");
    AAF_CONFIG_BIND(moMasterAddress, "main", "master_ip_address");
    AAF_CONFIG_BIND(miMasterPort, "main", "master_port");
    AAF_CONFIG_BIND(moLocalAddress, "main", "local_ip_address");
    AAF_CONFIG_BIND(miLocalPort, "main", "local_port");
    AAF_CONFIG_BIND(miMaxConnectionCount, "main", "max_connection_count");
    AAF_CONFIG_BIND(miReadBufferSize, "main", "read_buffer_size");
    AAF_CONFIG_BIND(miWriteBufferSize, "main", "write_buffer_size");
    AAF_CONFIG_BIND(miAcceptorPPSize, "main", "acceptor_processor_pool_size");
    AAF_CONFIG_BIND(miConnectorPPSize, "main", "connector_processor_pool_size");
    AAF_CONFIG_BIND(miHeartbeatInterval, "main", "heartbeat_time_interval");
    AAF_CONFIG_BIND(miConnectionIdleTimedout, "main", "connection_idle_timedout");
    AAF_CONFIG_BIND(miSegmentInfoSyncInterval, "main", "node_segment_info_sync_interval");
    AAF_CONFIG_BIND(miStatInfoSyncInterval, "main", "node_stat_info_sync_interval");
    AAF_CONFIG_BIND(moNodeGroup, "main", "node_group");
    AAF_CONFIG_BIND(miNodePriority, "main", "node_priority");
    
    AAF_CONFIG_BIND(moStoragePath, "storage", "path");
    AAF_CONFIG_BIND(moStorageType, "storage", "type");
    AAF_CONFIG_BIND(moStorageBufferPoolType, "storage", "buffer_pool_type");
    AAF_CONFIG_BIND(moStorageBufferPoolSize, "storage", "buffer_pool_size");
    
    AAF_CONFIG_BIND_D(moDefaultName, "log_service", "name", "SegmentServer.log");
    AAF_CONFIG_BIND_D(moDefaultBackupDir, "log_service", "backup_dir", "LogBak");
    AAF_CONFIG_BIND_D(moLevelFileName, "log_service", "level_file_name", "SegmentServer.level");
    AAF_CONFIG_BIND_D(miDefaultBufferSize, "log_service", "buffer_size", 5242880);
    AAF_CONFIG_BIND_D(miDefaultBackupSize, "log_service", "backup_size", 5242880);
    AAF_CONFIG_BIND_D(miDefaultBackupTimeInterval, "log_service", "backup_time_interval", 900);
    
    AAF_CONFIG_BIND_TABLE_START(moLogHandlers, "log_handler")
        AAF_CONFIG_BIND_ROW(moLogHandlers, moName, "name");
        AAF_CONFIG_BIND_ROW_D(moLogHandlers, moName, "type", "file");
        AAF_CONFIG_BIND_ROW_D(moLogHandlers, moBackupDir, "backup_dir", "LogBak");
        AAF_CONFIG_BIND_ROW_D(moLogHandlers, miBufferSize, "buffer_size", miDefaultBufferSize);
        AAF_CONFIG_BIND_ROW_D(moLogHandlers, miBackupSize, "backup_size", miDefaultBackupSize);
        AAF_CONFIG_BIND_ROW_D(moLogHandlers, miBackupTimeInterval, "backup_time_interval", miDefaultBackupTimeInterval);
        AAF_CONFIG_BIND_ROW_D(moLogHandlers, miTriggerLevel, "trigger_level", -1);
        AAF_CONFIG_BIND_ROW_D(moLogHandlers, moTriggerType, "trigger_type", "b");
    AAF_CONFIG_BIND_TABLE_END();
    
    return 0;
}

bool CServerConfig::GetValue( char const* apcField, char const* apcKey, std::string& aoVal )
{
    return this->mpoConfigImpl->GetValue(apcField, apcKey, aoVal);
}

bool CServerConfig::GetValue( char const* apcField, char const* apcKey, apl_int64_t& aiVal )
{
    return this->mpoConfigImpl->GetValue(apcField, apcKey, aiVal);
}

bool CServerConfig::GetValue( char const* apcField, char const* apcKey, apl_int32_t& aiVal )
{
    return this->mpoConfigImpl->GetValue(apcField, apcKey, aiVal);
}

bool CServerConfig::IsFieldExisted( char const* apcField )
{
    return this->mpoConfigImpl->IsFieldExisted(apcField);
}

void CServerConfig::PrintAll(void)
{
    CServerConfig* lpoConfig = this;
    
    {
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
        AAF_LOG_INFO("|-%-40s :%-40s|", "master ip address", lpoConfig->moMasterAddress.c_str() );
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "master port", lpoConfig->miMasterPort);
        AAF_LOG_INFO("|-%-40s :%-40s|", "local ip address", lpoConfig->moLocalAddress.c_str() );
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "local port", lpoConfig->miLocalPort);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "max connection count", lpoConfig->miMaxConnectionCount);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "acceptor processor pool size", lpoConfig->miAcceptorPPSize);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "connector processor pool size", lpoConfig->miConnectorPPSize);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "read buffer size", lpoConfig->miReadBufferSize);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "write buffer size", lpoConfig->miWriteBufferSize);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "heartbeat time interval", lpoConfig->miHeartbeatInterval);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "connection idle timedout", lpoConfig->miConnectionIdleTimedout);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "server node priority", lpoConfig->miNodePriority);
        AAF_LOG_INFO("|-%-40s :%-40s|", "server node group", lpoConfig->moNodeGroup.c_str() );
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
    }

    {
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
        AAF_LOG_INFO("|-%-40s :%-40s|", "storage path", lpoConfig->moStoragePath.c_str() );
        AAF_LOG_INFO("|-%-40s :%-40s|", "storage type", lpoConfig->moStorageType.c_str() );
        AAF_LOG_INFO("|-%-40s :%-40s|", "storage buffer pool type", lpoConfig->moStorageBufferPoolType.c_str() );
        AAF_LOG_INFO("|-%-40s :%-40s|", "storage buffer pool size", lpoConfig->moStorageBufferPoolSize.c_str() );
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
    }

    {
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
        AAF_LOG_INFO("|-%-40s :%-40s|", "default log name", this->moDefaultName.c_str() );
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "default buffer size", this->miDefaultBufferSize);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "default backup size", this->miDefaultBackupSize);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "default backup interval", this->miDefaultBackupTimeInterval);
        AAF_LOG_INFO("|-%-40s :%-40s|", "default backup dir", this->moDefaultBackupDir.c_str() );
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
    }

    AAF_LOG_INFO("log_handler");
    AAF_LOG_INFO("-------------------------------------------------------------------------------------");
    AAF_LOG_INFO("|%14s|%8s|%8s|%8s|%13s|%13s|%13s|", "log_name", "type", "buffer", "bak_size", "bak_interval", "trigger_level", "trigger_type");
    AAF_LOG_INFO("-------------------------------------------------------------------------------------");
    for (CServerConfig::LogHandlerTableType::iterator loIter = this->moLogHandlers.begin();
         loIter != this->moLogHandlers.end(); ++loIter)
    {
        AAF_LOG_INFO("|%14s|%8s|%8"APL_PRIdINT"|%8"APL_PRIdINT"|%13"APL_PRIdINT"|%13"APL_PRIdINT"|%13s|", 
            loIter->moName.c_str(),
            loIter->moType.c_str(),
            loIter->miBufferSize,
            loIter->miBackupSize,
            loIter->miBackupTimeInterval,
            loIter->miTriggerLevel,
            loIter->moTriggerType.c_str() );
    }
    AAF_LOG_INFO("-------------------------------------------------------------------------------------");
}

AIBC_GFQ_NAMESPACE_END

