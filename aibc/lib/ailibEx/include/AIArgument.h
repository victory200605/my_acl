
#ifndef __AILIB_ARGUMENT_H__
#define __AILIB_ARGUMENT_H__

#include "AIGetOpt.h"

///start namespace
AIBC_NAMESPACE_START

class AIArgument
{
public:
	static const int MAX_ARG = 'z' + 1;

protected:
	struct stOption
	{
		char        cbIsSetted;
		char        cbIsHitted;
		char        cbHasValue;
		const char* cpcLongOption;
		const char* cpcDescription;
		const char* cpcValue;
	};

public:
	AIArgument( int argc, char* argv[] );

	void AddOption( unsigned char acOpt, bool abHasValue, const char* apcLongOpt = NULL, const char* apcDesc = NULL );
	int Parser( void );

	void PrintUsage( void );
    void PrintOption( void );
    
	bool IsHasOpt( const char* apcOption ) const;
	bool IsHasOpt( char apcOption ) const;
	
	int  GetIntValue( char apcOption, int aiDefault );
	bool GetStrValue( char apcOption, char* apcValue, size_t aiSize, const char* apcDefault = NULL );
	const char* GetValue( char apcOption ) const;
	const char* GetProcessName() const;
	
	int Argc( void ) const           { return ciArgc; }

    char** Argv( void ) const        { return cpcArgv; }

protected:
	char      csOptString[AI_MAX_ARG_LEN];
	stOption  coaOption[MAX_ARG];
	int       ciArgc;
	char**    cpcArgv;
};

///end namespace
AIBC_NAMESPACE_END

#endif //__AILIB_ARGUMENT_H__
