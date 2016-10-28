//=============================================================================
/**
 * \file    GFQueue.cpp
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: GFQueue.cpp,v 1.2 2012/04/09 08:24:46 daizh Exp $
 *
 * History
 * 2008.04.04 first release by DZH
 */
//=============================================================================
#include "GFQueue.h"
#include "gfq/GFQProtocol.h"

// start namespace
AI_GFQ_NAMESPACE_START

clsGFQueue::clsGFQueue() : 
	cfIsPermisionFunc(NULL), 
	cfLoadFunc(NULL),
	ciQueueDefaultCapacity(199999999)
{
}

void clsGFQueue::SetErrorLimit( int32_t aiNum )
{
    GetModuleMgr()->SetErrorLimit(aiNum);
}

int32_t clsGFQueue::LoadPermisionFunc( const char* apsLibName )
{
	if ( coShlib.Load( apsLibName ) != 0 )
	{
		return AI_ERROR_LOAD_SHLIB;
	}
	// Load queue info
	cfLoadFunc = coShlib.GetSymbol<TLoadQueueInfoFunc>("LoadQueueInfo");
	if ( cfLoadFunc == NULL )
	{
		return AI_ERROR_QUEUEINFO_LOAD_FUNC_UNDEFINED;
	}
	
	// Load queue info first here
	if ( (cfLoadFunc)() != 0 )
	{
		return AI_ERROR_QUEUEINFO_LOAD;
	}
	
	// Load permision function from share lib
	cfIsPermisionFunc = coShlib.GetSymbol<TPermisionFunc>("IsPermision");
	if ( cfIsPermisionFunc == NULL )
	{
		return AI_ERROR_PERMISION_FUNC_UNDEFINED;
	}
	
	return AI_NO_ERROR;
}

void clsGFQueue::SetQueueDefaultCapacity( int32_t aiCapacity )
{
	ciQueueDefaultCapacity = aiCapacity;
}

bool clsGFQueue::IsPermision( const char* apsQueueName, int32_t& aiQueueCapacity )
{
	// lock scope
	AISmartLock loLock(coQueueInfoLock);
	
	if ( cfIsPermisionFunc != NULL && (cfIsPermisionFunc)( apsQueueName, &aiQueueCapacity ) != 0 )
	{
	    return false;
	}
	
	return true;
}

int32_t clsGFQueue::LoadQueueInfo()
{
	// lock scope
	AISmartLock loLock(coQueueInfoLock);
	
	if ( cfLoadFunc != NULL )
	{
		if ( (cfLoadFunc)() != 0 )
		{
			return AI_ERROR_QUEUEINFO_LOAD;
		}
	}
	
	return AI_NO_ERROR;
}

void clsGFQueue::RefreshQueue()
{
	int32_t liQueueCapacity = 0;
	
	// lock scope
	AISmartLock loLock(coLock);
	
	clsQueueMgr::TIterator liIt = coQueueMgr.QueueBegin();
	for ( ; liIt != coQueueMgr.QueueEnd(); liIt++ )
	{
		if ( IsPermision( liIt->ccQueueName, liQueueCapacity ) )
		{
		    liIt->ciQueueCapacity = liQueueCapacity;
		}
	}
}

// for store module
int32_t clsGFQueue::AddModule( const char* apsModuleName, const char* apsIpAddr, int32_t aiPort )
{
	// lock scope
	AISmartLock loLock(coLock);
	
	return GetModuleMgr()->AddModule( apsModuleName, apsIpAddr, aiPort );
}

