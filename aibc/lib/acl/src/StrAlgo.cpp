
#include "acl/Utility.h"
#include "acl/StrAlgo.h"

ACL_NAMESPACE_START

namespace stralgo {

/*
 * Lower
 */
void ToLower( std::string& aoInput )
{
    for (apl_size_t luN = 0; luN < aoInput.size(); luN++)
    {
        aoInput[luN]= apl_tolower(aoInput[luN]);
    }
}

void ToLower( char* apcInput )
{
    if (apcInput != APL_NULL)
    {
        for (char* apcFirst = apcInput; *apcFirst != '\0'; apcFirst++)
        {
            *apcFirst = apl_tolower(*apcFirst);
        }
    }
}

void ToLowerCopy( std::string const& aoInput, std::string& aoResult)
{
    aoResult.resize( aoInput.size() );
    
    for (apl_size_t luN = 0; luN < aoInput.size(); luN++)
    {
        aoResult[luN] = apl_tolower(aoInput[luN]);
    }
}

void ToLowerCopy( char const* apcInput, char* apcBuff, apl_size_t auSize )
{
    apl_size_t liIdx = 0;
    
    if (apcInput != APL_NULL && apcBuff != APL_NULL)
    {
        for (char const* apcFirst = apcInput; *apcFirst != '\0' && liIdx < auSize - 1; apcFirst++)
        {
            apcBuff[liIdx++] = apl_tolower(*apcFirst);
        }
        
        apcBuff[liIdx++] = '\0';
    }
}

/*
 * Upper
 */
void ToUpper( std::string& aoInput )
{
    for (apl_size_t luN = 0; luN < aoInput.size(); luN++)
    {
        aoInput[luN]= apl_toupper(aoInput[luN]);
    }
}

void ToUpper( char* apcInput )
{
    if (apcInput != APL_NULL)
    {
        for (char* apcFirst = apcInput; *apcFirst != '\0'; apcFirst++)
        {
            *apcFirst = apl_toupper(*apcFirst);
        }
    }
}

void ToUpperCopy( std::string const& aoInput, std::string& aoResult )
{
    aoResult.resize( aoInput.size() );
    
    for (apl_size_t luN = 0; luN < aoInput.size(); luN++)
    {
        aoResult[luN] = apl_toupper(aoInput[luN]);
    }
}

void ToUpperCopy( char const* apcInput, char* apcBuff, apl_size_t auSize )
{
    apl_size_t liIdx = 0;
    
    if (apcInput != APL_NULL && apcBuff != APL_NULL)
    {
        for (char const* apcFirst = apcInput; *apcFirst != '\0' && liIdx < auSize - 1; apcFirst++)
        {
            apcBuff[liIdx++] = apl_toupper(*apcFirst);
        }
        
        apcBuff[liIdx++] = '\0';
    }
}

void Trim( std::string& aoInput )
{
    TrimLeft(aoInput);
    TrimRight(aoInput);
}

void Trim( char* apcInput )
{
    TrimLeft(apcInput);
    TrimRight(apcInput);
}

void TrimCopy( std::string const& aoInput, std::string& aoResult )
{
    aoResult = aoInput;
    
    TrimLeft(aoResult);
    TrimRight(aoResult);
}

void TrimCopy( char const* apcInput, char* apcBuff , apl_size_t  auSize )
{
    if (apcInput != APL_NULL && apcBuff != APL_NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
    
        TrimLeft(apcBuff);
        TrimRight(apcBuff);
    }
}

void TrimRight( std::string& aoInput )
{
    apl_int_t liLast = aoInput.size() - 1;
    
    while(liLast >= 0)
    {
        if (aoInput[liLast] != ' ')
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

void TrimRight( char* apcInput )
{
    if (apcInput != APL_NULL)
    {
        apl_int_t liLength = apl_strlen(apcInput);
        apl_int_t liLast = liLength - 1;
        
        while(liLast >= 0)
        {
            if (apcInput[liLast] != ' ')
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

void TrimRightCopy( std::string const& aoInput, std::string& aoResult )
{
    aoResult = aoInput;
    
    TrimRight(aoResult);
}

void TrimRightCopy( char const* apcInput, char* apcBuff, apl_size_t auSize )
{
    if (apcInput != APL_NULL && apcBuff != NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
        
        TrimRight(apcBuff);
    }
}

void TrimLeft( std::string& aoInput )
{
    std::string::size_type liFirst = aoInput.find_first_not_of(' ');

    if (liFirst == std::string::npos)
    {
        aoInput.erase( aoInput.begin(), aoInput.end() );
    }
    else if (liFirst > 0)
    {
        std::copy( aoInput.begin() + liFirst, aoInput.end(), aoInput.begin() );
        aoInput.erase( aoInput.begin() + (aoInput.size() - liFirst), aoInput.end() );
    }
}

void TrimLeft( char* apcInput )
{
    if (apcInput != APL_NULL)
    {
        apl_int_t liLength = apl_strlen(apcInput);
        apl_int_t liFirst = 0;
        
        while(liFirst < liLength)
        {
            if (apcInput[liFirst] != ' ')
            {
                break;
            }
            
            liFirst++;
        }
        
        if (liFirst >= liLength)
        {
            apcInput[0] = '\0';
        }
        else if (liFirst > 0)
        {
            apl_memmove(apcInput, apcInput + liFirst, liLength - liFirst);
            apcInput[liLength - liFirst] = '\0';
        }
    }
}

void TrimLeftCopy( std::string const& aoInput, std::string& aoResult )
{
    aoResult = aoInput;
    
    TrimLeft(aoResult);
}

void TrimLeftCopy( char const* apcInput, char* apcBuff , apl_size_t auSize )
{
    if (apcInput != APL_NULL && apcBuff != NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
        
        TrimLeft(apcBuff);
    }
}

void EraseRange( std::string& aoInput, apl_size_t auPos, apl_size_t auLen  )
{
    aoInput.erase(auPos, auLen);
}

void EraseRange( char* apcInput, apl_size_t auPos, apl_size_t auLen )
{
    apl_size_t luLength = apl_strlen(apcInput);
    
    if (auPos + auLen < luLength)
    {
        apl_memmove(apcInput + auPos, apcInput + auPos + auLen, luLength - auPos - auLen + 1/*end*/);
    }
    else if (auPos + auLen == luLength || auPos < luLength)
    {
        apcInput[auPos] = '\0';
    }
}

void EraseRangeCopy(
    std::string const& aoInput,
    apl_size_t auPos,
    apl_size_t auLen,
    std::string& aoResult )
{
    aoResult = aoInput;
    
    EraseRange(aoResult, auPos, auLen);
}

void EraseRangeCopy(
    char const* apcInput,
    apl_size_t auPos,
    apl_size_t auLen,
    char* apcBuff,
    apl_size_t auSize )
{
    if (apcInput != APL_NULL && apcBuff != APL_NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
        
        EraseRange(apcBuff, auPos, auLen);
    }
}

void EraseFirst( std::string& aoInput, std::string const& aoSearch)
{
    std::string::size_type liFirst = aoInput.find(aoSearch);
    if ( liFirst != std::string::npos )
    {
        EraseRange(aoInput, liFirst, aoSearch.size() );
    }
}

void EraseFirst( char* apcInput, char const* apcSearch )
{
    if (apcInput != APL_NULL && apcSearch != APL_NULL)
    {
        char const* apcFirst = apl_strstr(apcInput, apcSearch);
        if (apcFirst != NULL)
        {
            EraseRange( apcInput, apcFirst - apcInput, apl_strlen(apcSearch) );
        }
    }
}

void EraseFirstCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    std::string& aoResult )
{
    aoResult = aoInput;
    
    EraseFirst(aoResult, aoSearch);
}

void EraseFirstCopy(
    char const* apcInput,
    char const* apcSearch,
    char* apcBuff,
    apl_size_t auSize )
{
    if (apcInput != APL_NULL && apcBuff != APL_NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
        
        EraseFirst(apcBuff, apcSearch);
    }
}

void EraseLast( std::string& aoInput, std::string const& aoSearch )
{
    std::string::size_type liLast = std::string::npos;
    std::string::size_type liNext = 0;
        
    while(true)
    {
        liNext = aoInput.find(aoSearch, liNext);
        if (liNext == std::string::npos)
        {
            break;
        }
        liLast = liNext;
        liNext += aoSearch.size();
    }
    
    if ( liLast != std::string::npos )
    {
        EraseRange(aoInput, liLast, aoSearch.size() );
    }
}

void EraseLast( char* apcInput, char const* apcSearch )
{
    if (apcInput != APL_NULL && apcSearch != APL_NULL)
    {
        char const* lpcNext = apcInput;
        char const* lpcLast = NULL;
        apl_size_t  luSearchLength = apl_strlen(apcSearch);
        
        while(true)
        {
            lpcNext = apl_strstr(lpcNext, apcSearch);
            if (lpcNext == APL_NULL)
            {
                break;
            }
            lpcLast = lpcNext;
            lpcNext += luSearchLength;
        }
    
        if (lpcLast != NULL)
        {
            EraseRange( apcInput, lpcLast - apcInput, luSearchLength );
        }
    }
}

void EraseLastCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    std::string& aoResult)
{
    aoResult = aoInput;
    
    EraseLast(aoResult, aoSearch);
}

void EraseLastCopy(
    char const* apcInput,
    char const* apcSearch,
    char* apcBuff,
    apl_size_t  auSize)
{
    if (apcInput != APL_NULL && apcBuff != APL_NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
        
        EraseLast(apcBuff, apcSearch);
    }
}

void EraseNth( std::string& aoInput, std::string const& aoSearch, apl_size_t auN )
{
    std::string::size_type liLast = std::string::npos;
    std::string::size_type liNext = 0;
    
    while(true)
    {
        liNext = aoInput.find(aoSearch, liNext);
        if (liNext == std::string::npos)
        {
            break;
        }
        //Last one
        liLast = liNext;
        liNext += aoSearch.size();
        
        if (--auN <= 0)
        {
            break;
        }
    }
    
    if (liLast != std::string::npos && auN == 0)
    {
        EraseRange(aoInput, liLast, aoSearch.size() );
    }
}

void EraseNth( char* apcInput, char const* apcSearch, apl_size_t  auN)
{
    if (apcInput != APL_NULL && apcSearch != APL_NULL)
    {
        char const* lpcNext = apcInput;
        char const* lpcLast = NULL;
        apl_size_t  luSearchLength = apl_strlen(apcSearch);
        
        while(true)
        {
            lpcNext = apl_strstr(lpcNext, apcSearch);
            if (lpcNext == APL_NULL)
            {
                break;
            }
            //Last one
            lpcLast = lpcNext;
            lpcNext += luSearchLength;
            
            if (--auN <= 0)
            {
                break;
            }
        }
    
        if (lpcLast != NULL && auN == 0)
        {
            EraseRange( apcInput, lpcLast - apcInput, luSearchLength );
        }
    }
}

void EraseNthCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    apl_size_t auN,
    std::string& aoResult)
{
    aoResult = aoInput;
    
    EraseNth(aoResult, aoSearch, auN);
}

void EraseNthCopy(
    char const* apcInput,
    char const* apcSearch,
    apl_size_t  auN,
    char*       apcBuff,
    apl_size_t  auSize)
{
    if (apcInput != APL_NULL && apcBuff != APL_NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
        
        EraseNth(apcBuff, apcSearch, auN);
    }
}

void EraseHead( std::string& aoInput, apl_size_t auN )
{
    EraseRange(aoInput, 0, auN);
}

void EraseHead( char* apcInput, apl_size_t auSize )
{
    EraseRange(apcInput, 0, auSize);
}

void EraseHeadCopy( std::string const& aoInput, apl_size_t auN, std::string& aoResult )
{
    EraseRangeCopy(aoInput, 0, auN, aoResult);
}

void EraseHeadCopy(
    char const* apcInput,
    apl_size_t  auN,
    char*       apcBuff,
    apl_size_t  auSize)
{
    EraseRangeCopy(apcInput, 0, auN, apcBuff, auSize);
}

void EraseTail( std::string& aoInput, apl_size_t auN )
{
    EraseRange(aoInput, aoInput.size() - auN, auN);
}

void EraseTail( char* apcInput, apl_size_t auN )
{
    if ( apcInput != APL_NULL )
    {
        apl_size_t luLength = apl_strlen(apcInput);
    
        EraseRange(apcInput, luLength - auN, auN);
    }
}

void EraseTailCopy( std::string const& aoInput, apl_size_t auN, std::string& aoResult )
{
    EraseRangeCopy(aoInput, aoInput.size() - auN, auN, aoResult);
}

void EraseTailCopy(
    char const* apcInput,
    apl_size_t  auN,
    char* apcBuff,
    apl_size_t  auSize )
{
    if ( apcInput != APL_NULL )
    {
        apl_size_t luLength = apl_strlen(apcInput);
    
        EraseRangeCopy(apcInput, luLength - auN, auN, apcBuff, auSize);
    }
}

apl_ssize_t Find( std::string const& aoInput, std::string const& aoSearch )
{
    std::string::size_type liPos = aoInput.find(aoSearch);
        
    return liPos == std::string::npos ? -1 : (apl_ssize_t)liPos;
}

apl_ssize_t Find( char const* apcInput, char const* apcSearch )
{
    if (apcInput != APL_NULL && apcSearch != APL_NULL)
    {
        char const* apcFirst = apl_strstr(apcInput, apcSearch);
        if (apcFirst != NULL)
        {
            return (apl_int_t)(apcFirst - apcInput);
        }
    }
    
    return -1;
}

apl_ssize_t FindFirst( std::string const& aoInput, std::string const& aoSearch )
{
    return Find(aoInput, aoSearch);
}

apl_ssize_t FindFirst( char const* apcInput, char const* apcSearch )
{
    return Find(apcInput, apcSearch );
}

apl_ssize_t FindLast( std::string const& aoInput, std::string const& aoSearch )
{
    std::string::size_type liLast = std::string::npos;
    std::string::size_type liNext = 0;
    
    while(true)
    {
        liNext = aoInput.find(aoSearch, liNext);
        if (liNext == std::string::npos)
        {
            break;
        }
        
        liLast = liNext;
        liNext += aoSearch.size();
    }

    return liLast == std::string::npos ? -1 : (apl_ssize_t)liLast;
}

apl_ssize_t FindLast( char const* apcInput, char const* apcSearch )
{
    if (apcInput != APL_NULL && apcSearch != APL_NULL)
    {
        char const* lpcLast = NULL;
        char const* lpcNext = apcInput;
        apl_size_t  liSearchLength = apl_strlen(apcSearch);
        
        while(true)
        {
            lpcNext = apl_strstr(lpcNext, apcSearch);
            if (lpcNext == NULL)
            {
                break;
            }
            
            lpcLast = lpcNext;
            lpcNext += liSearchLength;
        }
    
        return lpcLast == NULL ? -1 : (apl_ssize_t)(lpcLast - apcInput);
    }

    return -1;
}

apl_ssize_t FindNth(
    std::string const& aoInput,
    std::string const& aoSearch,
    apl_size_t  auN )
{
    std::string::size_type liLast = std::string::npos;
    std::string::size_type liNext = 0;
    
    while(true)
    {
        liNext = aoInput.find(aoSearch, liNext);
        if (liNext == std::string::npos)
        {
            break;
        }
        
        liLast = liNext;
        liNext += aoSearch.size();
        
        if (--auN <= 0)
        {
            break;
        }
    }

    return liLast == std::string::npos || auN != 0 ? -1 : (apl_ssize_t)liLast;
}

apl_ssize_t FindNth( char const* apcInput, char const* apcSearch, apl_size_t auN )
{
    if (apcInput != APL_NULL && apcSearch != APL_NULL)
    {
        char const* lpcLast = NULL;
        char const* lpcNext = apcInput;
        apl_size_t  liSearchLength = apl_strlen(apcSearch);
        
        while(true)
        {
            lpcNext = apl_strstr(lpcNext, apcSearch);
            if (lpcNext == NULL)
            {
                break;
            }
            
            lpcLast = lpcNext;
            lpcNext += liSearchLength;
            
            if (--auN <= 0)
            {
                break;
            }
        }
    
        return lpcLast == NULL || auN != 0 ? -1 : (apl_ssize_t)(lpcLast - apcInput);
    }

    return -1;
}

inline void KMPGetNext( char const* apcPattern, apl_int_t* apiVal, apl_size_t auN )
{
	apl_int_t liI = 0;
	apl_int_t liJ = -1;
	apiVal[0] = -1;

	while( liI < (apl_int_t)auN - 1 )
	{
		if (liJ == -1 || apcPattern[liI] == apcPattern[liJ])
		{
			++liI;
			++liJ;

			if (apcPattern[liI] != apcPattern[liJ])
			{
				apiVal[liI] = liJ;
			}
			else
			{
				apiVal[liI] = apiVal[liJ];
			}
		}
		else
		{
			liJ = apiVal[liJ];
		}
	}
}

apl_ssize_t KMPFind( std::string const& aoInput, std::string const& aoSearch )
{
    return KMPFind(aoInput.c_str(), aoInput.length(), aoSearch.c_str(), aoSearch.length() );
}

apl_ssize_t KMPFind( char const* apcInput, const char* apcSearch )
{
    return KMPFind(apcInput, apcSearch, apl_strlen(apcSearch) );
}

apl_ssize_t KMPFind( char const* apcInput, apl_size_t auLength, const char* apcSearch )
{
    return KMPFind(apcInput, auLength, apcSearch, apl_strlen(apcSearch) );
}

apl_ssize_t KMPFind( char const* apcInput, const char* apcSearch, apl_size_t auSearchLen )
{
    apl_int_t* lpiVal = NULL;
    apl_size_t luN = 0;
    apl_int_t  liI = 0;
    
    ACL_NEW_N_ASSERT(lpiVal, apl_int_t, auSearchLen);
    
    KMPGetNext(apcSearch, lpiVal, auSearchLen);
    
    for ( luN = 0; apcInput[luN] != '\0' && liI < (apl_int_t)auSearchLen; luN++ )
    {
        while(true)
        {
            if (liI == -1 || apcInput[luN] == apcSearch[liI])
            {
                liI++;
                break;
            }
            else
            {
                liI = lpiVal[liI];
            }
        }
    }
    
    ACL_DELETE_N(lpiVal);
    
    return liI == (apl_int_t)auSearchLen ? (apl_ssize_t)(luN - liI) : -1;
}

apl_ssize_t KMPFind( char const* apcInput, apl_size_t auLength, const char* apcSearch, apl_size_t auSearchLen )
{
    apl_int_t* lpiVal = NULL;
    apl_size_t luN = 0;
    apl_int_t  liI = 0;
    
    ACL_NEW_N_ASSERT(lpiVal, apl_int_t, auSearchLen);
    
    KMPGetNext(apcSearch, lpiVal, auSearchLen);
    
    for ( luN = 0; luN < auLength && liI < (apl_int_t)auSearchLen; luN++ )
    {
        while(true)
        {
            if (liI == -1 || apcInput[luN] == apcSearch[liI])
            {
                liI++;
                break;
            }
            else
            {
                liI = lpiVal[liI];
            }
        }
    }
    
    ACL_DELETE_N(lpiVal);
    
    return liI == (apl_int_t)auSearchLen ? (apl_ssize_t)(luN - liI) : -1;
}

/*
 * Replace
 */
void ReplaceRangeCopy(
    std::string const& aoInput,
    apl_size_t auPos,
    apl_size_t auLen,
    std::string const& aoRep,
    std::string& aoResult )
{
    aoResult.clear();
    aoResult.reserve( aoInput.size() );
    
    if ( auPos + auLen <= aoInput.size() )
    {    
        //Copy First
        aoResult.assign(aoInput, 0, auPos);
        
        //Replace
        aoResult.append(aoRep);
        
        //Tail Last
        aoResult.append(aoInput, auPos + auLen, aoInput.size() - auPos - auLen);
    }
    else if ( auPos >= aoInput.size() )
    {
        aoResult = aoInput;
    }
    else
    {
        //Copy First
        aoResult.assign(aoInput, 0, auPos);
        
        aoResult += aoRep;
    }
}

void ReplaceRangeCopy(
    char const* apcInput,
    apl_size_t auPos,
    apl_size_t auLen,
    std::string const& aoRep,
    char* apcBuff,
    apl_size_t auSize )
{
    apl_size_t luLength = apl_strlen(apcInput);
    apl_size_t luN = 0;
    //Copy first
    for (; luN < auPos && luN < luLength && luN < auSize; luN++)
    {
        apcBuff[luN] = apcInput[luN];
    }
    
    //Copy replace
    for (apl_size_t liRN = 0; liRN < aoRep.size() && luN < auSize; liRN++, luN++)
    {
        apcBuff[luN] = aoRep[liRN];
    }
    
    //Copy last
    for (apl_size_t liLN = auPos + auLen; liLN < luLength && luN < auSize; liLN++, luN++)
    {
        apcBuff[luN] = apcInput[liLN];
    }
    
    apcBuff[luN] = '\0';
}

void ReplaceFirstCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    std::string& aoResult )
{
    apl_ssize_t liFirst = FindFirst(aoInput, aoSearch);
    if (liFirst != -1)
    {
        ReplaceRangeCopy(aoInput, liFirst, aoSearch.size(), aoRep, aoResult);
    }
    else
    {
        aoResult = aoInput;
    }
}

void ReplaceFirstCopy(
    char const* apcInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    char* apcBuff,
    apl_size_t auSize )
{
    apl_ssize_t liFirst = FindFirst(apcInput, aoSearch);
    if (liFirst != -1)
    {
        ReplaceRangeCopy(apcInput, liFirst, aoSearch.size(), aoRep, apcBuff, auSize);
    }
    else if (apcInput != APL_NULL && apcBuff != APL_NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
    }
}

void ReplaceLastCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    std::string& aoResult )
{
    apl_ssize_t liLast = FindLast(aoInput, aoSearch);
    if (liLast != -1)
    {
        ReplaceRangeCopy(aoInput, liLast, aoSearch.size(), aoRep, aoResult);
    }
    else
    {
        aoResult = aoInput;
    }
}

void ReplaceLastCopy(
    char const* apcInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    char* apcBuff,
    apl_size_t auSize )
{
    apl_ssize_t liLast = FindLast(apcInput, aoSearch);
    if (liLast != -1)
    {
        ReplaceRangeCopy(apcInput, liLast, aoSearch.size(), aoRep, apcBuff, auSize);
    }
    else if (apcInput != APL_NULL && apcBuff != APL_NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
    }
}

void ReplaceCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    std::string& aoResult )
{
    std::string::size_type liCurr = 0;
    std::string::size_type liNext = 0;
        
    aoResult.clear();
    aoResult.reserve( aoInput.size() );
    
    while(true)
    {
        liNext = aoInput.find(aoSearch, liCurr);
        if (liNext == std::string::npos)
        {
            aoResult.append(aoInput, liCurr, aoInput.size() - liCurr);
            break;
        }
        else
        {
            aoResult.append(aoInput, liCurr, liNext - liCurr);
            aoResult.append(aoRep);
            liCurr = liNext + aoSearch.size();
        }
    }
}

