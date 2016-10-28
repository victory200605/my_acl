//=============================================================================
/**
* \file    AIGetOpt.cpp
* \brief
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: AIGetOpt.cpp,v 1.3 2009/03/02 13:39:51 daizh Exp $
*
* History
* 2008.01.23 first release by DZH
*/
//=============================================================================

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "AIGetOpt.h"

///start namespace
AIBC_NAMESPACE_START

AIGetOpt::AIGetOpt( int aiAgrc, 
				   char* apsAgrv[],
				   char* apsOptString,
				   int   aiSkipArg,
				   bool  abReportErrors,
				   int   aiOrdering,
				   int   aiLongOnly ) :
	cbOptErr(abReportErrors),
	ciArgc(aiAgrc),
	cpcArgv(apsAgrv),
	cpcNextChar(0),
	ciOptInd(0),
	ciOptOpt(0),
	cpcOptArg(0),
	ciOrdering(aiOrdering),
	ciLongOnly(aiLongOnly),
	ciHasColon(0),
	ciNonoptStart(ciOptInd),
	ciNonoptEnd(ciOptInd),
	cpoLongOption(NULL),
	cpoLongOpts(NULL)
{
	::snprintf( csOptString, sizeof(csOptString), "%s", apsOptString );

	// Now, check to see if any or the following were passed at
	// the begining of optstring: '+' same as POSIXLY_CORRECT;
	// '-' turns off POSIXLY_CORRECT; or ':' which signifies we
	// should return ':' if a parameter is missing for an option.
	// We use a loop here, since a combination of "{+|-}:" in any
	// order should be legal.
	int liDone  = 0;
	int liOffset = 0;
	while ( !liDone )
	{
		switch (csOptString[liOffset++])
		{
		case '+':
			this->ciOrdering = REQUIRE_ORDER;
			break;
		case '-':
			this->ciOrdering = RETURN_IN_ORDER;
			break;
		case ':':
			this->ciHasColon = 1;
			break;
		default:
			// Quit as soon as we see something else...
			liDone = 1;
			break;
		}
	}

	// Initialize other argument
	::memset( csLastOption, 0, sizeof(csLastOption) );
}

AIGetOpt::~AIGetOpt( void )
{
	stGetOptLongOption* lpoOption = cpoLongOpts;
	while( lpoOption != NULL )
	{
		stGetOptLongOption* lpoTmp = lpoOption;
		lpoOption = lpoOption->cpoNext;
		delete lpoTmp;
	}
}

int AIGetOpt::operator () ( void )
{
	// First of all, make sure we reinitialize any pointers..
	this->ciOptOpt = 0;
	this->cpoLongOption = 0;

	if ( this->cpcArgv == 0 )
	{
		// It can happen, e.g., on VxWorks.
		this->ciOptInd = 0;
		return -1;
	}

	// We check this because we can string short options together if the
	// preceding one doesn't take an argument.
	if ( this->cpcNextChar == 0 || *this->cpcNextChar == '\0' )
	{
		int liRetval = this->NextChar();
		if ( liRetval != 0 )
		{
			return liRetval;
		}
	}

	if ( ( (this->cpcArgv[this->ciOptInd][0] == '-')
		&& (this->cpcArgv[this->ciOptInd][1] == '-') ) || this->ciLongOnly )
		return this->LongOptionI ();

	return this->ShortOption();
}

int AIGetOpt::Argc( void ) const
{
	return this->ciArgc;
}

char** AIGetOpt::Argv( void ) const
{
	return this->cpcArgv;
}

char* AIGetOpt::OptArg( void ) const
{
	return this->cpcOptArg;
}

int AIGetOpt::OptOpt( void ) const
{
	return this->ciOptOpt;
}

int& AIGetOpt::OptInd( void )
{
	return this->ciOptInd;
}

const char* AIGetOpt::LongOption( void ) const
{
	if ( this->cpoLongOption )
		return this->cpoLongOption->cpsName;
	return 0;
}

const char* AIGetOpt::LastOption( void ) const
{
	return this->csLastOption;
}

void AIGetOpt::LastOption( const char* apsLastOption, size_t aiLen )
{
	::strncpy( csLastOption, apsLastOption, aiLen );
	csLastOption[aiLen] = '\0';
}

void AIGetOpt::LastOption( char acOpt )
{
	csLastOption[0] = acOpt;
	csLastOption[1] = '\0';
}

