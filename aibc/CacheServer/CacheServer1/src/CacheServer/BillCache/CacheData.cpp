//=============================================================================
/**
* \file    CacheData.h
* \brief store file
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: CacheData.cpp,v 1.1 2011/01/05 08:34:16 daizh Exp $
*
* History
* 2008.06.18 first release by DZH
*/
//=============================================================================

#include "CacheData.h"

AI_CACHE_NAMESPACE_START

//----------------------------------stDataNode-----------------------------//
stDataNode::stDataNode()
{
    memset( this, 0, sizeof(stDataNode) );
}

//----------------------------------clsCacheData---------------------------//
int clsCacheData::Create( const char* apsFileName, int aiCount, size_t aiSize )
{
    int liRet = TBase::Create( apsFileName, aiCount, aiSize );
    if ( liRet == AI_ERROR_OPEN_FILE )
    {
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
	        "Open file %s fail, MSG:%s", apsFileName, strerror(errno) );
#endif
    }
    else if ( liRet == AI_ERROR_WRITE_FILE )
    {
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		    "Write file %s fail, MSG:%s", apsFileName, strerror(errno) );
#endif
    }
    return liRet;
}

int clsCacheData::Open( const char* apsFileName, int aiCount, size_t aiSize )
{
    int liRet = TBase::Open( apsFileName, aiCount, aiSize );
    if ( liRet == AI_ERROR_OPEN_FILE )
    {
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
	        "Open file %s fail, MSG:%s", apsFileName, strerror(errno) );
#endif
    }
    return liRet;
}

int clsCacheData::Read( int aiIdx, AIChunkEx& aoKey, AIChunkEx& aoValue, int* apiTimestamp )
{
    stDataNode loDataNode;
    int liRet = TBase::Read( aiIdx, loDataNode );
    if ( liRet != AI_NO_ERROR )
    {
        if ( liRet == AI_ERROR_OUT_OF_RANGE )
        {
    #if defined(AI_ENABLE_BILLCACHE_LOG)
            AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, "File index %d out of range", GetFileIdx( aiIdx ) );
    #endif
        }
        else if ( liRet == AI_ERROR_READ_FILE )
        {
    #if defined(AI_ENABLE_BILLCACHE_LOG)
            AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
    		    "Read file fail, file index = %d / record offset = %ld, MSG:%s", GetFileIdx( aiIdx ), GetFileOffset( aiIdx ), strerror(errno) );
    #endif
        }
        //Oh, no, read data node fail!
        return liRet;
    }
    //check data valid
    if ( loDataNode.ciDataSize <= 0 || loDataNode.ciDataSize > AI_MAX_VALUE_LEN )
    {
        //Oh, invalid data, may be filesystem destroy
        return AI_ERROR_INVALID_DATA;
    }
    
    //set return variable
    if ( apiTimestamp != NULL )
    {
        *apiTimestamp = loDataNode.ciTimestamp;
    }

    aoKey.Resize(loDataNode.ciKeySize);
    memcpy( aoKey.BasePtr(), loDataNode.ccMsgID, loDataNode.ciKeySize );
    
    aoValue.Resize(loDataNode.ciDataSize);
    memcpy( aoValue.BasePtr(), loDataNode.ccData, loDataNode.ciDataSize );
    
    return AI_NO_ERROR;
}

int clsCacheData::Write( int aiIdx, const AIChunkEx& aoKey, const AIChunkEx& aoValue, int aiTimestamp )
{
    if ( aoValue.GetSize() > AI_MAX_VALUE_LEN )
    {
        //Oh, sorry, i can't help you
        return AI_ERROR_OUT_OF_RANGE;
    }

    stDataNode loDataNode;
    //set save variable
    loDataNode.ciKeySize   = aoKey.GetSize();
    loDataNode.ciDataSize  = aoValue.GetSize();
    loDataNode.ciTimestamp = aiTimestamp;
    memcpy( loDataNode.ccMsgID, aoKey.BasePtr(), aoKey.GetSize() );
    memcpy( loDataNode.ccData, aoValue.BasePtr(), aoValue.GetSize() );
    
    int liRet = TBase::Write( aiIdx, loDataNode );
    if ( liRet == AI_ERROR_OUT_OF_RANGE )
    {
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, "File index %d out of range", GetFileIdx( aiIdx ) );
#endif
    }
    else if ( liRet == AI_ERROR_WRITE_FILE )
    {
#if defined(AI_ENABLE_BILLCACHE_LOG)
        AIWriteLOG( AI_BILLCACHE_SYS_LOG, AILOG_LEVEL_SYSTEM, 
		    "Write file fail, file index = %d / record offset = %ld, MSG:%s", GetFileIdx( aiIdx ), GetFileOffset( aiIdx ), strerror(errno) );
#endif
    }
    
    return liRet;
}

AI_CACHE_NAMESPACE_END