void ReplaceCopy(
    char const* apcInput,
    std::string const& aoSearch,
    std::string const& aoRep,
    char* apcBuff,
    apl_size_t auSize )
{
    char const* lpcCurr = apcInput;
    char const* lpcNext = NULL;
    char*       lpcResult = apcBuff;
        
    while(true)
    {
        lpcNext = apl_strstr( lpcCurr, aoSearch.c_str() );
        if (lpcNext == NULL)
        {
            apl_strncpy(lpcResult, lpcCurr, auSize - (lpcResult - apcBuff) );
            break;
        }
        else if ( lpcNext - lpcCurr + aoRep.size() < auSize - (lpcResult - apcBuff) )
        {
            apl_memcpy(lpcResult, lpcCurr, lpcNext - lpcCurr );
            lpcResult += (lpcNext - lpcCurr);
            
            apl_memcpy(lpcResult, aoRep.c_str(), aoRep.size() );
            lpcResult += aoRep.size();
            
            *lpcResult = '\0';

            lpcCurr = lpcNext + aoSearch.size();
        }
        else if ( apl_size_t(lpcNext - lpcCurr) >= auSize - (lpcResult - apcBuff) )
        {
            apl_memcpy(lpcResult, lpcCurr, auSize - (lpcResult - apcBuff) - 1 );
            lpcResult += (auSize - (lpcResult - apcBuff) - 1);
            *lpcResult = '\0';
            break;
        }
        else
        {
            apl_memcpy(lpcResult, lpcCurr, lpcNext - lpcCurr );
            lpcResult += (lpcNext - lpcCurr);
            
            apl_memcpy(lpcResult, aoRep.c_str(), auSize - (lpcResult - apcBuff) - 1 );
            lpcResult += (auSize - (lpcResult - apcBuff) - 1);
            
            *lpcResult = '\0';
            break;
        }
    }
}

