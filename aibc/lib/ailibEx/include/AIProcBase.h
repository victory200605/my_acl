#ifndef  __AILIBEX__AIPROCBASE_H__
#define  __AILIBEX__AIPROCBASE_H__

#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////
/////
#define  AP_MODULE_MAX_COUNT   (64)
#define  AP_MODULE_MAX_SIZE    (64)
#define  AP_VERSION_MAX_SIZE   (32)
#define  AP_MODIFY_TIME_SIZE   (24)

////////////////////////////////////////////////////////////////////////////////////
/////
class SetModuleVersion
{
	public:
		SetModuleVersion( char *apcModule, char *apcVer, char *apcTime );
};

////////////////////////////////////////////////////////////////////////////////////
/////
#define  SET_MODULE_VERSION( module, version, time ) \
static SetModuleVersion moSetModVer##module( ""#module, version, time );

////////////////////////////////////////////////////////////////////////////////////
/////
typedef struct ProgOptParam
{
	bool	cbHasVal;
	char	*cpcOptCmd;
	char	*cpcIntroduce;
	char	csCmdValue[128];
} ProgOptParam;

#define  DECLARE_CMDOPT_BEGIN( ) \
	ProgOptParam	moProgOptArray[] = { \
	{ 1, "m", "program log level", {0,} }, \
	{ 0, "v", "program version & usage", {0,} }, \
	{ 0, "d", "program startup daemon mode", {0,} }, 

#define  DECLARE_CMDOPT( Cmd, HasVal, Intro ) \
	{ HasVal, Cmd, Intro, {0,} }, 

#define  DECLARE_CMDOPT_END( ) \
	{ 0, NULL, NULL, {0,} } };


////////////////////////////////////////////////////////////////////////////////////
/////
extern ProgOptParam moProgOptArray[];

////////////////////////////////////////////////////////////////////////////////////
/////
extern void gvPrintVersion( void );
extern void gvPrintOptUsage( void );
extern void gvParseCmdLines( int argc, char *argv[] );

extern int giHasOptArg( const char *apcOptCmd );
extern int giGetOptArg( const char *apcOptCmd, const int aiDefault );
extern int giGetOptArg( const char *apcOptCmd, char *apcValue, const int aiValueMaxSize );

///end namespace
AIBC_NAMESPACE_END

#endif   /* __AIPROCBASE_H__ */
