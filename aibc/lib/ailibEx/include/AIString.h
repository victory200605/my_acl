/** @file AIString.h
*/

#ifndef __AILIBEX__AISTRING_H__
#define __AILIBEX__AISTRING_H__

#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Delete the blank character both before the head of the string and the end of the string.
 *
 * @param apcInStr   IN/OUT - pointer to the start of input/output string
 *
 * @return   Return the pointer to the start of output string.
 */
extern  char* StringTrim(char* apcInStr);

/**
 * @brief Translate lowercase characters to uppercase.
 *
 * @param apcInStr   IN/OUT - pointer to the start of input/output string
 *
 * @return   Return the pointer to the start of output string.
 */  
extern  char* StringUpper(char* apcInStr);

/**
 * @brief Translate uppercase characters to lowercase.
 *
 * @param apcInStr   IN/OUT - pointer to the start of input/output string
 *
 * @return   Return the pointer to the start of output string.
 */
extern  char* StringLower(char* apcInStr);

/**
 * @brief Convert a string to an unsigned long integer(dec or hex).
 *
 * @param apcInStr   IN - pointer to the start of input string
 *
 * @return   Return the converted value if successfully completion.Otherwise,0 shall be returned.
 */
extern  int StringAtoi(char const* apcInStr);

/**
 * @brief Copy part of a string.
 *
 * @param apcDestStr     OUT - pointer to the start of output string 
 * @param apcSrcStr      IN - pointer to the start of input string
 * @param aiSize         IN - the max size to copy
 *
 */  
extern  void StringCopy(char* apcDestStr, char const* apcSrcStr, size_t aiSize);

/**
 * @brief Find a specified string in the original string.
 *
 * @param apData     IN - the original string
 * @param aiLen      IN - the specified searching size 
 * @param apcStr     IN - the specified string which you want to find
 *
 * @return Return the position of the specified string in the original string.
           Return NULL if cannot find.
 */  
extern  char const* FindSubstr(char const* apData, size_t aiLen, char const* apcStr);


//////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Test a string whether is integer or not.
 *
 * @parma apcString      IN - pointer to the start of input string
 *
 * @return 0:if the string is integer;1:if is not
 */  
extern	int StringIsInteger(char const* apcString);

/**
 * @brief Test a string whether is decimal fraction or not.
 * 
 * @param apcString      IN - pointer to the start of input string
 *
 * @return 0:if the string is decimal fraction;1:if is not
 */
extern	int StringIsNumber(char const* apcString);

//////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Get string hash key,method 1.
 *
 * @param apcString     IN - the specified string
 * @param aiMaxHashKey  IN - the table size
 *
 * @return the hash key
 */
extern  unsigned int GetStringHashKey(char const* apcString, size_t aiMaxHashKey);

/**
 * @brief Get string hash key,method 2.
 *
 * @param apcString     IN - the specified string
 * @param aiMaxHashKey  IN - the table size
 *
 * @return the hash key
 */
extern	unsigned int GetStringHashKey1(char const* apcString, size_t aiMaxHashKey);

/**
 * @brief Get string hash key,method 3.
 *
 * @param apcString     IN - the specified string
 * @param aiMaxHashKey  IN - the table size
 *
 * @return the hash key
 */
extern	unsigned int GetStringHashKey2(char const* apcString, size_t aiMaxHashKey);

//////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Get the path name.
 *
 * @param apcFullName    IN - full path name(include pathname and filename)
 * @param apcPath        OUT - path name
 * @param aiSize         IN - the specified size to copy
 *
 */
extern	void GetPathName(char const* apcFullName, char *apcPath, size_t aiSize);

/**
 * @brief Get the file name.
 *
 * @param apcFullName    IN - full path name(include pathename and filename)
 * @param apcFile        OUT - file name
 * @param aiSize         IN - the specified size to copy
 *
 */
extern	void GetFileName(char const* apcFullName, char *apcFile, size_t aiSize);

/**
 * @brief Test two strings can match or not.
 *
 * @param apcInData    IN - one original string
 * @param apcPattern   IN - the pattern string
 * @param aiNoCase     IN - distinguish lowercase and capital or not,if aiNoCase=true then don't distinguish lowercase and capital
 *
 * @return 1: if two strings can match. 0:cannot match.
 */
extern  bool StringMatch(char const* apcInData, char const* apcPattern, bool aiNoCase);

//////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Format data to hex
 *
 * @param apcInBuf     IN - the input buffer
 * @param aiBufSize    IN - the buffer size 
 *
 * @return the format buffer
 */ 
extern	char* FormatDataToHex(char const* apcInBuf, size_t aiBufSize);

//////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Get column by the specified number.
 *
 * @param apcString      IN - pointer to the start of input string
 * @param aiColumn       IN - which column you want to get
 * @param acDelimiter    IN - the delimiter
 * @param aiLen          OUT -the max size of column to get
 *
 * @return Return the pointer to the start of the column string you get.
 */
extern	char* GetStringColumn(char const* apcString, size_t aiColumn, char acDelimiter, size_t* aiLen);

/**
 * @brief Extract the input string to columns by delimiter. 
 *
 * @param apcString           IN - pointer to the start of input string
 * @param acDelimiter         IN - the delimiter
 * @param apcQuoter           IN - the quoter(ignore the delimiter in the quoter)
 * @param apcStringColumn     OUT - the output array to store columns
 * @param aiColumnCount       IN - the max count of columns
 *
 * @return the actual count of columns
 */ 
extern	int ExtractStringColumns(char const* apcString, char acDelimiter,
				char const* apcQuoter, char** apcStringColumn, size_t aiColumnCount);

/**
 * @brief Release the allocated memory of columns.
 *
 * @param apcStringColumn       IN - the input column array
 * @param aiColumnCount         IN - the num of column
 */ 
extern	void ReleaseStringColumns(char** apcStringColumn, size_t aiColumnCount);

//////////////////////////////////////////////////////////////////////////////////////////
#define FOR_EACH_TOKEN(tokbegin, tokend, data, len, sep) \
    for (char const *tokbegin = ((char const*)(data)), *tokend = FindSubstr(((char const*)(data)), (len), (sep)); \
        tokend != NULL && tokend < (((char const*)(data)) + (len)); \
        (tokbegin = tokend + strlen(sep)), (tokend = FindSubstr(tokbegin, (len) - (tokbegin - ((char const*)(data))), (sep)))) 

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AISTRING_H__

