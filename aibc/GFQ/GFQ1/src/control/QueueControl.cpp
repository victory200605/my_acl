
#include <unistd.h>
#include <stdio.h>
#include <regex.h>
#include "gfq/GFQUtility.h"
#include "QueueControl.h"

#if defined(AI_OS_AIX)
    //Make compiler happy on AIX env
#   include "AIProcBase.h"
    ProgOptParam moProgOptArray[1];
#endif

#define QUEUE_CONTROL_LOG "QueueInfo.log"

//-----------------------------clsQueueInfo-----------------------------------//
clsQueueConfig::stQueueInfo::stQueueInfo() : ciQueueCapacity(0)
{
	memset( ccQueueName, 0, AI_MAX_NAME_LEN );
}

const char* clsQueueConfig::stQueueInfo::GetKey()
{
	return ccQueueName;
}

int clsQueueConfig::stQueueInfo::Compare( const char* apR )
{
	return strcmp( ccQueueName, apR );
}

//-------------------------stQueueInfoFormat---------------------------------//
clsQueueConfig::stQueueInfoPattern::stQueueInfoPattern() : ciQueueCapacity(0)
{
    memset( ccQueueName, 0, AI_MAX_NAME_LEN );
    memset( &coPatternChild, 0, sizeof(LISTHEAD) );
}

clsQueueConfig* clsQueueConfig::cpoInstance = NULL;

clsQueueConfig::clsQueueConfig()
{
    ai_init_list_head( &coPatternHead );
    
    AIInitIniHandler();
    AIInitLOGHandler();
}

clsQueueConfig::~clsQueueConfig()
{
    FreeDualLink( &coPatternHead );
    
    AICloseLOGHandler();
    AICloseIniHandler();
}

clsQueueConfig* clsQueueConfig::Instance()
{
	if ( cpoInstance == NULL )
	{
		AI_NEW_ASSERT( cpoInstance, clsQueueConfig );
	}
	
	return cpoInstance;
}
	
int clsQueueConfig::Load( const char* apsConfig )
{
	char lcLine[4096];
	char lcErrorBuff[1024];
	char lcQueueName[AI_MAX_NAME_LEN];
 	int  liQueueCapacity = 0;
 	int  liLineNum = 0;
 	clsHashTable<const char*, stQueueInfo> loQueueInfoList;
 	LISTHEAD loTmpHead;
 	ai_init_list_head( &loTmpHead );
	
 	FILE* lpFile =fopen( apsConfig, "r" );
 	if ( lpFile == NULL )
 	{
 	    AIWriteLOG( QUEUE_CONTROL_LOG, AILOG_LEVEL_ERROR, "Load queue info fail, open file %s fail, MSG:%s\n", apsConfig, strerror(errno) );
 	    fclose(lpFile);
 		return -1;
 	}
    
 	while( fgets( lcLine, sizeof(lcLine), lpFile ) )
 	{
 	    liLineNum++;
 	    
 		if( lcLine[0] == '#' )  continue;
 		    
 		if( strlen( lcLine ) == 0 ) continue;

 		if( sscanf( lcLine, "%s %d", lcQueueName, &liQueueCapacity ) < 2 
 		    || liQueueCapacity <= 0 
 		    || strlen(lcQueueName) > AI_MAX_NAME_LEN - 2 )
 		{
 		    AIWriteLOG( QUEUE_CONTROL_LOG, AILOG_LEVEL_ERROR, "Load queue info fail, invalid format, [Line Num=%d]\n", liLineNum );
 		    FreeDualLink( &loTmpHead );
 		    fclose(lpFile);
 			return -1;
 		}

 		if ( this->IsPatternName( lcQueueName ) )
 		{
 		    char lcTmpQueueName[AI_MAX_NAME_LEN];
 		    snprintf( lcTmpQueueName, AI_MAX_NAME_LEN - 1, "^%s$", lcQueueName );
 		    
 		    if ( this->PatternMatch( lcTmpQueueName, "test", lcErrorBuff, sizeof(lcErrorBuff) ) < 0 )
 		    {
 		        AIWriteLOG( QUEUE_CONTROL_LOG, AILOG_LEVEL_ERROR, "Load queue info fail, invalid format, MSG:%s, [Line Num=%d]\n", lcErrorBuff, liLineNum );
 		        FreeDualLink( &loTmpHead );
 		        fclose(lpFile);
 		        return -1;
 		    }
 		    
 		    stQueueInfoPattern* lpQueueInfoPattern = NULL;
 		    AI_NEW_ASSERT( lpQueueInfoPattern, stQueueInfoPattern );
 		    
 		    StringCopy( lpQueueInfoPattern->ccQueueName, lcTmpQueueName, AI_MAX_NAME_LEN );
 		    lpQueueInfoPattern->ciQueueCapacity = liQueueCapacity;
 		    ai_list_add_tail( &lpQueueInfoPattern->coPatternChild, &loTmpHead );
 		    continue;
 		}
 		
 		clsHashTable<const char*, stQueueInfo>::iterator loIt = loQueueInfoList.Find( lcQueueName );
 		if ( loIt != coQueueInfoList.End() )
 		{
 		    AIWriteLOG( QUEUE_CONTROL_LOG, AILOG_LEVEL_ERROR, "Load queue info fail, queue name dual, [Queue name=%s],[Line Num=%d]\n", lcQueueName, liLineNum );
 		    FreeDualLink( &loTmpHead );
 		    fclose(lpFile);
 		    return -1;
 		}
 		
 		stQueueInfo* lpoQueueInfo = clsHashTable<const char*, stQueueInfo>::CreateNode();
 		
 		lpoQueueInfo->ciQueueCapacity = liQueueCapacity;
 		StringCopy( lpoQueueInfo->ccQueueName, lcQueueName, AI_MAX_NAME_LEN );
 		
 		loQueueInfoList.Insert(lpoQueueInfo);
 	}
 	
 	coQueueInfoList.Swap( loQueueInfoList );
 	ai_list_swap( &coPatternHead, &loTmpHead );
 	
 	FreeDualLink( &loTmpHead );
 	fclose(lpFile);
 	
 	return 0;
}