void ReplaceNthCopy(
    std::string const& aoInput,
    std::string const& aoSearch,
    apl_size_t auN,
    std::string const& aoRep,
    std::string& aoResult )
{
    apl_ssize_t liNth = FindNth(aoInput, aoSearch, auN);
    if (liNth != -1)
    {
        ReplaceRangeCopy(aoInput, liNth, aoSearch.size(), aoRep, aoResult);
    }
    else
    {
        aoResult = aoInput;
    }
}

void ReplaceNthCopy(
    char const* apcInput,
    std::string const& aoSearch,
    apl_size_t auN,
    std::string const& aoRep,
    char* apcBuff,
    apl_size_t auSize )
{
    apl_ssize_t liNth = FindNth(apcInput, aoSearch, auN);
    if (liNth != -1)
    {
        ReplaceRangeCopy(apcInput, liNth, aoSearch.size(), aoRep, apcBuff, auSize);
    }
    else if (apcInput != APL_NULL && apcBuff != APL_NULL)
    {
        apl_strncpy(apcBuff, apcInput, auSize);
    }
}

/*
 * Compare
 */
bool StartWith( char const* apcInput, apl_size_t aiInputLen, char const* apcTest, apl_size_t aiTestLen )
{
    if ( aiTestLen <= aiInputLen )
    {
        if (apl_strncmp( apcInput, apcTest, aiTestLen ) == 0)
        {
            return true;
        }
    }
    
    return false;
}

