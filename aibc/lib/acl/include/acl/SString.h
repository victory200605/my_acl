#ifndef ACL_SPLIT_H
#define ACL_SPLIT_H

#include "acl/Utility.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"

ACL_NAMESPACE_START

/////////////////////////////////////////////  Tokenizer /////////////////////////////////////////

/**
 * This class used to split string. The string wanted to split may be changed according to the input argment.
 */
class CTokenizer
{
public:
    CTokenizer(void);
        
    /**
     * This function used to construct the CTokenizer object.
     * @param [in]    apcDelimiter             a pointer pointed to the string that uses as spliter.
     * @param [in]    abIsSkipBlankField       specifies the result shall contain the null chararcter or not.
     */ 
    CTokenizer( char const* apcDelimiter, bool abIsSkipBlankField = true );

    /**
     * This function used to specify the spliter.
     * @param [in]    apcDelimiter             a pointer pointed to the string that uses as spliter.
     * @param [in]    abIsSkipBlankField       specifies the result shall contain the spliter or not.
     */
    void Delimiter ( char const* apcDelimiter, bool abIsSkipBlankField = true );

    /**
     * This function used to set the preserve area of the string, the string in the preserve area should not split
     * event it contains spliters.Every string in preserve area shall return as a substring.
     * @param [in]    acStart    the character of the start of the preserve.
     * @param [in]    acStop     the character of the end of the preserve.
     * @param [in]    abIsStrip  a flag that specifies if the edge of the preserve area shall remain.
     * @retval 0    successful.
     */
    apl_int_t Preserve( char acStart, char acStop, bool abIsStrip = true );

    ~CTokenizer(void);
    /**
     * This function used to split the string pointed by apcInput.
     * @param [in]    apcInput    a pointer pointed to the sting that wanted to be splited.
     * @retval >=0    This value specifies the number of parts the string is splited.
     */     
    apl_ssize_t Parse( char* apcInput );

    /**
     * This function shall get the number of parts the string which is the first argment of the function Parse() splited.
     * @retval >=0    This value specifies the number of parts the string is splited.
     */
    apl_size_t GetSize(void) const;

    /**
     * This function shall get the specific part of the string which has been splited.
     * @param [in]    aiN    specifies the location of the substrig which wanted to be got.
     * @retval null    Get substring fail.
     * @retval >0      Get substring successfully, the return value is a pointer pointed to the substring.
     */
    char const* GetField( apl_size_t aiN );

protected:
    bool IsDelimiter( char const* apcInput ) const;
    
    bool IsPreserve( char acStart, char& acStop, bool& abIsStrip ) const;
    
    void GetToken(apl_size_t aiFieldID, char* apcFirst, apl_size_t aiLen);
    
    CTokenizer& operator = ( CTokenizer const& );
    
    CTokenizer( CTokenizer const& ) {};
    
private:
    struct CPreserve
    {
        char mcStart;
        char mcStop;
        bool mbIsStrip;
    };
    
    std::string moDelimiter;
    
    bool mbIsSkipBlankField;
    
    std::vector<CPreserve> moPreserves;
    
    std::vector<char*> moFields;
    apl_size_t muFieldCount;
}; 

/////////////////////////////////////////////// Spliter //////////////////////////////////////////////
/**
 * This class used to split string according to the delimiter.The string which wanted to be splited shall
 * not be changed after splited.
 */
class CSpliter
{
public:
    CSpliter(void);
    /**
     * CSpliter object constructor.
     * @param [in]    apcDelimiter        a pointer pointed to the string which used as delimiter.
     * @param [in]    abIsSkipBlankField  a flag specifies if the null character shall save. 
     */
    CSpliter( char const* apcDelimiter, bool abIsSkipBlankField = true );

    /**
     * This function used to specify the delimiter.
     * @param [in]    apcDelimiter        a pointer pointed to the string which used as delimiter.
     * @param [in]    abIsSkipBlankField  a flag specifies if the null character shall save. 
     */
    void Delimiter ( char const* apcDelimiter, bool abIsSkipBlankField = true );

