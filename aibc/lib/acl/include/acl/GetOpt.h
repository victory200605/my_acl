/**
 * @file GetOpt.h
 */
#ifndef ACL_GETOPT_H
#define ACL_GETOPT_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

/**
 * class CGetOpt
 */
class CGetOpt
{
public:
    /**
     *  enum 
     *  Mutually exclusive ordering values.
     */
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

    /**
     * option of argu enum
     * Mutually exclusive option argument mode used by long options.
     */
    enum OPTION_ARG_MODE
    {
         /**
          * NO_ARG
          *  -Doesn't take an argument.
          */
         NO_ARG = 0,

         /**
          * ARG_REQUIRED
          * - Requires an argument, same as passing ":" after a short option character in @a optstring.
          */
         ARG_REQUIRED = 1,

         /**
          * ARG_OPTIONAL
          * - Argument is optional, same as passing "::" after a short option character in @a optstring.
          */
         ARG_OPTIONAL = 2
    };
    
    enum
    {
        MAX_ARG_LEN = 64
    };
    
public:
    /**
     * Constructor
     *
     * @param [in] aiAgrc    the number of Agrc
     * @param [in] apcAgrv   the Agrv
     * @param [in] apcOptString    the OptionString
     * @param [in] aiSkipArg       skip the number of Agrc
     * @param [in] abReportErrors  If print the errInfo
     *     - true.    print the errInfo
     *     - false.   don't print the errInfo
     * @param [in] aiOrdering      Ordering Option
     * @param [in] aiLongOnly      If only the long option
     */ 
    CGetOpt(
        apl_int_t aiAgrc, 
        char* apcAgrv[],
        char const* apcOptString = "",
        apl_int_t   aiSkipArg = 0,
        bool  abReportErrors = false,
        apl_int_t   aiOrdering = PERMUTE_ARGS,
        apl_int_t   aiLongOnly = 0 );
    
    /**
     * Deconstructor
     */
    ~CGetOpt(void);

    /**
     * Overloaded operator()
     * handle the Agrv iterator
     *
     * @retval x the option of OptionString
     * @retval ? Invalid option
     * @retval : lose option
     * @retval 0 the default long option
     * @retval -1 EOF
     * @retval 1 illegal option
     */
    apl_int_t operator () (void);
    
    /**
     * Adds a long option with no corresponding short option.
     * If the @a name option is seen, @c operator() returns 0.
     *
     * @param apcName       The long option to add.
     * @param aeHasArg      Defines the argument requirements for
     *                      the new option.
     *
     * @retval 0  Success
     * @retval -1  The long option can not be added.
     */
    apl_int_t LongOption(
        char const* apcName,
        OPTION_ARG_MODE aeHasArg = NO_ARG );
    
    /**
     * Adds a long option with a corresponding short option.
     *
     * @param apcName          The long option to add.
     * @param aiShortOption    A character, the short option that corresponds
     *                         to @a name.
     * @param aeHasArg         Defines the argument requirements for
     *                         the new option.  If the short option has already
     *                         been supplied in the @a optstring, @a has_arg
     *                         must match or an error is returned; otherwise, the
     *                         new short option is added to the @a optstring.
     *
     * @retval 0  Success
     * @retval -1  The long option can not be added.
     */
    apl_int_t LongOption(
        char const* apcName,
        apl_int_t aiShortOption,
        OPTION_ARG_MODE aeHasArg = NO_ARG );
     
    /**
     * Get the number of Argv.
     *
     * @retval >=0
     */ 
    apl_int_t Argc(void) const;

    /**
     * Get the Argv.
     *
     * @retval char**
     */
    char** Argv(void) const;

    /**
     * Get the optionArgv.
     *
     * @return a pointer pointed to string
     */
    char* OptArg(void) const;

    /**
     * Get the option of Argument.
     *
     * @retval >=0
     */ 
    apl_int_t OptOpt(void) const;

    /**
     * Get the index of current handle argument.
     *
     * @retval >=0
     */
    apl_int_t& OptInd (void);
    
    /**
     * Get the option of LongOption.
     *
     * @return a pointer pointed to string
     */
    char const* LongOption(void) const;
    
    /**
     * Get the option of last handle argument.
     *
     * @return a pointer pointed to string
     */
    char const* LastOption(void) const;
    
    /**
     * This is handy to verify that calls to
     * long_option added short options as expected.
     *
     * @return the @a optstring.
     */
    char const* OptString(void) const;

protected:
    /**
     * @class ACE_Get_Opt_Long_Option  
     * The class is for internal use in the ACE_Get_Opt class, and is inaccessible to users.
     */
    struct CGetOptLongOption
    {
        /* ctor */
        CGetOptLongOption(
            char const* apcName,
            apl_int_t aiHasArg,
            apl_int_t aiVal = 0 );

        /* Dtor. */
        ~CGetOptLongOption(void);

        /* Long option name. */
        char* cpcName;

        /* Contains value for <OPTION_ARG_MODE>. */
        apl_int_t ciHasArg;

        /* Contains a valid short option character or zero if it doesn't
        have a corresponding short option.  It can also contain a
        non-printable value that cannot be passed to <optstring> but
        will be returned by <operator()>.  This is handy for
        simplifying long option handling, see tests/Get_Opt_Test.cpp
        for an example of this technique. */
        apl_int_t ciVal;
        
        CGetOptLongOption* cpoNext;
    };
  
protected:
    apl_int_t Permute(void);
    
    void PermuteArgs(void);
    
    void LastOption( char acOpt );
    void LastOption( char const* apcLastOption, size_t aiLen );
        
    /* Updates NextChar. */
    apl_int_t NextChar(void);

    /* Handles long options. */
    apl_int_t LongOptionI(void);

    /* Handles short options. */
    apl_int_t ShortOption(void);
    
    unsigned long Gcb( unsigned long aiX, unsigned long aiY );

protected:
	bool mbOptErr;

    apl_int_t miArgc;
    
    char** mppcArgv;

    char macOptString[MAX_ARG_LEN];

    char* mpcNextChar;

    apl_int_t miOptInd;

    apl_int_t miOptOpt;

    char* mpcOptArg;
    
    apl_int_t miOrdering;
    
    /* Treat all options as long options. */
    apl_int_t miLongOnly;
  
    apl_int_t miHasColon;
    
    /* Index of the first non-option <argv>-element found (only valid
     when permuting). */
    apl_int_t miNonoptStart;

    /* Index of the <argv>-element following the last non-option element
     (only valid when permuting). */
    apl_int_t miNonoptEnd;
  
    char macLastOption[MAX_ARG_LEN];

    CGetOptLongOption* mpoLongOption;
    
    CGetOptLongOption* mpoLongOpts;
};

ACL_NAMESPACE_END

#endif //__AILIB_GETOPT_H__