bool EndWith( char const* apcInput, apl_size_t aiInputLen, char const* apcTest, apl_size_t aiTestLen )
{
    if ( aiTestLen <= aiInputLen )
    {
        if (apl_strncmp( apcInput + (aiInputLen - aiTestLen), apcTest, aiTestLen ) == 0)
        {
            return true;
        }
    }
    
    return false;
}

bool StartWith( std::string const& aoInput, std::string const& aoTest )
{
    return StartWith( aoInput.c_str(), aoInput.size(), aoTest.c_str(), aoTest.size() );
}

bool StartWith( std::string const& aoInput, char const* apcTest )
{
    if (apcTest != APL_NULL)
    {
        return StartWith( aoInput.c_str(), aoInput.size(), apcTest, apl_strlen(apcTest) );
    }
    else
    {
        return false;
    }
}

bool StartWith( char const* apcInput, char const* apcTest )
{
    if (apcInput != APL_NULL && apcTest != APL_NULL)
    {
        return StartWith( apcInput, apl_strlen(apcInput), apcTest, apl_strlen(apcTest) );
    }
    else
    {
        return false;
    }
}

bool StartWith( char const* apcInput, std::string const& aoTest )
{
    if (apcInput != APL_NULL)
    {
        return StartWith( apcInput, apl_strlen(apcInput), aoTest.c_str(), aoTest.size() );
    }
    else
    {
        return false;
    }
}