    /**
     * This function used to set the preserve area.The area shall identify by the begin and end character.
     * @param [in]    acStart    specifies the start character of the preserve area.
     * @param [in]    acStop     specifies the end character of the preserve area.
     * @param [in]    abIsStrip  specifies if the area's edge shall remain or not.
     * @retval 0    successful.
     */
    apl_int_t Preserve( char acStart, char acStop, bool abIsStrip = true );

    ~CSpliter(void);

    /**
     * This function shall split the string pointed by apcInput.
     * @param [in]    apcInput    a pointer pointed to the string which wanted to be splited.
     * @retval >=0    This value specifies the number of parts the string splited.
     */
    apl_ssize_t Parse( char const* apcInput );

    /**
     * Overloaded function.
     * @param [in]    aoInput   the string which wanted to be splited.
     * @retval >=0    This value specifies the number of parts the string splited.
     */
    apl_ssize_t Parse( std::string const& aoInput );

    /**
     * This function shall get the number of parts the string which is the first argment of the function Parse() splited.
     * @retval >=0     This value specifies the number of parts the string is splited.
     */
    apl_size_t GetSize(void) const;

    /**
     * This function shall get the specific part of the string which has been splited.
     * @param [in]    aiN    specifies the location of the substrig which wanted to be got.
     * @retval null    Get substring fail.
     * @retval >0      Get substring successfully, the return value is a pointer pointed to the substring.
     */
    char const* GetField( apl_size_t aiN );

protected:
    bool IsDelimiter( char const* apcInput ) const;
    
    bool IsPreserve( char acStart, char& acStop, bool& abIsStrip ) const;
    
    void GetToken( apl_size_t aiFieldID, char const* apcFirst, apl_size_t aiLen );
    
    CSpliter& operator = ( CSpliter const& );
    
    CSpliter( CSpliter const& );
    
private:
    struct CPreserve
    {
        char mcStart;
        char mcStop;
        bool mbIsStrip;
    };
    
    std::string moDelimiter;
    
    bool mbIsSkipBlankField;
    
    std::vector<CPreserve> moPreserves;
    
    std::vector<std::string> moFields;
    apl_size_t muFieldCount;
}; 

///////////////////////////////////////////// FixSpliter //////////////////////////////////////////////
/**
 * This class used to split string according to fix width.
 */
class CFixSpliter
{
public:

    /**
     * CFixSpliter object constructor.
     * @param [in]    apiFixWidth   a pointer pointed to the array which specifies the width of every part
     *                              that the string splited.
     * @param [in]    aiFieldCnt    the length of the array pointed by apiFixWidth.
     */ 
    CFixSpliter( apl_size_t* apiFixWidth, apl_size_t aiFieldCnt ); 

    ~CFixSpliter(void);

    /**
     * This function shall split the string pointed by apcInput.
     * @param [in]    apcInput    a pointer pointed to the string which wanted to be splited.
     * @retval >=0    the return value specifies the number of parts the string splited.
     * @retval -1     fail.
     */
    apl_ssize_t Parse( char const* apcInput );

    /**
     * Overloaded function.
     * @param [in]    aoInput   the string which wanted to be splited.
     * @retval >=0    the return value specifies the number of parts the string splited.
     * @retval -1     Fail.
     */
    apl_ssize_t Parse( std::string const& aoInput );

    /**
     * This function shall get the number of parts the string which is the first argment of the function Parse() splited.
     * @retval >=0    The return value specifies the number of parts the string is splited.
     */
    apl_size_t GetSize(void) const;

    /**
     * This function shall get the specific part of the string which has been splited.
     * @param [in]    aiN    specifies the location of the substrig which wanted to be got.
     * @retval null    Get substring fail.
     * @retval >0      Get substring successfully, the return value is a pointer pointed to the substring.
     */
    char const* GetField( apl_size_t aiN);

protected:
    apl_ssize_t Parse( char const* apcInput, apl_size_t aiLen );
    
    CFixSpliter( const CFixSpliter& );
    
    CFixSpliter& operator = ( const CFixSpliter& );
    
private:
    std::vector<apl_size_t>  moFieldWidths;
    apl_size_t muTotalLen;
    
    std::vector<std::string> moFields;
};

ACL_NAMESPACE_END

#endif //ACL_SPLIT_H
