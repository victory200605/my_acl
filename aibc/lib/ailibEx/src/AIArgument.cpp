
#include <stdio.h>
#include <string.h>
#include "AIArgument.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////// *Argument* ///////////////////////////////////////
AIArgument::AIArgument( int argc, char* argv[] ) : 
    ciArgc(argc),cpcArgv(argv)
{
    // Initialize
    memset( &this->coaOption,    0,   sizeof(this->coaOption) );
    memset( this->csOptString,   0,   sizeof(csOptString) );
}

void AIArgument::AddOption( unsigned char acOpt, bool abHasValue, const char* apcLongOpt, const char* apcDesc )
{
    char   lsStrOpt[2] = { acOpt, 0 };

    this->coaOption[acOpt].cbIsSetted     = 1;
    this->coaOption[acOpt].cbHasValue     = abHasValue ? 1 : 0;
    this->coaOption[acOpt].cpcLongOption  = apcLongOpt;
    this->coaOption[acOpt].cpcDescription = apcDesc;
    this->coaOption[acOpt].cpcValue       = NULL;

    ::strcat( csOptString, lsStrOpt );
    if ( this->coaOption[acOpt].cbHasValue == 1 )
    {
        ::strcat( csOptString, ":" );
    }
}

int AIArgument::Parser( void )
{
    // Set option
    AIGetOpt loOpt( this->ciArgc, this->cpcArgv, this->csOptString, 0, true, AIGetOpt::RETURN_IN_ORDER );
    // Set long option
    for ( int liN = 0; liN < MAX_ARG; liN++ )
    {
        stOption* lpoOption = &this->coaOption[liN];
        if ( lpoOption->cbIsSetted == 1 
            && lpoOption->cpcLongOption != NULL )
        {
            AIGetOpt::OPTION_ARG_MODE liHasValue = 
                (lpoOption->cbHasValue == 1? AIGetOpt::ARG_REQUIRED : AIGetOpt::NO_ARG);
            loOpt.LongOption(lpoOption->cpcLongOption, liN, liHasValue);
        }
    }

    // Get option
    int liOpt;
    while( (liOpt = loOpt()) != -1 )
    {
        if ( liOpt == ':' || liOpt == '?' || liOpt > MAX_ARG  )
        {
            return -1;
        }

        stOption* lpoOption = &this->coaOption[liOpt];
        lpoOption->cbIsHitted = 1;
        if ( lpoOption->cbHasValue == 1 )
        {
            lpoOption->cpcValue = loOpt.OptArg();
        }
    }

    return 0;
}

void AIArgument::PrintUsage()
{
    printf( "Usage : %s\n", this->GetProcessName() );
    this->PrintOption();
}

void AIArgument::PrintOption( void )
{
    char lsValue[20]      = {0};
    char lsLongOption[20] = {0};
    char lsDescriptin[64] = {0};
    
    for ( int liN = 0; liN < MAX_ARG; liN++ )
    {
        if ( this->coaOption[liN].cbIsSetted == 1 )
        {
            if ( this->coaOption[liN].cpcLongOption != NULL )
            {
                if ( this->coaOption[liN].cbHasValue == 1 )
                {
                    snprintf( lsValue, sizeof(lsValue), "%s <arg>", this->coaOption[liN].cpcLongOption );
                    snprintf( lsLongOption, sizeof(lsLongOption), "--%-15s", lsValue );
                }
                else
                {
                    snprintf( lsLongOption, sizeof(lsLongOption), "--%-15s", this->coaOption[liN].cpcLongOption );
                }
            }
            else
            {
                snprintf( lsLongOption, sizeof(lsLongOption), "  %-15s", "" );
            }

            if ( this->coaOption[liN].cpcDescription != NULL )
            {
                snprintf( lsDescriptin, sizeof(lsDescriptin), "%s", this->coaOption[liN].cpcDescription );
            }
            else
            {
                snprintf( lsDescriptin, sizeof(lsDescriptin), "%s", "<no description>" );
            }

            printf( " -%c %s : %s\n", liN, lsLongOption, lsDescriptin );
        }
    }
}

bool AIArgument::IsHasOpt( const char* apcOption ) const
{
    for ( int liN = 0; liN < ciArgc; liN++ )
    {
        if ( ::strcmp( cpcArgv[liN], apcOption ) == 0 )
        {
            return true;
        }
    }
    return false;
}

bool AIArgument::IsHasOpt( char apcOption ) const
{
    return this->coaOption[static_cast<int>(apcOption)].cbIsHitted == 1 ? true : false;
}

int AIArgument::GetIntValue( char apcOption, int aiDefault )
{
    const char* lpcValue = this->GetValue( apcOption );
    if ( lpcValue != NULL )
    {
        return atoi(lpcValue);
    }
    else
    {
        return aiDefault;
    }
}

bool AIArgument::GetStrValue( char apcOption, char* apcValue, size_t aiSize, const char* apcDefault )
{
    const char* lpcValue = this->GetValue( apcOption );
    if ( lpcValue != NULL )
    {
        ::snprintf( apcValue, aiSize, "%s", lpcValue );
        return true;
    }
    else
    {
        ::snprintf( apcValue, aiSize, "%s", ( apcDefault==NULL? "" : apcDefault ) );
        return false;
    }
}

const char* AIArgument::GetValue( char apcOption ) const
{
    if ( this->coaOption[static_cast<int>(apcOption)].cbIsHitted == 1 )
    {
        return this->coaOption[static_cast<int>(apcOption)].cpcValue;
    }

    return NULL;
}

const char* AIArgument::GetProcessName() const
{
    if ( cpcArgv != NULL )
    {
        const char* lpcNext = strrchr( cpcArgv[0], '/' );
        if ( lpcNext == NULL )
            lpcNext = cpcArgv[0];
        else
            ++lpcNext;
        
        return lpcNext;
    }
    else
    {
        return "";
    }
}

///end namespace
AIBC_NAMESPACE_END
