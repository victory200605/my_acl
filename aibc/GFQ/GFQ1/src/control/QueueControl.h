
#ifndef AI_QUEUE_CONTROL_H_2008
#define AI_QUEUE_CONTROL_H_2008

#include "gfq/GFQUtility.h"

#if defined(_AI_LIB_EX_OLD)
#   define CONFIG_FILE "../config/QueueInfo.ini"
#else
#   define CONFIG_FILE "../../config/QueueInfo.ini"
#endif

using namespace gfq;

class clsQueueConfig
{
public:
	struct stQueueInfo
	{
		stQueueInfo();
		
		char ccQueueName[AI_MAX_NAME_LEN];
		int ciQueueCapacity;
		
		const char* GetKey();
		int Compare( const char* apR );
	};
	
	// Define for wildcard
	struct stQueueInfoPattern
	{
	    stQueueInfoPattern();
		
		char ccQueueName[AI_MAX_NAME_LEN];
		int ciQueueCapacity;
		
		LISTHEAD coPatternChild;
	};
	
public:
	static clsQueueConfig* Instance();
	
	~clsQueueConfig();
	
	// Load Queue info
	int Load( const char* apsConfig );
	
	// Search Queue info by queue name
	const stQueueInfo* Find( const char* apsQueueName );
	
	// Match Queue name
	const stQueueInfoPattern* Match( const char* apsName );
	
protected:
    bool IsPatternName( const char* apsName );
    int  PatternMatch( const char* apsPattern, const char* apsString, char* apsErrorBuff, size_t aiSize );
    void FreeDualLink( LISTHEAD* apoListHead );
    
protected:
	static clsQueueConfig* cpoInstance;
	clsQueueConfig();
	
protected:
	clsHashTable<const char*, stQueueInfo> coQueueInfoList;
	
	LISTHEAD coPatternHead;
};

extern "C" {
int LoadQueueInfo( void );
int IsPermision( const char* apsQueueName, int* apiQueueCapacity );
}

#endif // AI_QUEUE_CONTROL_H_2008