int32_t clsGFQueue::AddSegment( const stSegmentServerInfo& aoSegmentInfo )
{
	// lock scope
	AISmartLock loLock(coLock);
	
	int32_t liRet = AI_NO_ERROR;
	
	TModuleHandle ltModuleHandle = GetModuleMgr()->SearchModule( aoSegmentInfo.ccModuleName );
	if ( ltModuleHandle == INVALID_MODULE_HANDLE )
	{
		return AI_ERROR_NO_FOUND_MODULE;
	}
	
	stSegmentNode* lpoSegmentNode = GetModuleMgr()->CreateSegment( ltModuleHandle );
	lpoSegmentNode->ciSegmentID   = aoSegmentInfo.ciSegmentID;
	lpoSegmentNode->ciSegmentSize = aoSegmentInfo.ciSegmentSize;
	lpoSegmentNode->ciSegmentCapacity = aoSegmentInfo.ciSegmentCapacity;
	lpoSegmentNode->ciQueueGlobalID   = aoSegmentInfo.ciQueueGlobalID;
	
	do
	{
		if ( aoSegmentInfo.ciQueueGlobalID > 0 && strlen(aoSegmentInfo.ccQueueName) > 0 )
		{
			// Search queue
			TQueueHandle ltQueueHandle = coQueueMgr.SearchQueue( aoSegmentInfo.ccQueueName );
			if ( ltQueueHandle == INVALID_QUEUE_HANDLE )
			{// Unexisted ? create it
				if ( ( liRet = CreateQueue( aoSegmentInfo.ccQueueName, ltQueueHandle ) ) != AI_NO_ERROR )
				{
					break;
				}
			}
			// Insert queue segment
			if ( ( liRet = coQueueMgr.InsertSegment( ltQueueHandle, lpoSegmentNode ) ) != AI_NO_ERROR )
			{
				break;
			}
			// Put segment to module manger
			if ( ( liRet = GetModuleMgr()->PutUsingSegment( lpoSegmentNode ) ) != AI_NO_ERROR )
			{
				break;
			}
		}
		else
		{
			// Put segment to module manger
			if ( ( liRet = GetModuleMgr()->PutFreeSegment( lpoSegmentNode ) ) != AI_NO_ERROR )
			{
				break;
			}
		}
		
		return liRet;
	
	}while( false );
	
	GetModuleMgr()->DestorySegment( lpoSegmentNode );
	
	return liRet;
}

int32_t clsGFQueue::AddSegmentTry( const stSegmentServerInfo& aoSegmentInfo )
{
	// lock scope
	AISmartLock loLock(coLock);
	
	int32_t liQueueCapacity = 0;

	TModuleHandle ltModuleHandle = GetModuleMgr()->SearchModule( aoSegmentInfo.ccModuleName );
	if ( ltModuleHandle != INVALID_MODULE_HANDLE )
	{
		return AI_ERROR_DUP_MODULE;
	}

	if ( aoSegmentInfo.ciQueueGlobalID > 0 && strlen(aoSegmentInfo.ccQueueName) > 0 )
	{
	    //TO DO
		//permissive
		if ( !IsPermision( aoSegmentInfo.ccQueueName, liQueueCapacity ) )
		{
			return AI_ERRRO_INVALID_QUEUE;
		}
    }
    
    return AI_NO_ERROR;
}

int32_t clsGFQueue::DelModule( const char* apsModuleName )
{
	// lock scope
	AISmartLock loLock(coLock);
	
	return GetModuleMgr()->DelModule( apsModuleName );
}

int32_t clsGFQueue::HeartBeat( const char* apsModuleName )
{
	// lock scope
	AISmartLock loLock(coLock);
	
	TModuleHandle ltModuleHandle = GetModuleMgr()->SearchModule(apsModuleName);
	if ( ltModuleHandle == INVALID_MODULE_HANDLE )
	{
		return AI_ERROR_NO_FOUND_MODULE;
	}

	GetModuleMgr()->UpdateHeartBeatTime(ltModuleHandle);
    
    return AI_NO_ERROR;
}

