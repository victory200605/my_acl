
#include "MasterConfig.h"
#include "aaf/LogService.h"
#include "../config/FileConfig.h"
#include "QueueManager.h"

AIBC_GFQ_NAMESPACE_START

void UpdateHandler(void)
{
    AAF_LOG_INFO("%s", "Config modified and update config now");

    acl::Instance<CMasterConfig>()->Update();
}

//////////////////////////////////////////////////////////////////////////////////////////////
CMasterConfig::CMasterConfig(void)
    : mpoConfigImpl(APL_NULL)
{
}

CMasterConfig::~CMasterConfig(void)
{
    this->Close();
}

apl_int_t CMasterConfig::Initialize( char const* apcCmd )
{
    if ((this->mpoConfigImpl = CConfigFactory::Create(apcCmd) ) == APL_NULL)
    {
        return -1;
    }

    this->mpoConfigImpl->SetUpdateHandler(UpdateHandler);

    return this->Update();
}

void CMasterConfig::Close(void)
{
    CConfigFactory::Destroy(this->mpoConfigImpl);
}

apl_int_t CMasterConfig::Update(void)
{
    AAF_CONFIG_BIND(moLocalAddress, "main", "local_ip_address");
    AAF_CONFIG_BIND(miLocalPort, "main", "local_port");
    AAF_CONFIG_BIND(moGroupAddress, "main", "group_ip_address");
    AAF_CONFIG_BIND(miGroupPort, "main", "group_port");
    AAF_CONFIG_BIND(miKeepalivedTimedout, "main", "keepalived_timedout");
    AAF_CONFIG_BIND(miKeepalivedRetryTimes, "main", "keepalived_retry_times");
    AAF_CONFIG_BIND(miMaxConnectionCount, "main", "max_connection_count");
    AAF_CONFIG_BIND(miReadBufferSize, "main", "read_buffer_size");
    AAF_CONFIG_BIND(miWriteBufferSize, "main", "write_buffer_size");
    AAF_CONFIG_BIND(miAcceptorPPSize, "main", "acceptor_processor_pool_size");
    AAF_CONFIG_BIND(miConnectionIdleTimedout, "main", "connection_idle_timedout");
    AAF_CONFIG_BIND(miAllocateWeighting, "main", "allocate_weighting");
    AAF_CONFIG_BIND(moAllocateScheduling, "main", "allocate_scheduling");
    
    AAF_CONFIG_BIND_D(moDefaultName, "log_service", "name", "Master.log");
    AAF_CONFIG_BIND_D(moDefaultBackupDir, "log_service", "backup_dir", "LogBak");
    AAF_CONFIG_BIND_D(moLevelFileName, "log_service", "level_file_name", "Master.level");
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
    
    //Queue info table
    TableType loTable;
    QueueInfoTableType loTempTable;
    
    //Load table from configurator
    this->mpoConfigImpl->GetTable("queue_info", loTable);

    loTempTable.resize(loTable.size() );

    for (apl_size_t luRow = 0; luRow < loTable.size(); luRow++)
    {
        //Column count must > 3, name_pattern:capacity:type:bind_node_group
        if (loTable[luRow].size() < 3)
        {
            apl_errprintf("Queue info table column count error, (Count=%"APL_PRIdINT")\n", loTable[luRow].size() );

            return -1;
        }
        
        CQueueInfoRegex& loQueueInfo = loTempTable[luRow];
        //Regex compile name_pattern
        if (loQueueInfo.moRegex.Compile(loTable[luRow][0].c_str() ) != 0)
        {
            apl_errprintf("Queue info regex compile fail, (Expression=%s)\n", loTable[luRow][0].c_str() );
            
            return -1;
        }
        
        loQueueInfo.moName = loTable[luRow][0];
        loQueueInfo.muCapacity = apl_strtoi32(loTable[luRow][1].c_str(), APL_NULL, 10);
        loQueueInfo.moType = loTable[luRow][2];
        loQueueInfo.moBindNodeGroup = loTable[luRow].size() > 3 ? loTable[luRow][3] : "default";
        
    }
    
    {//Swap queue config info
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);

        this->moQueueInfos.swap(loTempTable);
    }

    //Notify queue manager update configuration for all queue
    GetQueueManager()->UpdateConfiguration();

    return 0;
}

bool CMasterConfig::FindQueueInfo( char const* apcQueueName, CQueueInfo& aoQueueInfo )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    for (apl_size_t luN = 0; luN < this->moQueueInfos.size(); luN++)
    {
        if (this->moQueueInfos[luN].moRegex.Match(apcQueueName) )
        {
            aoQueueInfo.muCapacity = this->moQueueInfos[luN].muCapacity;
            aoQueueInfo.moType = this->moQueueInfos[luN].moType;
            aoQueueInfo.moBindNodeGroup = this->moQueueInfos[luN].moBindNodeGroup;
            
            return true;
        }
    }

    return false;
}

bool CMasterConfig::GetValue( char const* apcField, char const* apcKey, std::string& aoVal )
{
    return this->mpoConfigImpl->GetValue(apcField, apcKey, aoVal);
}

bool CMasterConfig::GetValue( char const* apcField, char const* apcKey, apl_int64_t& aiVal )
{
    return this->mpoConfigImpl->GetValue(apcField, apcKey, aiVal);
}

bool CMasterConfig::GetValue( char const* apcField, char const* apcKey, apl_int32_t& aiVal )
{
    return this->mpoConfigImpl->GetValue(apcField, apcKey, aiVal);
}

bool CMasterConfig::IsFieldExisted( char const* apcField )
{
    return this->mpoConfigImpl->IsFieldExisted(apcField);
}

void CMasterConfig::PrintAll(void)
{
    CMasterConfig* lpoConfig = this;
    {
        AAF_LOG_INFO("-------------------------------------------------------------------------------------");
        AAF_LOG_INFO("|-%-40s :%-40s|", "local ip address", lpoConfig->moLocalAddress.c_str() );
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "local port", lpoConfig->miLocalPort);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "max connection count", lpoConfig->miMaxConnectionCount);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "acceptor processor pool size", lpoConfig->miAcceptorPPSize);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "read buffer size", lpoConfig->miReadBufferSize);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "write buffer size", lpoConfig->miWriteBufferSize);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "connection idle timedout", lpoConfig->miConnectionIdleTimedout);
        AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "allocate weighting", lpoConfig->miAllocateWeighting);
        AAF_LOG_INFO("|-%-40s :%-40s|", "allocate scheduling", lpoConfig->moAllocateScheduling.c_str() );
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
    for (CMasterConfig::LogHandlerTableType::iterator loIter = this->moLogHandlers.begin();
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

