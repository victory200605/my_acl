/**
 * @file Regex.h
 */
#ifndef ACL_REGEX_H
#define ACL_REGEX_H

#include "acl/Utility.h"
#include "apl/regex.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"

ACL_NAMESPACE_START

/**
 * This class used to save the substring which is the result that used a regular expression to match a string.
 */
 
//CRegex forward declare
class CRegex;

/**
 * class CRegexMatchResult
 */
class CRegexMatchResult
{
    friend class CRegex;
    
private:
    class CItem
    {
    public:
        CItem( char const* apcSubStr, apl_size_t aiLen );
  
        /**
         * This function used to get the string in the CItem object.
         * @return a char type pointer of the string that save in CItem object.
         */      
        char const* GetStr(void);

        /**This function used to get the length of the string in CItem object.
         * @return the length of the string in CItem object.
         */
        apl_size_t GetLength(void);

    private:
        std::string moSubStr;
    };

public:
    ~CRegexMatchResult(void);

    /**
     * This operator used to get the substring which in the specific location of CRegexMatchResult object.
     * @param [in]    aiN    specifies the location.
     * @return the cication of the CItem object the save the substring.
     */    
    CItem& operator[] ( apl_size_t aiN );

    /** 
     * This function used to get the number of the substring in the object.
     * @return the number of the substring in the object.
     */  
    apl_size_t GetSize(void);

protected:
    void AddMatchResult(char const* apcSubStr, apl_size_t aiLen); 
    
    void Reset(void);
    
protected:
    std::vector<CItem*> moItems;
};

////////////////////////////////////////////////////////////////////////////////////////

/**
 * This class used to get the substring which find in a string through a regular expression.
 */
class CRegex
{
public:
    //Compile Option

    /** This value used in the second argument of compile() function, it specifies to use extend regular expression.
     */  
    static const apl_int_t OPT_EXTENDED = APL_REG_EXTENDED;

    /**
     * This value used in the second argument of compile() function, it specifies to ignore case in match.
     */
    static const apl_int_t OPT_ICASE = APL_REG_ICASE;

    /**
     * This value used in the second argument of compile() function, it specifies to save no substring in match.
     */
    static const apl_int_t OPT_NOSUB = APL_REG_NOSUB;

    /**
     * This value used in the second argument of compile() function, it sepcifies to identify newline.
     */
    static const apl_int_t OPT_NEWLINE = APL_REG_NEWLINE;
    
    //Match & Search Option

    /**
     * This value used in the second argument of Match() or Search() function, it specifies that the character
     * '^' is taken as special character, do not match the begin of string.
     */ 
    static const apl_int_t OPT_NOTBOL = APL_REG_NOTBOL;

    /**
     * This value used in the second argument of Match() or Search() function, it specifies that the character
     * '$' is taken as special character, do not match the end of string.
     */
    static const apl_int_t OPT_NOTEOL = APL_REG_NOTEOL;
    
    typedef CRegexMatchResult ResultType;

public:
    CRegex(void);
    
    ~CRegex(void);

    /**
     * This function used to compile the regular expression before used Match() or Search() function.
     * @param [in]    apcPattern    the regular expression.
     * @param [in]    aiOpt         the compile option, its value can be taken one or more option of OPT_EXTENDED,
     *                              OPT_ICASE, OPT_NOSUB, OPT_NEWLINE that used OR connected.
     * @retval 0 successful.
     * @retval -1 fail.
     */    
    apl_int_t Compile( char const* apcPattern, apl_int_t aiOpt = OPT_EXTENDED );
   
    /**
     * This function used to find substring that match the regular expression in apcInput.
     * @param[in]    apcInput    the string that used to find substring.
     * @param[in]    aiOpt       match option, its value can be taken one or more option of OPT_NOTBOL,
     *                           OPT_NOTEOL that used OR connected.
     * @retval true Specifies that in apcInput can find substring that match the regular expression.
     * @retval false Specifies that in apcInput can not find substring that match the regular expression.
     */ 
    bool Match( char const* apcInput, apl_int_t aiOpt = 0 );
    
    /**
     * This function used to find substring that match the regular expression in apcInput, it is similar to Match() function
     * expect this function will return the substring as result.
     * @param [in]    apcInput    the string that used to find substring.
     * @param [out]   apoMatch    a CRegexMatchResult object that used to save the substring that match successfully.
     * @param [in]    aiOpt       search option, its value can be taken one or more option of OPT_NOTBOL,
     *                            OPT_NITEOL that used OR connected.
     * @retval >=0 search successful.
     * @retval -1 search error.
     */ 
    apl_ssize_t Search( char const* apcInput, ResultType* apoMatch, apl_int_t aiOpt = 0 );

    /**
     * This function used to check if the regular expression has already compiled.
     * @retval true regular expression has compiled.
     * @retval false Regular expression has not compiled.
     */
    bool IsReady(void) const;

protected:
    bool mbIsCompiled;
    
    apl_regex_t moRegex;
};

ACL_NAMESPACE_END

#endif//ACL_REGEX_H