int AIGetOpt::Permute( void )
{
	if( this->ciNonoptStart != this->ciNonoptEnd
		&& this->ciNonoptStart != this->ciOptInd )
	{
		this->PermuteArgs();
	}

	this->ciNonoptStart = this->ciOptInd;

	// Skip over args untill we find the next option.
	while( this->ciOptInd < this->ciArgc
		&& (this->cpcArgv[this->ciOptInd][0] != '-'
		|| this->cpcArgv[this->ciOptInd][1] == '\0') )
	{
		this->ciOptInd++;
	}

	// Got an option, so mark this as the end of the non options.
	this->ciNonoptEnd = this->ciOptInd;

	if ( this->ciOptInd != this->ciArgc
		&& ::strcmp( this->cpcArgv[this->ciOptInd], "--" ) == 0 )
	{
		// We found the marker for the end of the options.
		this->ciOptInd++;

		if ( this->ciNonoptStart != this->ciNonoptEnd
			&& this->ciNonoptEnd != this->ciOptInd )
		{
			this->PermuteArgs ();
		}
	}

	if ( this->ciOptInd == this->ciArgc )
	{
		if ( this->ciNonoptStart != this->ciNonoptEnd )
		{
			this->ciOptInd = this->ciNonoptStart;
		}
		return EOF;
	}

	return 0;
}

const char* AIGetOpt::OptString( void ) const
{
	return this->csOptString;
}

AIGetOpt::stGetOptLongOption::stGetOptLongOption (
	const char* apsName,
	int aiHasArg,
	int aiVal )
	:cpsName( strdup(apsName) ),
	ciHasArg(aiHasArg),
	ciVal(aiVal)
{}

AIGetOpt::stGetOptLongOption::~stGetOptLongOption( void )
{
	::free( this->cpsName );
}

int AIGetOpt::LongOption( const char* apsName,
						 OPTION_ARG_MODE aiHasArg )
{
	return this->LongOption( apsName, 0, aiHasArg );
}

int AIGetOpt::LongOption( const char* apsName,
						 int aiShortOption,
						 OPTION_ARG_MODE aiHasArg )
{
	// We only allow valid alpha-numeric characters as short options.
	// If short_options is not a valid alpha-numeric, we can still return it
	// when the long option is found, but won't allow the caller to pass it on
	// the command line (how could they?).  The special case is 0, but since
	// we always return it, we let the caller worry about that.
	if ( aiShortOption > 0 &&
		aiShortOption < 256 &&
		isalnum( static_cast<char>(aiShortOption) ) != 0)
	{
		// If the short_option already exists, make sure it matches, otherwise
		// add it.
		char* lpS = 0;
		if ( ( lpS = const_cast<char*>(
			::strchr(csOptString, aiShortOption) ) ) != 0)
		{
			// Short option exists, so verify the argument options
			if ( lpS[1] == ':')
			{
				if ( lpS[2] == ':')
				{
					if ( aiHasArg != ARG_OPTIONAL )
					{
						if ( this->cbOptErr )
						{
							fprintf( stderr, 
								"Existing short option '%c' takes "
								"optional argument; adding %s "
								"requires ARG_OPTIONAL\n",aiShortOption, apsName );
						}
						return -1;
					}
				}
				else
					if ( aiHasArg != ARG_REQUIRED )
					{
						if ( this->cbOptErr )
						{
							fprintf( stderr, 
								"Existing short option '%c' takes "
								"optional argument; adding %s "
								"requires ARG_REQUIRED\n",aiShortOption, apsName );
						}
						return -1;
					}
			}
			else if ( aiHasArg != NO_ARG )
			{
				if ( this->cbOptErr )
				{
					fprintf( stderr, 
						"Existing short option '%c' does not "
						"accept an argument; adding %s "
						"requires NO_ARG\n",aiShortOption, apsName );
				}
				return -1;
			}
		}
		else
		{
			// Didn't find short option, so add it...
			char liTmp[2] = { aiShortOption, 0 };
			::strcat( csOptString, liTmp );

			if ( aiHasArg == ARG_REQUIRED )
				::strcat( csOptString, ":" );
			else if (aiHasArg == ARG_OPTIONAL)
				::strcat( csOptString, "::" );
		}
	}

	stGetOptLongOption* lpoOption =
		new stGetOptLongOption( apsName, aiHasArg, aiShortOption );

	if ( !lpoOption ) return -1;

	// Add to array
	if ( cpoLongOpts != NULL )
	{
		lpoOption->cpoNext = cpoLongOpts;
		cpoLongOpts = lpoOption;
	}
	else
	{
		lpoOption->cpoNext = NULL;
		cpoLongOpts = lpoOption;
	}

	return 0;
}