void clsGFQueue::CheckModule( int32_t aiTimeout )
{
    int32_t liCurrTime = time(NULL);
    
	// lock scope
	AISmartLock loLock(coLock);
	
	TModuleMgr::TIterator liIt = GetModuleMgr()->ModuleBegin();
	for ( ; liIt != GetModuleMgr()->ModuleEnd(); )
	{
		TModuleHandle loModuleHandle = (TModuleHandle)liIt;
		liIt++;
	    if ( liCurrTime - GetModuleMgr()->GetLastHeartBeatTime(loModuleHandle) > aiTimeout )
		{
			GetModuleMgr()->MasterErrorObserver( loModuleHandle, AI_ERROR_HEARTBEAT_TIMEOUT );
		}
	}
}

// for queue
int32_t clsGFQueue::CreateQueue( const char* apsQueueName, TQueueHandle& atQueueHandle )
{
	int32_t liQueueCapacity = ciQueueDefaultCapacity;
	
	{
		//TO DO
		//permissive
		if ( !IsPermision( apsQueueName, liQueueCapacity ) )
		{
			return AI_ERRRO_INVALID_QUEUE;
		}
	}
	
	atQueueHandle = coQueueMgr.CreateQueue( apsQueueName, liQueueCapacity );
	
	return AI_NO_ERROR;
}

int32_t clsGFQueue::DelQueue( const char* apsQueueName )
{
	// Lock scope
	AISmartLock loLock(coLock);
	
	return coQueueMgr.DelQueue( apsQueueName );
}

int32_t clsGFQueue::GetWriteSegment( TQueueHandle atQueueHandle, stSegmentNode** apoSegmentNode )
{
    int32_t liRet = AI_NO_ERROR;
    
    assert( atQueueHandle != INVALID_QUEUE_HANDLE );
    
    // Repeat twice, if can't get it return error, it's exception
	for ( int liIt = 0; liIt < 2; liIt++ )
	{
		if ( ( liRet = coQueueMgr.GetWriteSegment( atQueueHandle, apoSegmentNode ) ) == AI_NO_ERROR )
		{
			return liRet;
		}
		
		// Oh, exception here
		if ( liIt >= 1 ) break;
		
		// Push segment for empty queue
		if ( ( liRet = coQueueMgr.PushSegment( atQueueHandle ) ) != AI_NO_ERROR )
		{
			return liRet;
		}
		// Yes, push success, repeat get writeable segment
	}
	
	return liRet;
}

int32_t clsGFQueue::GetWriteSegment( const char* apsQueueName, stSegmentNode** apoSegmentNode )
{
    int32_t      liRet         = AI_NO_ERROR;
    TQueueHandle ltQueueHandle = coQueueMgr.SearchQueue( apsQueueName );
    
	if ( ltQueueHandle == INVALID_QUEUE_HANDLE )
	{
		// Unexisted and create it
		if ( ( liRet = CreateQueue( apsQueueName, ltQueueHandle ) ) != AI_NO_ERROR )
		{
			return liRet;
		}
	}
	
	return GetWriteSegment( ltQueueHandle, apoSegmentNode );
}

int32_t clsGFQueue::GetWriteSegment( const char* apsQueueName, stSegmentClientInfo& aoSegmentInfo )
{
	int32_t liRet = AI_NO_ERROR;
	
	// Lock scope
	AISmartLock loLock(coLock);
	
	stSegmentNode* lpoSegmentNode = NULL;
	if ( ( liRet = GetWriteSegment( apsQueueName, &lpoSegmentNode ) ) == AI_NO_ERROR )
	{
	    aoSegmentInfo.ciSegmentID     = coQueueMgr.GetSegmentID( lpoSegmentNode );
	    aoSegmentInfo.ciQueueGlobalID = coQueueMgr.GetQueueGlobalID( lpoSegmentNode );
		aoSegmentInfo.ciQueueID       = coQueueMgr.GetQueueID( lpoSegmentNode );
		aoSegmentInfo.ciModuleID      = GetModuleMgr()->GetModuleID( lpoSegmentNode );
		StringCopy( aoSegmentInfo.ccIpAddr, GetModuleMgr()->GetModuleIpAddr( lpoSegmentNode ), AI_IP_ADDR_LEN );
		aoSegmentInfo.ciPort = GetModuleMgr()->GetModulePort( lpoSegmentNode );
		return liRet;
	}
	
	return liRet;
}