bool EndWith( std::string const& aoInput, std::string const& aoTest )
{
    return EndWith( aoInput.c_str(), aoInput.size(), aoTest.c_str(), aoTest.size() );
}

bool EndWith( std::string const& aoInput, char const* apcTest )
{
    if (apcTest != APL_NULL)
    {
        return EndWith( aoInput.c_str(), aoInput.size(), apcTest, apl_strlen(apcTest) );
    }
    else
    {
        return false;
    }
}

bool EndWith( char const* apcInput, char const* apcTest )
{
    if (apcInput != APL_NULL && apcTest != APL_NULL)
    {
        return EndWith( apcInput, apl_strlen(apcInput), apcTest, apl_strlen(apcTest) );
    }
    else
    {
        return false;
    }
}

bool EndWith( char const* apcInput, std::string const& aoTest )
{
    if (apcInput != APL_NULL)
    {
        return EndWith( apcInput, apl_strlen(apcInput), aoTest.c_str(), aoTest.size() );
    }
    else
    {
        return false;
    }
}

bool Contain( std::string const& aoInput, std::string const& aoTest )
{
    return Find(aoInput, aoTest) >= 0 ? true : false;
}

bool Contain( std::string const& aoInput, char const* apcTest )
{
    return Find(aoInput.c_str(), apcTest) >= 0 ? true : false;
}

bool Contain( char const* apcInput, char const* apcTest )
{
    return Find(apcInput, apcTest) >= 0 ? true : false;
}

bool Contain( char const* apcInput, std::string const& aoTest )
{
    return Find( apcInput, aoTest.c_str() ) >= 0 ? true : false;
}

bool Equal( std::string const& aoInput, std::string const& aoTest )
{
    return aoInput == aoTest;
}

bool Equal( std::string const& aoInput, char const* apcTest )
{
    return aoInput == apcTest;
}

bool Equal( char const* apcInput, char const* apcTest )
{
    if (apcInput != APL_NULL && apcTest != APL_NULL)
    {
        return apl_strcmp(apcInput, apcTest) == 0 ? true : false;
    }
    else
    {
        return false;
    }
}

bool Equal( char const* apcInput, std::string const& aoTest )
{
    return aoTest == apcInput;
}

} //end stralgo namespace

ACL_NAMESPACE_END

