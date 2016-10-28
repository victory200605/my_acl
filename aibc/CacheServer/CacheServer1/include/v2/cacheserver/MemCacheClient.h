
#ifndef	__AI_MCACHECLIENT_API_H__
#define	__AI_MCACHECLIENT_API_H__

#include "CacheUtility.h"

/////////////////////////////////////////////////////////////////////////
class clsMemCacheClient
{
public:
    clsMemCacheClient(void);
    
    ~clsMemCacheClient(void);
    
	int Connect( const char* apcServerIp, int aiPort, int aiConnCnt = 1 );
	void Close();
	
	int Add( const char* apcKey, const char* apcValue );
	
    int Put( const char* apcKey, const char* apcValue );

	int Get( const char* apcKey, AIBC::AIChunkEx& aoValue );
	
	int Del( const char* apcKey, AIBC::AIChunkEx& aoValue );
	
// attribute
protected:
    class clsMemCacheClientImpl* mpoImpl;
};

#endif //__AI_MCACHECLIENT_API_H__