int32_t clsGFQueue::GetReadSegment( const char* apsQueueName, stSegmentClientInfo& aoSegmentInfo )
{
	// Lock scope
	AISmartLock loLock(coLock);
	
	int32_t liRet = AI_NO_ERROR;
	
	TQueueHandle ltHandle = coQueueMgr.SearchQueue( apsQueueName );
	if ( ltHandle == INVALID_QUEUE_HANDLE )
	{
		return AI_ERROR_NO_FOUND_QUEUE;
	}
	
	stSegmentNode* lpoSegmentNode = NULL;
	if ( ( liRet = coQueueMgr.GetReadSegment( ltHandle, &lpoSegmentNode ) ) == AI_NO_ERROR )
	{
		aoSegmentInfo.ciSegmentID     = coQueueMgr.GetSegmentID( lpoSegmentNode );
		aoSegmentInfo.ciQueueGlobalID = coQueueMgr.GetQueueGlobalID( lpoSegmentNode );
		aoSegmentInfo.ciQueueID       = coQueueMgr.GetQueueID( lpoSegmentNode );
		aoSegmentInfo.ciModuleID      = GetModuleMgr()->GetModuleID( lpoSegmentNode );
		StringCopy( aoSegmentInfo.ccIpAddr, GetModuleMgr()->GetModuleIpAddr( lpoSegmentNode ), AI_IP_ADDR_LEN );
		aoSegmentInfo.ciPort = GetModuleMgr()->GetModulePort( lpoSegmentNode );
	}
	
	return liRet;
}

int32_t clsGFQueue::WriteSegmentFail( 
    const char* apsQueueName, int32_t aiSegmentID, int32_t aiModuleID, int32_t aiRetcode, int32_t* apiBehave )
{
	int32_t        liRet          = AI_NO_ERROR;
	int32_t        liBehave       = TModuleMgr::SG_RETRY;
	stSegmentNode* lpoSegmentNode = NULL;
	
	// Lock scope
	AISmartLock loLock(coLock);

	if ( ( liRet = GetWriteSegment( apsQueueName, &lpoSegmentNode ) ) != AI_NO_ERROR )
	{
	    return liRet;
	}
	
	// Save QueueHandle
	TQueueHandle lptQueueHandle = clsQueueMgr::GetQueueHandle(lpoSegmentNode);

	TModuleHandle ltModuleHandle = INVALID_MODULE_HANDLE;
	// Sorry, this error have processed before, return AI_NO_ERROR
	if ( coQueueMgr.GetSegmentID( lpoSegmentNode ) == aiSegmentID && 
		 GetModuleMgr()->GetModuleID( lpoSegmentNode ) == aiModuleID )
	{
		ltModuleHandle = GetModuleMgr()->GetModuleHandle(lpoSegmentNode);
	}
	
	liBehave = GetModuleMgr()->ApiErrorObserver( ltModuleHandle, aiRetcode );
	
	if ( apiBehave != NULL ) *apiBehave = liBehave;

	switch( liBehave )
	{
	    // Return error code to api
	    case TModuleMgr::SG_RETURN:
	    {
	        return AI_NO_ERROR;
	    }
	    // Retry it
	    case TModuleMgr::SG_RETRY:
	    {
	        return AI_NO_ERROR;
	    }
	    default:
	    {
	        // If can't hit that segment retry it
	        if ( ltModuleHandle == INVALID_MODULE_HANDLE )
            {
                // Modify behave 
                if ( apiBehave != NULL ) *apiBehave = TModuleMgr::SG_RETRY;
                return AI_NO_ERROR;
            }
            
            // TModuleMgr::SG_SWITCH
	        return coQueueMgr.PushSegment( lptQueueHandle );
	    }
	}
}

