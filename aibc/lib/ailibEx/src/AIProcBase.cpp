#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "AIProcBase.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////
/////
static int   giModuleCount = 0;
static char  gsModuleName[AP_MODULE_MAX_COUNT][AP_MODULE_MAX_SIZE];
static char  gsModuleVersion[AP_MODULE_MAX_COUNT][AP_VERSION_MAX_SIZE];
static char  gsModuleLastModify[AP_MODULE_MAX_COUNT][AP_VERSION_MAX_SIZE];

////////////////////////////////////////////////////////////////////////////////////
/////
SetModuleVersion::SetModuleVersion( char *apcModule, char *apcVer, char *apcTime )
{
	if( apcModule && apcVer )
	{
		memset( gsModuleName[giModuleCount], 0, AP_MODULE_MAX_SIZE );
		strncpy( gsModuleName[giModuleCount], apcModule, AP_MODULE_MAX_SIZE-1 );
		
		memset( gsModuleVersion[giModuleCount], 0, AP_VERSION_MAX_SIZE );
		strncpy( gsModuleVersion[giModuleCount], apcVer, AP_VERSION_MAX_SIZE-1 );

		memset( gsModuleLastModify[giModuleCount], 0, AP_MODIFY_TIME_SIZE );
		if(apcTime)  strncpy( gsModuleLastModify[giModuleCount], apcTime, AP_MODIFY_TIME_SIZE-1 );

		giModuleCount += 1;
	}
}

void gvPrintVersion( void )
{
	if( giModuleCount > 0 )
	{
		fprintf( stderr, "\nModule version detail:\n" );

		for( int liIt = 0; liIt < giModuleCount; liIt++ )
		{
			fprintf( stderr, "%2d: %s - %s - %s\n", liIt+1, gsModuleName[liIt], 
				gsModuleVersion[liIt], gsModuleLastModify[liIt] );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
/////
void gvParseCmdLines( int argc, char *argv[] )
{
	int				liOptChar = 0;
	char			lsOptString[64];
	ProgOptParam	*lpoParam = NULL;

	/*
	 * Construct lsOptString variable
	*/
	
	for( lsOptString[0] = (char)0, lpoParam = &(moProgOptArray[0]); lpoParam->cpcOptCmd; lpoParam++ )
	{
		sprintf( lsOptString+strlen(lsOptString), "%c%s", lpoParam->cpcOptCmd[0], \
			((lpoParam->cbHasVal) ? ":" : "") );
	}

	opterr = 0;
	while( (liOptChar = getopt( argc, argv, lsOptString )) != EOF )
	{
		for( lpoParam = &(moProgOptArray[0]); lpoParam->cpcOptCmd; lpoParam++ )
		{
			if( lpoParam->cpcOptCmd[0] == (char)liOptChar )
			{
				if( !lpoParam->cbHasVal )
				{
					strcpy( lpoParam->csCmdValue, "ON" );
				}
				else if( lpoParam->cbHasVal && optarg )
				{
					memset( lpoParam->csCmdValue, 0, sizeof(lpoParam->csCmdValue) );
					strncpy( lpoParam->csCmdValue, optarg, sizeof(lpoParam->csCmdValue)-1 );
				}

				break;
			}
		}
	}
}

void gvPrintOptUsage( void )
{
	ProgOptParam	*lpoParam = NULL;

	for( lpoParam = &(moProgOptArray[0]); lpoParam->cpcOptCmd; lpoParam++ )
	{
		fprintf( stderr, "        -%c:  %s\n", lpoParam->cpcOptCmd[0], \
			(lpoParam->cpcIntroduce?lpoParam->cpcIntroduce:"") );
	}

	fprintf( stderr, "\n" );
}

int giHasOptArg( const char *apcOptCmd )
{
	ProgOptParam	*lpoParam = NULL;

	for( lpoParam = &(moProgOptArray[0]); lpoParam->cpcOptCmd; lpoParam++ )
	{
		if( strcmp( lpoParam->cpcOptCmd, apcOptCmd ) == 0 )
		{
			if( !lpoParam->cbHasVal && strcmp(lpoParam->csCmdValue, "ON") == 0 )
				return  (1);

			break;
		}
	}
	
	return  (0);
}

int giGetOptArg( const char *apcOptCmd, const int aiDefault )
{
	ProgOptParam	*lpoParam = NULL;

	for( lpoParam = &(moProgOptArray[0]); lpoParam->cpcOptCmd; lpoParam++ )
	{
		if( strcmp( lpoParam->cpcOptCmd, apcOptCmd ) == 0 )
		{
			if( lpoParam->cbHasVal && strlen(lpoParam->csCmdValue) > 0 )
			{
				/* NOTE: atoi function limit */
				return  atoi(lpoParam->csCmdValue);
			}

			break;
		}
	}
	
	return  (aiDefault);
}

int giGetOptArg( const char *apcOptCmd, char *apcValue, const int aiValueMaxSize )
{
	ProgOptParam	*lpoParam = NULL;

	for( lpoParam = &(moProgOptArray[0]); lpoParam->cpcOptCmd; lpoParam++ )
	{
		if( strcmp( lpoParam->cpcOptCmd, apcOptCmd ) == 0 )
		{
			if( lpoParam->cbHasVal && strlen(lpoParam->csCmdValue) > 0 )
			{
				if( apcValue && aiValueMaxSize > 0 )
				{
					memset( apcValue, 0, aiValueMaxSize );
					strncpy( apcValue, lpoParam->csCmdValue, aiValueMaxSize-1 );

					return  (1);
				}
			}

			break;
		}
	}
	
	if( apcValue && aiValueMaxSize > 0 )
	{
		apcValue[0] = (char)0;
	}

	return  (0);
}

///end namespace
AIBC_NAMESPACE_END
