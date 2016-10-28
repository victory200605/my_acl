//=============================================================================
/**
 * \file    AIGetOpt.h
 * \brief
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: AIGetOpt.h,v 1.2 2009/03/02 13:39:50 daizh Exp $
 *
 * History
 * 2008.01.23 first release by DZH
 */
//=============================================================================

#ifndef __AILIB_GETOPT_H__
#define __AILIB_GETOPT_H__

#include <stdlib.h>
#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

#define AI_MAX_ARG_LEN 64

class AIGetOpt
{
public:
    /// Mutually exclusive ordering values.
    enum
    {
         /**
          * REQUIRE_ORDER means that processing stops and @c EOF is
          * returned as soon as a non-option argument is found. @c OptInd()
          * will return the index of the next @a argv element so the program
          * can continue processing the rest of the @a argv elements.
          */
         REQUIRE_ORDER = 1,

         /**
          * PERMUTE_ARGS means the @a argv elements are reordered dynamically
          * (permuted) so that all options appear first. When the elements are
          * permuted, the order of the options and the following arguments are
          * maintained. When the last option has been processed, @c EOF is
          * returned and @c OptInd() returns the index into the next non-option
          * element.
          */
         PERMUTE_ARGS = 2,

         /**
          * RETURN_IN_ORDER means each @a argv element is processed in the
          * order is it seen.  If the element is not recognized as an option, '1'
          * is returned and @c OptArg() refers to the @a argv element found.
          */
         RETURN_IN_ORDER = 3
    };

    /// Mutually exclusive option argument mode used by long options.
    enum OPTION_ARG_MODE
    {
         /// Doesn't take an argument.
         NO_ARG = 0,

         /// Requires an argument, same as passing ":" after a short option
         /// character in @a optstring.
         ARG_REQUIRED = 1,

         /// Argument is optional, same as passing "::" after a short
         /// option character in @a optstring.
         ARG_OPTIONAL = 2
    };
    
public:
    AIGetOpt( int aiAgrc, 
              char* apsAgrv[],
              char* apsOptString = "",
              int   aiSkipArg = 0,
              bool  abReportErrors = false,
              int   aiOrdering = PERMUTE_ARGS,
              int   aiLongOnly = 0 );
    
    ~AIGetOpt( void );

    int operator () ( void );
    
    /// Adds a long option with no corresponding short option.
    /**
     * If the @a name option is seen, @c operator() returns 0.
     *
     * @param apcName       The long option to add.
     * @param aiHasArg      Defines the argument requirements for
     *                      the new option.
     *
     * @retval 0  Success
     * @retval -1 The long option can not be added.
     */
    int LongOption( const char* apcName,
                    OPTION_ARG_MODE aiHasArg = NO_ARG );
    
    /// Adds a long option with a corresponding short option.
    /**
     * @param name          The long option to add.
     * @param short_option  A character, the short option that corresponds
     *                      to @a name.
     * @param has_arg       Defines the argument requirements for
     *                      the new option.  If the short option has already
     *                      been supplied in the @a optstring, @a has_arg
     *                      must match or an error is returned; otherwise, the
     *                      new short option is added to the @a optstring.
     *
     * @retval 0  Success
     * @retval -1 The long option can not be added.
     */
    int LongOption( const char* apcName,
                    int aiShortOption,
                    OPTION_ARG_MODE aiHasArg = NO_ARG );
                 
    int Argc( void ) const;

    char** Argv( void ) const;

    char* OptArg( void ) const;

    int OptOpt( void ) const;

    int& OptInd ( void );
    
    const char* LongOption( void ) const;
    
    const char* LastOption( void ) const;
    
    /// Return the @a optstring.  This is handy to verify that calls to
    /// long_option added short options as expected.
    const char* OptString( void ) const;

protected:
    /**
     * @class ACE_Get_Opt_Long_Option  This class is for internal use
     * in the ACE_Get_Opt class, and is inaccessible to users.
     */
    struct stGetOptLongOption
    {
        /// ctor
        stGetOptLongOption( const char* apsName,
                             int aiHasArg,
                             int aiVal = 0 );

        /// Dtor.
        ~stGetOptLongOption(void);

        /// Long option name.
        char* cpsName;

        /// Contains value for <OPTION_ARG_MODE>.
        int ciHasArg;

        /// Contains a valid short option character or zero if it doesn't
        /// have a corresponding short option.  It can also contain a
        /// non-printable value that cannot be passed to <optstring> but
        /// will be returned by <operator()>.  This is handy for
        /// simplifying long option handling, see tests/Get_Opt_Test.cpp
        /// for an example of this technique.
        int ciVal;
        
        stGetOptLongOption* cpoNext;
    };
  
protected:
    int Permute( void );
    
    void PermuteArgs( void );
    
    void LastOption( char acOpt );
    void LastOption( const char* apsLastOption, size_t aiLen );
        
    /// Updates NextChar.
    int NextChar( void );

    /// Handles long options.
    int LongOptionI( void );

    /// Handles short options.
    int ShortOption( void );
    
    unsigned long Gcb( unsigned long aiX, unsigned long aiY );

protected:
	bool cbOptErr;

    int ciArgc;
    
    char** cpcArgv;

    char csOptString[AI_MAX_ARG_LEN];

    char* cpcNextChar;

    int ciOptInd;

    int ciOptOpt;

    char* cpcOptArg;
    
    int ciOrdering;
    
    /// Treat all options as long options.
    int ciLongOnly;
  
    int ciHasColon;
    
    /// Index of the first non-option <argv>-element found (only valid
    /// when permuting).
    int ciNonoptStart;

    /// Index of the <argv>-element following the last non-option element
    /// (only valid when permuting).
    int ciNonoptEnd;
  
    char csLastOption[AI_MAX_ARG_LEN];

    stGetOptLongOption* cpoLongOption;
    
    stGetOptLongOption* cpoLongOpts;
};

///end namespace
AIBC_NAMESPACE_END

#endif //__AILIB_GETOPT_H__