int32_t clsGFQueue::ReadSegmentFail( 
    const char* apsQueueName, int32_t aiSegmentID, int32_t aiModuleID, int32_t aiRetcode, int32_t* apiBehave )
{
	int32_t liRet = AI_NO_ERROR;
	int32_t liBehave = TModuleMgr::SG_RETRY;

	// Lock scope
	AISmartLock loLock(coLock);
	
	// Search queue first
	TQueueHandle ltQueueHandle = coQueueMgr.SearchQueue( apsQueueName );
	if ( ltQueueHandle == INVALID_QUEUE_HANDLE )
	{
		return AI_ERROR_NO_FOUND_QUEUE;
	}
    
    stSegmentNode* lpoSegmentNode = NULL;
    TModuleHandle ltModuleHandle = INVALID_MODULE_HANDLE;
    // Repeat twice, sure lpoSegmentNode valid after ApiErrorObserver done
    for ( int liIt = 0; liIt < 2; liIt++ )
    {	
    	// Get current segment node pointer first
    	if ( ( liRet = coQueueMgr.GetReadSegment( ltQueueHandle, &lpoSegmentNode ) ) != AI_NO_ERROR )
    	{
    		return liRet;
    	}
    	
    	ltModuleHandle = INVALID_MODULE_HANDLE;
    	    
        // Sorry, this error have processed before or module deleted, return AI_NO_ERROR
    	if ( coQueueMgr.GetSegmentID( lpoSegmentNode ) == aiSegmentID && 
    		 GetModuleMgr()->GetModuleID( lpoSegmentNode ) == aiModuleID )
    	{
    		ltModuleHandle = GetModuleMgr()->GetModuleHandle(lpoSegmentNode);
    	}
    	
    	// Do once
    	if ( liIt == 0 )
    	{
    	    // Process module error
    	    liBehave = GetModuleMgr()->ApiErrorObserver( ltModuleHandle, aiRetcode );
    	}	
    }
    
    if ( apiBehave != NULL ) *apiBehave = liBehave;
        
    switch( liBehave )
	{
	    // Return error code to api
	    case TModuleMgr::SG_RETURN:
	    {
	        return AI_NO_ERROR;
	    }
	    // Retry it
	    case TModuleMgr::SG_RETRY:
	    {
	        return AI_NO_ERROR;
	    }
	    default:
	    {
	        // If can't hit that segment retry it
	        if ( ltModuleHandle == INVALID_MODULE_HANDLE )
            {
                // Modify behave 
                if ( apiBehave != NULL ) *apiBehave = TModuleMgr::SG_RETRY;
                return AI_NO_ERROR;
            }
            
            // liBehave == TModuleMgr::SG_SWITCH
	        if ( coQueueMgr.IsLastSegment(lpoSegmentNode) && aiRetcode == AI_ERROR_EMPTY_OF_SEGMENT )
	        {
	            return AI_ERROR_END_OF_QUEUE;
	        }
	    
            // TModuleMgr::SG_SWITCH
	        return coQueueMgr.PopSegment( ltQueueHandle );
	    }
	}
}

int32_t clsGFQueue::GetQueueStat( const char* apsQueueName, size_t* apiSize, size_t* apiCapacity )
{
	// lock scope
	AISmartLock loLock(coLock);
	
	TQueueHandle ltQueueHandle = coQueueMgr.SearchQueue( apsQueueName );
	if ( ltQueueHandle == INVALID_QUEUE_HANDLE )
	{
		return AI_ERROR_NO_FOUND_QUEUE;
	}
	
	if ( apiSize != NULL ) *apiSize = coQueueMgr.GetQueuePreciseSize(ltQueueHandle);
	if ( apiCapacity != NULL ) *apiCapacity = coQueueMgr.GetQueueCapacity(ltQueueHandle);
	
	return AI_NO_ERROR;
}

size_t clsGFQueue::GetQueueCount()
{
	return coQueueMgr.GetQueueCount();
}

