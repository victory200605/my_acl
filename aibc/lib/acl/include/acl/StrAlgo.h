
#ifndef ACL_STRALGO_H
#define ACL_STRALGO_H

#include "acl/Utility.h"
#include "acl/stl/string.h"

ACL_NAMESPACE_START

namespace stralgo {

/**
 * This function used to change the upper case in aoInput to lower case.
 * @param [in]    aoInput    the string to be changed.
 */ 
void ToLower( std::string& aoInput );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be changed.
 */
void ToLower( char* apcInput );

/**
 * This function used to change the upper case in aoInput to lower case, and set the result to aoResult.
 * The aoInput string shall not be changed.
 * @param [in]    aoInput    the string that wanted to change.
 * @param [out]   apResult   the result string.
 */
void ToLowerCopy( std::string const& aoInput, std::string& aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be changed.
 * @param [out]   apcBuffer   a pointer pointed to a buffer that used to save result string.
 * @param [in]    auSize      specifies length of the buffer that pointed by apcBuffer.
 */  
void ToLowerCopy( char const* apcInput, char* apcBuff, apl_size_t auSize );

/**
 * This function used to change the lower case in aoInput to upper case.
 * @param [in]    aoInput    the string that wanted to change.
 */
void ToUpper( std::string& aoInput );

/**
 * Overloaded function.
 * @param [in]     apcInput    a pointer pointed to the string that wanted to be changed.
 */
void ToUpper( char* apcInput );

/**
 * This function used to change the lower case in aoInput to upper case, and set the result to aoResult.
 * The aoInput string shall not be changed.
 * @param [in]    aoInput    the string that wanted to be changed.
 * @param [out]   aiResult   the result string.
 */
void ToUpperCopy( std::string const& aoInput, std::string& aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be changed.
 * @param [out]   apcBuff     a pointer pointed to the location that hold the result string.
 * @param [in]    auSize      specifies the length of buffer pointed by apcBuff.
 */
void ToUpperCopy( char const* apcInput, char* apcBuff, apl_size_t auSize );

/**
 * This function used to erase space in beginning and end of a string.
 * @param [in]    aoInput    the string wanted to be erase space that in its beginning and end.
 */
void Trim( std::string& aoInput );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be erase space.
 */
void Trim( char* apcInput );

/**
 * This function used to erase space in beginning and end of a string, and set the result to aoResult.
 * The aoInput shall not be changed.
 * @param [in]    aoInput    the string that wanted to be erased space.
 * @param [out]   aoResult   the string that hold the result.
 */
void TrimCopy( std::string const& aoInput, std::string& aoResult );

/**
 * Overloaded function.
 * @param [in]   apcInput    a pointer pointed to the string that wanted to be earsed space.
 * @param [out]  apcBuff     a pointer pointed to the location that used to hold the result string.
 * @param [in]   auSize      specifies the length of the buffer that pointed by apcBuff.
 */ 
void TrimCopy( char const* apcInput, char* apcBuff, apl_size_t auSize );

/**
 * This function used to erase space at the end of a string.
 * @param [in]    aoInput    the string that wanted to be erase right space.
 */
void TrimRight( std::string& aoInput );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be erase right space.
 */
void TrimRight( char* apcInput );

/**
 * This function used to erase space at the end of a stirng, and set the result to aoResult.
 * The aoInput string shall not be changed.
 * @param [in]    aoInput    the string that wanted to be erased right space.
 * @param [out]   aoResult   the result string.
 */ 
void TrimRightCopy( std::string const& aoInput, std::string& aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be reased right space.
 * @param [out]   apcBuff     a pointer pointed to the location that hold the result string.
 * @param [in]    aiSzie      specifies the length of the buffer that pointed by apcBuff.
 */ 
void TrimRightCopy( char const* apcInput, char* apcBuff, apl_size_t auSize );

/**
 * This function used to erase the space that at the beginning of a string.
 * @param [in]    aoInput    the string that wanted to be erased left space.
 */ 
void TrimLeft( std::string& aoInput );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be erased left space.
 */
void TrimLeft( char*  apcInput );

/**
 * This function used to erase the space that at the beginning of a string, and set the result th aoResult.
 * The input string aoInput shall not be changed.
 * @param [in]    aoInput    the string that wanted to be eraseed left string.
 * @param [out]   aiResult   the result string.
 */
void TrimLeftCopy( std::string const& aoInput, std::string& aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be erased left space.
 * @param [out]   apcBuff     a pointer pointed to the buffer that hold the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */
void TrimLeftCopy( char const* apcInput, char* apcBuff, apl_size_t auSize );

/**
 * This function shall erase the character in aoInput that satisfy the predicate afPred.
 * @param [in]    aoInput    the input string that wanted to erase some character.
 * @param [in]    afPred     a function pointer pointed to the function that have definition as bool afPred(char).
 */
template< class TPredicate>
void TrimIf( std::string& aoInput, TPredicate afPred )
{
    TrimLeftIf(aoInput, afPred);
    TrimRightIf(aoInput, afPred);
}

/**
 * Overloaded function.
 * @param [in]    a pointer pointed to the string that wanted to erase some charactor.
 * @param [in]    a function pointer pointed th the function that have definition as bool afPred(char).
 */
template<class TPredicate>
void TrimIf( char* apcInput, TPredicate afPred )
{
    TrimLeftIf(apcInput, afPred);
    TrimRightIf(apcInput, afPred);
}

/**
 * This function shall be erase the character in aoInput that satisfy the Predicate afPred, and set the result
 * to aoResult.The input string aoInput shall not be changed.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    afPred     a function pointer pointed to the function that have definition as bool afPred(char).
 * @param [out]   aoResult   the result string.
 */
template<class TPredicate>
void TrimIfCopy( std::string const& aoInput, TPredicate afPred, std::string& aoResult )
{
    aoResult = aoInput;
    
    TrimLeftIf(aoResult, afPred);
    TrimRightIf(aoResult, afPred);
}

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be erased some character.
 * @param [in]    afPred      a function pointer pointed to the function that have definition as bool afPred(char)/
 * @param [out]   apcBuff     a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */
template<class TPredicate>
void TrimIfCopy(
    char const* apcInput,
    TPredicate afPred,
    char* apcBuff,
    apl_size_t  auSize )
{
    if (apcInput != APL_NULL && apcBuff != APL_NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
    
        TrimLeftIf(apcBuff, afPred);
        TrimRightIf(apcBuff, afPred);
    }
}

/**
 * Overloaded function.
 * @param [in]    a pointer pointed to the string that wanted to erase some charactor.
 * @param [in]    a function pointer pointed th the function that have definition as bool afPred(char).
 */
template<class TPredicate>
void TrimRightIf( std::string& aoInput, TPredicate afPred )
{
    apl_int_t liLast = aoInput.size() - 1;
    
    while(liLast >= 0)
    {
        if ( !afPred(aoInput[liLast]) )
        {
            break;
        }
        liLast--;
    }
    
    if (liLast < 0)
    {
        aoInput.erase( aoInput.begin(), aoInput.end() );
    }
    else if ( (apl_size_t)liLast != aoInput.size() - 1 )
    {
        aoInput.erase( aoInput.begin() + liLast + 1, aoInput.end() );
    }
}

/**
 * Overloaded function.
 * @param [in]    a pointer pointed to the string that wanted to erase some charactor.
 * @param [in]    a function pointer pointed th the function that have definition as bool afPred(char).
 */
template<class TPredicate>
void TrimRightIf( char* apcInput, TPredicate afPred )
{
    if (apcInput != APL_NULL)
    {
        apl_int_t liLength = apl_strlen(apcInput);
        apl_int_t liLast = liLength - 1;
        
        while(liLast >= 0)
        {
            if ( !afPred(apcInput[liLast]) )
            {
                break;
            }
            liLast--;
        }
        
        if (liLast < 0)
        {
            apcInput[0] = '\0';
        }
        else if (liLast != liLength - 1)
        {
            apcInput[liLast + 1] = '\0';
        }
    }
}

template<class TPredicate>
void TrimRightIfCopy( std::string const& aoInput, TPredicate afPred, std::string& aoResult )
{
    aoResult = aoInput;
    
    TrimRightIf(aoResult, afPred);
}

template<class TPredicate>
void TrimRightIfCopy( char const* apcInput, TPredicate afPred, char* apcBuff, apl_size_t auSize )
{
    if (apcInput != APL_NULL && apcBuff != NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
        
        TrimRightIf(apcBuff, afPred);
    }
}

template<class TPredicate>
void TrimLeftIf( std::string& aoInput, TPredicate afPred )
{
    std::string::size_type liFirst = 0;
    
    while( liFirst < aoInput.size() )
    {
        if ( !afPred(aoInput[liFirst]) )
        {
            break;
        }
        liFirst++;
    }
        
    if ( liFirst >= aoInput.size() )
    {
        aoInput.erase( aoInput.begin(), aoInput.end() );
    }
    else if (liFirst > 0)
    {
        std::copy( aoInput.begin() + liFirst, aoInput.end(), aoInput.begin() );
        aoInput.erase( aoInput.begin() + (aoInput.size() - liFirst), aoInput.end() );
    }
}

template<class TPredicate>
void TrimLeftIf( char* apcInput, TPredicate afPred )
{
    if (apcInput != APL_NULL)
    {
        apl_int_t liFirst = 0;
        
        if ( !afPred(apcInput[liFirst++]) )
        {
            return;
        }
        
        while(apcInput[liFirst] != '\0')
        {
            if ( !afPred(apcInput[liFirst]) )
            {
                apl_int_t liN = 0;
            
                while(apcInput[liFirst] != '\0')
                {
                    apcInput[liN++] = apcInput[liFirst++];
                }
    
                apcInput[liN++] = '\0';
            
                return;
            }
            
            liFirst++;
        }
        
        apcInput[0] = '\0';
    }
}

template<class TPredicate>
void TrimLeftCopy( std::string const& aoInput, TPredicate afPred, std::string& aoResult )
{
    aoResult = aoInput;
    
    TrimLeftIf(aoResult, afPred);
}

template<class TPredicate>
void TrimLeftCopy( char const* apcInput, TPredicate afPred, char* apcBuff, apl_size_t auSize )
{
    if (apcInput != APL_NULL && apcBuff != NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
        
        TrimLeftIf(apcBuff, afPred);
    }
}

/**
 * This function shall erase the character in aoInput that from the location auPos to the location auPos + auLen.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    auPos      specifies the beginning location that wanted to be erased.
 * @param [in]    auLen      specifies the length of the string that shall be erased.
 */
void EraseRange( std::string& aoInput, apl_size_t auPos, apl_size_t auLen );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be erased some charactor.
 * @param [in]    auPos       specifies the beginning location that wanted to be erased.
 * @param [in]    auLen       specifies the length of the string that shall be erased.
 */
void EraseRange( char* apcInput, apl_size_t auPos, apl_size_t auLen );

/**
 * This function shall be erased the character in aoInput that from the location auPos to the location auPos + auLen.
 * The result string shall be set to aoResult, and the input string aoInput shall not be changed.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    auPos      specifies the beginning location that wanted to be erased.
 * @param [in]    auLen      specifies the length of the string that shall be erased.
 * @param [out]   aoResult   the result string.
 */
void EraseRangeCopy(
    std::string const& aoInput,
    apl_size_t         auPos,
    apl_size_t         auLen,
    std::string&       aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to erased some character.
 * @param [in]    auPos       specifies the beginning location that wanted to be eraseed.
 * @param [in]    auLen       specifies the length of the string that wanted to be erased.
 * @param [out]   apcBuff     a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */
void EraseRangeCopy(
    char const*  apcInput,
    apl_size_t   auPos   ,
    apl_size_t   auLen   ,
    char*        apcBuff ,
    apl_size_t   auSize  );

/**
 * This function used to erase the substring aoSearch in the string aoInput that appears the first time.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    aoSearch   the substring that used to be found in aoInput.
 */
void EraseFirst( std::string& aoInput, std::string const& aoSearch );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to erased some character.
 * @param [in]    aoSearch    a pointer pointed to the substring that used to be found in aoInput.
 */
void EraseFirst( char* apcInput, char const* apcSearch );

/**
 * This function used to erase the substring aoSearch in the string aoInput that appears the first time.
 * The result string shall be set to aoResult.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    aoSearch   the substring that used to be found in aoInput.
 * @param [out]   aoResult   the result string.
 */
void EraseFirstCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    std::string&       aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to erased some character.
 * @param [in]    apcSearch   a pointer pointed to the substring.
 * @param [out]   apcBuff     a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */ 
void EraseFirstCopy(
    char const* apcInput,
    char const* apcSearch,
    char*       apcBuff,
    apl_size_t  auSize );

/**
 * This function used to erase the substring aoSearch in the string aoInput that appears the last time.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    aoSearch   the substring that used to be found in aoInput.
 */
void EraseLast( std::string& aoInput, std::string const& aoSearch );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to erased some character.
 * @param [in]    aoSearch    a pointer pointed to the substring that used to be found in aoInput.
 */
void EraseLast( char* apcInput, char const* apcSearch );

/**
 * This function used to erase the substring aoSearch in the string aoInput that appears the last time.
 * The result string shall be set to aoResult.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    aoSearch   the substring that used to be found in aoInput.
 * @param [out]   aoResult   the result string.
 */
void EraseLastCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    std::string&       aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to erased some character.
 * @param [in]    apcSearch   a pointer pointed to the substring.
 * @param [out]   apcBuff     a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */ 
void EraseLastCopy(
    char const* apcInput,
    char const* apcSearch,
    char*       apcBuff,
    apl_size_t  auSize );

/**
 * This function used to erase the substring aoSearch in the string aoInput that appears at auN times.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    aoSearch   the substring that used to be found in aoInput.
 * @param [in]    auN        specifies the times that the aoSearch appears in aoInput shall be erased.
 */
void EraseNth( std::string& aoInput, std::string const& aoSearch, apl_size_t auN );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to erased some character.
 * @param [in]    aoSearch    a pointer pointed to the substring that used to be found in aoInput.
 * @param [in]    auN         specifies the times that the aoSearch appears in aoInput shall be erased.
 */
void EraseNth( char* apcInput, char const* apcSearch, apl_size_t  auN );

/**
 * This function used to erase the substring aoSearch in the string aoInput that appears at auN times.
 * The result string shall be set to aoResult.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    aoSearch   the substring that used to be found in aoInput.
 * @param [in]    auN        specifies the times that the aoSearch appears in aoInput shall be erased.
 * @param [out]   aoResult   the result string.
 */
void EraseNthCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    apl_size_t         auN,
    std::string&       aoResult);

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to erased some character.
 * @param [in]    aoSearch    a pointer pointed to the substring that used to be found in aoInput.
 * @param [in]    auN         specifies the times that the aoSearch appears in aoInput shall be erased.
 * @param [out]   apcBuff     a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */
void EraseNthCopy(
    char const* apcInput,
    char const* apcSearch,
    apl_size_t  auN,
    char*       apcBuff,
    apl_size_t  auSize);

/** This function shall erase the length of auN string at the beginning of aoInput.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    auN        specifies the length that wanted to be earsed.
 */
void EraseHead( std::string& aoInput, apl_size_t auN );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to erased some character.
 * @param [in]    auN         specifies the length that wanted to be earsed.
 */
void EraseHead( char* apcInput, apl_size_t auSize );

/** This function shall erase the length of auN string at the beginning of aoInput.The result string shall set
 * to aoResult.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    auN        specifies the length that wanted to be earsed.
 * @param [out]   aoResult   the result string.
 */
void EraseHeadCopy( std::string const& aoInput, apl_size_t auN, std::string& aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to erased some character.
 * @param [in]    auN         specifies the length that wanted to be earsed.
 * @param [out]   apcBuff     a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */
void EraseHeadCopy( char const* apcInput, apl_size_t auN, char* apcBuff, apl_size_t auSize );

/** This function shall erase the length of auN string at the end of aoInput.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    auN        specifies the length that wanted to be earsed.
 */
void EraseTail( std::string& aoInput, apl_size_t auN );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to erased some character.
 * @param [in]    auN         specifies the length that wanted to be earsed.
 */
void EraseTail( char* apcInput, apl_size_t auN );

/** This function shall erase the length of auN string at the end of aoInput.The result string shall set
 * to aoResult.
 * @param [in]    aoInput    the input string that wanted to be erased some character.
 * @param [in]    auN        specifies the length that wanted to be earsed.
 * @param [out]   aoResult   the result string.
 */
void EraseTailCopy( std::string const& aoInput, apl_size_t auN, std::string& aoResult);

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to erased some character.
 * @param [in]    auN         specifies the length that wanted to be earsed.
 * @param [out]   apcBuff     a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */
void EraseTailCopy( char const* apcInput, apl_size_t auN, char* apcBuff, apl_size_t auSize );

/**
 * This function shall find the substring aoSearch in the string aoInput and return the location
 * in which find the substring at the first time.
 * @param [in]    aoInput    the stirng wanted to find substring.
 * @param [in]    aoSearch   the substring that used to be found.
 * @retval -1    Can not find the substring.
 * @retval >=0   The location that the substring first appeared.
 */
apl_ssize_t Find( std::string const& aoInput, std::string const& aoSearch );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the stirng wanted to find substring.
 * @param [in]    apcSearch   a pointer pointed to the substring that used to be found.
 * @retval -1    Can not find the substring.
 * @retval >=0   The location that the substring first appeared.
 */
apl_ssize_t Find( char const* apcInput, char const* apcSearch );

/**
 * This function shall find the substring aoSearch in the string aoInput and return the location
 * in which find the substring at the first time.
 * @param [in]    aoInput    the stirng wanted to find substring.
 * @param [in]    aoSearch   the substring that used to be found.
 * @retval -1    Can not find the substring.
 * @retval >=0   The location that the substring first appeared.
 */
apl_ssize_t FindFirst( std::string const& aoInput, std::string const& aoSearch );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the stirng wanted to find substring.
 * @param [in]    apcSearch   a pointer pointed to the substring that used to be found.
 * @retval -1    Can not find the substring.
 * @retval >=0   The location that the substring first appeared.
 */
apl_ssize_t FindFirst( char const* apcInput, char const* apcSearch );

/**
 * This function shall find the substring aoSearch in the string aoInput and return the location
 * in which find the substring at the last time.
 * @param [in]    aoInput    the stirng wanted to find substring.
 * @param [in]    aoSearch   the substring that used to be found.
 * @retval -1    Can not find the substring.
 * @retval >=0   The location that the substring last appeared.
 */
apl_ssize_t FindLast( std::string const& aoInput, std::string const& aoSearch );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the stirng wanted to find substring.
 * @param [in]    apcSearch   a pointer pointed to the substring that used to be found.
 * @retval -1    Can not find the substring.
 * @retval >=0   The location that the substring last appeared.
 */
apl_ssize_t FindLast( char const* apcInput, char const* apcSearch );

/**
 * This function shall find the substring aoSearch in the string aoInput and return the location
 * in which find the substring at auN times.
 * @param [in]    aoInput    the stirng wanted to find substring.
 * @param [in]    aoSearch   the substring that used to be found.
 * @param [in]    auN        specifies the times of the substring appeared in the aoInput that shall be found. 
 * @retval -1    Can not find the substring.
 * @retval >=0   The location that the substring appeared at auN times.
 */
apl_ssize_t FindNth( std::string const& aoInput, std::string const& aoSearch, apl_size_t auN );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the stirng wanted to find substring.
 * @param [in]    apcSearch   a pointer pointed to the substring that used to be found.
 * @param [in]    auN        specifies the times of the substring appeared in the aoInput that shall be found. 
 * @retval -1    Can not find the substring.
 * @retval >=0   The location that the substring appeared in the aoInput that shall be found.
 */
apl_ssize_t FindNth( char const* apcInput, char const* apcSearch, apl_size_t auN );

apl_ssize_t KMPFind( std::string const& aoInput, std::string const& aoSearch );

apl_ssize_t KMPFind( char const* apcInput, const char* apcSearch );

apl_ssize_t KMPFind( char const* apcInput, apl_size_t auLength, const char* apcSearch );

apl_ssize_t KMPFind( char const* apcInput, const char* apcSearch, apl_size_t auSearchLen );

apl_ssize_t KMPFind( char const* apcInput, apl_size_t auLength, const char* apcSearch, apl_size_t auSearchLen );

/**
 * This function shall replace the substring that from the location auPos to the location aiPis + auLen in aoInput,
 * and the repace string is aoRep.
 * @param [in]    aoInput    the string that wanted to be changed.
 * @param [in]    auPos      specifies the location that beginning replace.
 * @param [in]    auLen      specifies the length of string to be replace.
 * @param [in]    aoRep      the string that use to replace the string that specifies by auPos and auLen.
 * @param [out]    aoResult   the result string.
 */
void ReplaceRangeCopy(
    std::string const& aoInput,
    apl_size_t         auPos,
    apl_size_t         auLen,
    std::string const& aoRep,
    std::string&       aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput   a pointer pointed to the string that wanted to be changed.
 * @param [in]    auPos      specifies the location that beginning replace.
 * @param [in]    auLen      specifies the length of string to be replace.
 * @param [in]    aoRep      the string that use to replace the string that specifies by auPos and auLen.
 * @param [out]   apcBuff    a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize     specifies the length of the buffer that pointed by apcBuff. 
 */
void ReplaceRangeCopy(
    char const* apcInput,
    apl_size_t  auPos,
    apl_size_t  auLen,
    std::string const& aoRep,
    char*       apcBuff,
    apl_size_t  auSize );

/**
 * This function shall replace the substring aoSearch to the substring aoRep in aoInput, the substring shall be the first time that
 * appeared in aoInput. The result shall be set to aoResult. 
 * @param [in]    aoInput    the string that wanted to be changed.
 * @param [in]    aoSearch   the substring wanted to be replaced.
 * @param [in]    aoRep      the substring used to replaced 
 * @param [out]   aoResult   the result string.
 */
void ReplaceFirstCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    std::string&       aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be changed.
 * @param [in]    apcSearch   a pointer pointed to the substring wanted to be replaced.
 * @param [in]    aoRep       a pointer pointed to the substring used to replaced 
 * @param [out]   apcBuff     a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */
void ReplaceFirstCopy(
    char const*        apcInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    char*              apcBuff,
    apl_size_t         auSize );

/**
 * This function shall replace the substring aoSearch to the substring aoRep in aoInput, the substring shall be the last time that
 * appeared in aoInput. The result shall be set to aoResult. 
 * @param [in]    aoInput    the string that wanted to be changed.
 * @param [in]    aoSearch   the substring wanted to be replaced.
 * @param [in]    aoRep      the substring used to replaced 
 * @param [out]   aoResult   the result string.
 */
void ReplaceLastCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    std::string&       aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be changed.
 * @param [in]    apcSearch   a pointer pointed to the substring wanted to be replaced.
 * @param [in]    aoRep       a pointer pointed to the substring used to replaced 
 * @param [out]   apcBuff     a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */
void ReplaceLastCopy(
    char const*        apcInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    char*              apcBuff,
    apl_size_t         auSize );

/**
 * This function shall replace the substring aoSearch to the substring aoRep in aoInput, all substring 
 * that appeared in aoInput shall be replaced. The result shall be set to aoResult. 
 * @param [in]    aoInput    the string that wanted to be changed.
 * @param [in]    aoSearch   the substring wanted to be replaced.
 * @param [in]    aoRep      the substring used to replaced 
 * @param [out]   aoResult   the result string.
 */
void ReplaceCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    std::string&       aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be changed.
 * @param [in]    apcSearch   a pointer pointed to the substring wanted to be replaced.
 * @param [in]    aoRep       a pointer pointed to the substring used to replaced 
 * @param [out]   apcBuff     a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */
void ReplaceCopy(
    char const*        apcInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    char*              apcBuff,
    apl_size_t         auSize );

/**
 * This function shall replace the substring aoSearch to the substring aoRep in aoInput, the substring shall be the auN times that
 * appeared in aoInput. The result shall be set to aoResult. 
 * @param [in]    aoInput    the string that wanted to be changed.
 * @param [in]    aoSearch   the substring wanted to be replaced.
 * @param [in]    auN        specifies the times of the substring appeared in the aoInput that shall be replaced. 
 * @param [in]    aoRep      the substring used to replaced 
 * @param [out]   aoResult   the result string.
 */
void ReplaceNthCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    apl_size_t         auN,
    std::string const& aoRep,
    std::string&       aoResult );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string that wanted to be changed.
 * @param [in]    apcSearch   a pointer pointed to the substring wanted to be replaced.
 * @param [in]    auN         specifies the times of the substring appeared in the aoInput that shall be replaced. 
 * @param [in]    aoRep       a pointer pointed to the substring used to replaced 
 * @param [out]   apcBuff     a pointer pointed to the buffer that holds the result string.
 * @param [in]    auSize      specifies the length of the buffer that pointed by apcBuff.
 */
void ReplaceNthCopy(
    char const*        apcInput,
    std::string const& aoSearch,
    apl_size_t         auN,
    std::string const& aoRep,
    char*              apcBuff,
    apl_size_t         auSize );


/**
 * This function shall check if the beginning of the string aoInput equal to aoTest.
 * @param [in]    aoInput    the string wanted to be checked.
 * @param [in]    aoTest     the substring used to check.
 * @retval true    The beginning of the string aoInput equal to aoTest.
 * @retval false   The beginning of the string aoInput does not equal to aoTest.
 */  
bool StartWith( std::string const& aoInput, std::string const& aoTest );

/**
 * Overloaded function.
 * @param [in]    aoInput    the string wanted to be checked.
 * @param [in]    apcTest     a pointer pointed to the substring used to check.
 * @retval true    The beginning of the string aoInput equal to the substring that pointed by apcTest.
 * @retval false   The beginning of the string aoInput does not equal to the substring that pointed by apcTest.
 */  
bool StartWith( std::string const& aoInput, char const* apcTest );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string wanted to be checked.
 * @param [in]    apcTest     a pointer pointed to the substring used to check.
 * @retval true    The beginning of the string pointed by apcInput equal to the substring that pointed by aoTest.
 * @retval false   The beginning of the string pointed byaoInput does not equal to the substring that pointed by aoTest.
 */  
bool StartWith( char const* apcInput, char const* apcTest );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string wanted to be checked.
 * @param [in]    aoTest     the substring used to check.
 * @retval true    The beginning of the string pointed by apcInput equal to the substring aoTest.
 * @retval false   The beginning of the string pointed byaoInput does not equal to the substring aoTest.
 */  
bool StartWith( char const* apcInput, std::string const& aoTest );


/**
 * This function shall check if the end of the string aoInput equal to aoTest.
 * @param [in]    aoInput    the string wanted to be checked.
 * @param [in]    aoTest     the substring used to check.
 * @retval true    The end of the string aoInput equal to aoTest.
 * @retval false   The end of the string aoInput does not equal to  aoTest.
 */  
bool EndWith( std::string const& aoInput, std::string const& aoTest );

/**
 * Overloaded function.
 * @param [in]    aoInput    the string wanted to be checked.
 * @param [in]    apcTest     a pointer pointed to the substring used to check.
 * @retval true    The end of the string aoInput equal to the substring that pointed by apcTest.
 * @retval false   The end of the string aoInput does not equal to the substring that pointed by apcTest.
 */  
bool EndWith( std::string const& aoInput, char const* apcTest );

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string wanted to be checked.
 * @param [in]    apcTest     a pointer pointed to the substring used to check.
 * @retval true    The end of the string pointed by apcInput equal to the substring that pointed by aoTest.
 * @retval false   The end of the string pointed byaoInput does not equal to the substring that pointed by aoTest.
 */  
bool EndWith( char const* apcInput, char const* apcTest );

/**
 * Overloaded function.
 * @param [in]    apcInput   a pointer pointed to the string wanted to be checked.
 * @param [in]    aoTest     the substring used to check.
 * @retval true    The end of the string pointed by apcInput equal to the substring aoTest.
 * @retval false   The end of the string pointed byaoInput does not equal to the substring aoTest.
 */  
bool EndWith( char const* apcInput, std::string const& aoTest );


/**
 * This function shall check if the string aoInput contain substring aoTest.
 * @param [in]    aoInput    the string wanted to be checked.
 * @param [in]    aoTest     the substring used to check.
 * @retval true    The string aoInput contains the substring aoTest.
 * @retval false   The string aoInput does not contain the substring aoTest.
 */  
bool Contain( std::string const& aoInput, std::string const& aoTest );

/**
 * Overloaded function.
 * @param [in]    aoInput    the string wanted to be checked.
 * @param [in]    apcTest    a pointer pointed to the substring used to check.
 * @retval true    The string aoInput contains the substring pointed by apcTest.
 * @retval false   The string aoInput does not contain the substring pointed by apcTest.
 */  
bool Contain( std::string const& aoInput, char const* apcTest );

/**
 * Overloaded function.
 * @param [in]    apcInput   a pointer pointed to the string wanted to be checked.
 * @param [in]    apcTest    a pointer pointed to the substring used to check.
 * @retval true    The string pointed by apcInput contains the substring pointed by apcTest.
 * @retval false   The string pointed by apcInput does not contain the substring pointed by apcTest.
 */  
bool Contain( char const* apcInput, char const* apcTest );

/**
 * Overloaded function.
 * @param [in]    apcInput   a pointer pointed to the string wanted to be checked.
 * @param [in]    aoTest     the substring used to check.
 * @retval true    The string pointed by apcInput contains the substring aoTest.
 * @retval false   The string pointed by apcInput does not contain the substring aoTest.
 */  
bool Contain( char const* apcInput, std::string const& aoTest );

/**
 * This function shall check if the string aoInput equal substring aoTest.
 * @param [in]    aoInput    the string wanted to be checked.
 * @param [in]    aoTest     the substring used to check.
 * @retval true    The string aoInput equals to  the substring aoTest.
 * @retval false   The string aoInput does not equals to the substring aoTest.
 */  
bool Equal( std::string const& aoInput, std::string const& aoTest );

/**
 * Overloaded function.
 * @param [in]    aoInput    the string wanted to be checked.
 * @param [in]    apcTest    a pointer pointed to the substring used to check.
 * @retval true    The string aoInput equals to the substring pointed by apcTest.
 * @retval false   The string aoInput does not equal to the substring pointed by apcTest.
 */  
bool Equal( std::string const& aoInput, char const* apcTest );

/**
 * Overloaded function.
 * @param [in]    apcInput   a pointer pointed to the string wanted to be checked.
 * @param [in]    apcTest    a pointer pointed to the substring used to check.
 * @retval true    The string pointed by apcInput equals to the substring pointed by apcTest.
 * @retval false   The string pointed by apcInput does not equal to the substring pointed by apcTest.
 */  
bool Equal( char const* apcInput, char const* apcTest );

/**
 * Overloaded function.
 * @param [in]    apcInput   a pointer pointed to the string wanted to be checked.
 * @param [in]    aoTest     the substring used to check.
 * @retval true    The string pointed by apcInput equals to  the substring aoTest.
 * @retval false   The string pointed by apcInput does not equal to the substring aoTest.
 */  
bool Equal( char const* apcInput, std::string const& aoTest );

/**
 * This function shall check if the string satisfy the predicate afPred.
 * @param [in]    aoInput    the string wanted to be check.
 * @param [in]    afPred      a function pointer pointed to the function that definition as bool afPred(char).
 * @retval true    The string aoInput satisfy the predicate afPred.
 * @retval false   The string aoInput does not satisfy the predicate afPred.
 */
template<class TPredicate>
bool All( std::string const& aoInput, TPredicate afPred )
{
    for (apl_size_t luN = 0; luN < aoInput.size(); luN++)
    {
        if ( !afPred(aoInput[luN]) )
        {
            return false;
        }
    }
    
    return true;
}

/**
 * Overloaded function.
 * @param [in]    apcInput    a pointer pointed to the string wanted to be check.
 * @param [in]    afPred      a function pointer pointed to the function that definition as bool afPred(char).
 * @retval true    The string pointed by apcInput satisfy the predicate afPred.
 * @retval false   The string pointed by apcInput does not satisfy the predicate afPred.
 */
template<class TPredicate>
bool All( char const* apcInput, TPredicate afPred )
{
    for (char const* lpcFirst = apcInput; lpcFirst != NULL && *lpcFirst != '\0'; lpcFirst++)
    {
        if ( !afPred(*lpcFirst) )
        {
            return false;
        }
    }
    
    return true;
}

}//end stralgo namespace

ACL_NAMESPACE_END

#endif//ACL_STRALGO_H