const clsQueueConfig::stQueueInfo* clsQueueConfig::Find( const char* apsQueueName )
{
	return coQueueInfoList.Find( apsQueueName );
}

const clsQueueConfig::stQueueInfoPattern* clsQueueConfig::Match( const char* apsName )
{
    char lcErrorBuff[1024];
    LISTHEAD* lpCurr = coPatternHead.cpNext;
    for( ; lpCurr != NULL && lpCurr != &coPatternHead; lpCurr = lpCurr->cpNext )
    {
        stQueueInfoPattern* lpTmp = AI_GET_STRUCT_PTR( lpCurr, stQueueInfoPattern, coPatternChild );
        if ( PatternMatch( lpTmp->ccQueueName, apsName, lcErrorBuff, sizeof(lcErrorBuff) ) == 1 )
 		{
 		    return lpTmp;
 		}
    }
    
    return NULL;
}

bool clsQueueConfig::IsPatternName( const char* apsName )
{
    while( *apsName != '\0' )
    {
        if ( ( *apsName < '0' || *apsName > '9' ) &&
             ( *apsName < 'a' || *apsName > 'z' ) &&
             ( *apsName < 'A' || *apsName > 'Z' ) &&
             ( *apsName != '_' ) )
        {
            return true;
        }
        apsName++;
    }
    
    return false;
}

int clsQueueConfig::PatternMatch( const char* apsPattern, const char* apsString, char* apsErrorBuff, size_t aiSize )
{
    int        liRet = 0;
	regex_t    ltReg;
    
    liRet = regcomp( &ltReg, apsPattern, REG_EXTENDED|REG_NOSUB );
    if ( liRet != 0 )
    {
        liRet = regerror( liRet, &ltReg, apsErrorBuff, aiSize );
        return -1;
    }
    
    liRet = regexec( &ltReg, apsString, (size_t)0, NULL, 0 );
    regerror( liRet, &ltReg, apsErrorBuff, sizeof(apsErrorBuff) );
    //free regex
    regfree(&ltReg);
    if ( liRet == REG_NOMATCH )
    {
        return 0;
    }
    else if ( liRet != 0 )
    {
        return -1;
    }
    
    return 1;
}

void clsQueueConfig::FreeDualLink( LISTHEAD* apoListHead )
{
    if( apoListHead->cpNext == NULL || apoListHead->cpPrev == NULL ) return;
        
    LISTHEAD* lpHead = NULL;
    while( !ai_list_is_empty(apoListHead) )
    {
        ai_list_del_head( lpHead, apoListHead );
        stQueueInfoPattern* lpTmp = AI_GET_STRUCT_PTR( lpHead, stQueueInfoPattern, coPatternChild );
        AI_DELETE(lpTmp);
    }
}

int LoadQueueInfo( void )
{
	return clsQueueConfig::Instance()->Load( CONFIG_FILE );
}

int IsPermision( const char* apsQueueName, int* apiQueueCapacity )
{
    const clsQueueConfig::stQueueInfo* lpQueueInfo = clsQueueConfig::Instance()->Find(apsQueueName);
    if ( lpQueueInfo != NULL )
	{
	    if ( apiQueueCapacity != NULL ) *apiQueueCapacity = lpQueueInfo->ciQueueCapacity;
	    
		return 0;
	}

	const clsQueueConfig::stQueueInfoPattern* lpQueueInfoPattern = clsQueueConfig::Instance()->Match(apsQueueName);
	if ( lpQueueInfoPattern != NULL )
	{
	    if ( apiQueueCapacity != NULL ) *apiQueueCapacity = lpQueueInfoPattern->ciQueueCapacity;
		
		return 0;
	}
	
	return -1;
}