void AIGetOpt::PermuteArgs( void )
{
	unsigned long liCycleLen, liNcycle, liNnonOpts, liNopts;
	unsigned long liOptEnd = this->ciOptInd;
	int liCStart, liPos = 0;
	char* lpsSwap = 0;

	liNnonOpts = this->ciNonoptEnd - this->ciNonoptStart;
	liNopts    = liOptEnd - this->ciNonoptEnd;
	liNcycle   = this->Gcb(liNnonOpts, liNopts);
	liCycleLen = (liOptEnd - this->ciNonoptStart) / liNcycle;

	this->ciOptInd = this->ciOptInd - liNnonOpts;

	for( unsigned long i = 0; i < liNcycle; i++ )
	{
		liCStart = this->ciNonoptEnd + i;
		liPos = liCStart;
		for ( unsigned long j = 0; j < liCycleLen; j++ )
		{
			if ( liPos >= this->ciNonoptEnd )
				liPos -= liNnonOpts;
			else
				liPos += liNopts;
			lpsSwap = this->cpcArgv[liPos];

			((char**)this->cpcArgv)[liPos] = this->cpcArgv[liCStart];

			((char**)this->cpcArgv)[liCStart] = lpsSwap;
		}
	}
}

int AIGetOpt::NextChar ( void )
{
	if ( this->ciOrdering == PERMUTE_ARGS )
	{
		if ( this->Permute () == EOF )
			return EOF;
	}

	// Update scanning pointer.
	if (this->ciOptInd >= this->ciArgc )
	{
		// We're done...
		this->cpcNextChar = 0;
		return EOF;
	}
	else if ( *( this->cpcNextChar = this->cpcArgv[this->ciOptInd] ) != '-'
		|| this->cpcNextChar[1] == '\0' )
	{
		// We didn't get an option.
		if ( this->ciOrdering == REQUIRE_ORDER
			|| this->ciOrdering == PERMUTE_ARGS ) 
		{
			// If we permuted or require the options to be in order, we're done.
			return EOF;
		}

		// It must be RETURN_IN_ORDER...
		this->cpcOptArg = this->cpcArgv[this->ciOptInd++];
		this->cpcNextChar = 0;
		return 1;
	}
	else if ( this->cpcNextChar[1] != 0
		&& *++this->cpcNextChar == '-'
		&& this->cpcNextChar[1] == 0 )
	{
		// Found "--" so we're done...
		++this->ciOptInd;
		this->cpcNextChar = 0;
		return EOF;
	}

	// If it's a long option, and we allow long options advance NextChar.
	if ( *this->cpcNextChar == '-' && this->cpoLongOpts != NULL )
		this->cpcNextChar++;

	return 0;
}

int AIGetOpt::LongOptionI( void )
{
	stGetOptLongOption* lpoLongOpt;
	char* lpS = this->cpcNextChar;
	int liHits  = 0;
	int liExact = 0;
	stGetOptLongOption* lpoFound = 0;
	size_t liIndFound = 0;

	// Advance to the end of the long option name so we can use
	// it to get the length for a string compare.
	while ( *lpS && *lpS != '=' )
		lpS++;

	size_t liLen = lpS - this->cpcNextChar;
	// set LastOption to NextChar, up to the '='.
	this->LastOption( this->cpcNextChar, liLen );

	size_t liOptionIndex = 0;
	for( lpoLongOpt = this->cpoLongOpts; 
		lpoLongOpt != NULL ; lpoLongOpt = lpoLongOpt->cpoNext )
	{
		if ( !::strncmp( lpoLongOpt->cpsName, this->cpcNextChar, liLen ) )
		{
			// Got at least a partial match.
			lpoFound = lpoLongOpt;
			liIndFound = liOptionIndex;
			liHits += 1;
			if ( liLen == ::strlen(lpoLongOpt->cpsName) )
			{
				// And in fact, it's an exact match, so let's use it.
				liExact = 1;
				break;
			}
		}
		liOptionIndex++;
	}

	if( (liHits > 1) && !liExact )
	{
		// Great, we found a match, but unfortunately we found more than
		// one and it wasn't exact.
		if ( this->cbOptErr )
		{
			fprintf( stderr, "%s: option `%s' is ambiguous\n",
				this->cpcArgv[0], this->cpcArgv[this->ciOptInd] );
		}
		this->cpcNextChar = 0;
		this->ciOptInd++;
		return '?';
	}

	if( lpoFound != 0 )
	{
		// Okay, we found a good one (either a single hit or an exact match).
		// OptionIndex = indfound;
		this->ciOptInd++;
		if( *lpS )
		{
			// s must point to '=' which means there's an argument (well
			// close enough).
			if( lpoFound->ciHasArg != NO_ARG)
			{
				// Good, we want an argument and here it is.
				this->cpcOptArg = ++lpS;
			}
			else
			{
				// Whoops, we've got what looks like an argument, but we
				// don't want one.
				if ( this->cbOptErr )
				{
					fprintf( stderr, 
						"%s: long option `--%s' doesn't allow an argument\n",
						this->cpcArgv[0], lpoFound->cpsName );
				}
				// The spec doesn't cover this, so we keep going and the program
				// doesn't know we ignored an argument if OptErr is off!!!
			}
		}
		else if( lpoFound->ciHasArg == ARG_REQUIRED )
		{
			// s didn't help us, but we need an argument. Note that
			// optional arguments for long options must use the "=" syntax,
			// so we won't get here in that case.
			if ( this->ciOptInd < this->ciArgc )
			{
				// We still have some elements left, so use the next one.
				this->cpcOptArg = this->cpcArgv[this->ciOptInd++];
			}
			else
			{
				// All out of elements, so we have to punt...
				if ( this->cbOptErr )
				{
					fprintf( stderr, 
						"%s: long option '--%s' requires an argument\n",
						this->cpcArgv[0], lpoFound->cpsName );
				}
				this->cpcNextChar = 0;
				this->ciOptOpt = lpoFound->ciVal;   // Remember matching short equiv
				return this->ciHasColon ? ':' : '?';
			}
		}
		this->cpcNextChar = 0;
		this->cpoLongOption = lpoFound;
		// Since val_ has to be either a valid short option or 0, this works
		// great.  If the user really wants to know if a long option was passed.
		this->ciOptOpt = lpoFound->ciVal;
		return lpoFound->ciVal;
	}
	if ( !this->ciLongOnly || this->cpcArgv[this->ciOptInd][1] == '-'
		|| ::strchr( this->csOptString, *this->cpcNextChar ) == NULL )
	{
		// Okay, we couldn't find a long option.  If it isn't long_only (which
		// means try the long first, and if not found try the short) or a long
		// signature was passed, e.g. "--", or it's not a short (not sure when
		// this could happen) it's an error.
		if ( this->cbOptErr )
		{
			fprintf( stderr, 
				"%s: illegal long option '--%s'\n", this->cpcArgv[0], this->cpcNextChar );
		}
		this->cpcNextChar = 0;
		this->ciOptInd++;
		return '?';
	}
	return this->ShortOption();
}