size_t clsGFQueue::GetModuleCount()
{
	return GetModuleMgr()->GetModuleCount();
}

size_t clsGFQueue::GetSegmentCount()
{
	// lock scope
	AISmartLock loLock(coLock);
	
	size_t liSum = 0;
	TModuleMgr::TIterator liIt = GetModuleMgr()->ModuleBegin();
	for ( ; liIt != GetModuleMgr()->ModuleEnd(); liIt++ )
	{
		liSum += GetModuleMgr()->GetSegmentCount( (TModuleHandle)liIt );
	}
	return liSum;
}

size_t clsGFQueue::GetUsingSegmentCount()
{
	// lock scope
	AISmartLock loLock(coLock);
	
	size_t liSum = 0;
	TModuleMgr::TIterator liIt = GetModuleMgr()->ModuleBegin();
	for ( ; liIt != GetModuleMgr()->ModuleEnd(); liIt++ )
	{
		liSum += GetModuleMgr()->GetUsingSegmentCount( (TModuleHandle)liIt );
	}
	
	return liSum;
}

size_t clsGFQueue::GetFreeSegmentCount()
{
	// lock scope
	AISmartLock loLock(coLock);
	
	size_t liSum = 0;
	TModuleMgr::TIterator liIt = GetModuleMgr()->ModuleBegin();
	for ( ; liIt != GetModuleMgr()->ModuleEnd(); liIt++ )
	{
		liSum += GetModuleMgr()->GetFreeSegmentCount( (TModuleHandle)liIt );
	}
	
	return liSum;
}

void clsGFQueue::GetAllQueueInfo( AI_STD::vector<stQueueInfo>& aoQueueInfos )
{
    size_t  liQueueCapacity = 0;
    int32_t liIndex = 0;
    
    {
    	// lock scope
    	AISmartLock loLock(coLock);
    	
    	aoQueueInfos.resize( coQueueMgr.GetQueueCount() );
    	
    	clsQueueMgr::TIterator liIt = coQueueMgr.QueueBegin();
    	for ( ; liIt != coQueueMgr.QueueEnd(); liIt++ )
    	{
    		StringCopy( aoQueueInfos[liIndex].ccQueueName, liIt->ccQueueName, AI_MAX_NAME_LEN );
    		aoQueueInfos[liIndex].ciSegmentCnt      = liIt->ciSegmentCnt;
    		aoQueueInfos[liIndex++].ciQueueCapacity = liIt->ciQueueCapacity;
    	}
    }
    
    for( size_t liIt = 0; liIt < aoQueueInfos.size(); liIt++  )
    {
        GetQueueStat( aoQueueInfos[liIt].ccQueueName, &(aoQueueInfos[liIt].ciQueueSize), &(liQueueCapacity) );
    }
}

void clsGFQueue::GetAllModuleInfo( AI_STD::vector<stModuleInfo>& aoModuleInfos )
{
    int32_t      liIndex = 0;
    
	// lock scope
	AISmartLock loLock(coLock);

    aoModuleInfos.resize( GetModuleMgr()->GetModuleCount() );
	
    TModuleMgr::TIterator liIt = GetModuleMgr()->ModuleBegin();
	for ( ; liIt != GetModuleMgr()->ModuleEnd(); liIt++ )
	{
		StringCopy( aoModuleInfos[liIndex].ccModuleName, liIt->ccModuleName, AI_MAX_NAME_LEN );
		aoModuleInfos[liIndex].ciUsingSegmentCnt = liIt->ciUsingSegmentCnt;
		aoModuleInfos[liIndex].ciFreeSegmentCnt  = liIt->ciFreeSegmentCnt;
		aoModuleInfos[liIndex++].ciSegmentCnt    = liIt->ciUsingSegmentCnt + liIt->ciFreeSegmentCnt;
	}
}
	
//end namespace
AI_GFQ_NAMESPACE_END