int AIGetOpt::ShortOption( void )
{
	/* Look at and handle the next option-character.  */
	char lcOpt = *this->cpcNextChar++;
	// Set LastOption to lcOpt
	this->LastOption(lcOpt);

	char* lpcOli = ::strchr( this->csOptString, lcOpt );

	/* Increment `optind' when we start to process its last character.  */
	if ( *this->cpcNextChar == '\0' )
		++this->ciOptInd;

	if ( lpcOli == 0 || lcOpt == ':' )
	{
		if ( this->cbOptErr )
		{
			fprintf( stderr, "%s: illegal short option -- %c\n", this->cpcArgv[0], lcOpt );
		}
		return '?';
	}

	if ( lcOpt == 'W' && lpcOli[1] == ';' )
	{
		if ( this->cpcNextChar[0] == 0 )
			this->cpcNextChar = this->cpcArgv[this->ciOptInd];

		return this->LongOptionI();
	}

	this->ciOptOpt = lpcOli[0];      // Remember the option that matched
	if ( lpcOli[1] == ':' )
	{
		if ( lpcOli[2] == ':' )
		{
			// Takes an optional argument, and since short option args must
			// must follow directly in the same argument, a NULL Nextchar
			// means we didn't get one.
			if ( *this->cpcNextChar != '\0' )
			{
				this->cpcOptArg = this->cpcNextChar;
				this->ciOptInd++;
			}
			else
			{
				this->cpcOptArg = 0;
			}
			this->cpcNextChar = 0;
		}
		else
		{
			// Takes a required argument.
			if ( *this->cpcNextChar != '\0' )
			{
				// Found argument in same argv-element.
				this->cpcOptArg = this->cpcNextChar;
				this->ciOptInd++;
			}
			else if ( this->ciOptInd == this->ciArgc )
			{
				// Ran out of arguments before finding required argument.
				if ( this->cbOptErr )
				{
					fprintf( stderr,
						"%s: short option requires an argument -- %c\n", this->cpcArgv[0], lcOpt );
				}
				lcOpt = this->ciHasColon ? ':' : '?';
			}
			else
			{
				// Use the next argv-element as the argument.
				this->cpcOptArg = this->cpcArgv[this->ciOptInd++];
			}
			this->cpcNextChar = 0;
		}
	}
	return lcOpt;
}

unsigned long AIGetOpt::Gcb( unsigned long aiX, unsigned long aiY )
{
	while( aiY != 0 )
	{
		unsigned long liR = aiX % aiY;
		aiX = aiY;
		aiY = liR;
	}

	return aiX;
}

///end namespace
AIBC_NAMESPACE_END
