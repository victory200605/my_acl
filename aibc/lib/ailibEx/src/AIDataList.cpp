/***************************************************************************/
/*  Program ID : datalist.cpp                                              */
/*  Description: The clsDataList class and AddrPath class implement module */
/*               The clsDataList class has the ability of unlimited buffers*/
/*               for large stream like values.                             */
/*  Functions  : clsDataList::clsDataList(void)                         */
/*               clsDataList::~clsDataList ()                              */
/*               int clsDataList::ciGetCount(void)                      */
/*               clsDataList::cvClean(void)                             */
/*               int clsDataList::ciAppend (char *asValue,int aiSize = 0)  */
/*               int clsDataList::ciInsert(                             */
/*                 const char *asBuffer, int iIndex, int aiSize = 0)      */
/*               int clsDataList::ciDelete(int iIndex)                  */
/*               char *clsDataList::csGetItem (int iIndex, int *apiSize = NULL)*/
/*               char *clsDataList::csGetMerge(int *apiSize = NULL)     */
/*               int clsDataList::ciSeek(const char *asValue,int iCaseSens)*/
/*               void clsDataList::operator+=(class clsDataList &pOtherObj)*/
/*  Update Log :                                                           */
/*  Date     Author  Description                                           */
/*  ---------------------------------------------------------------------- */
/*  98.08.21 jeikul  First release                                         */
/*                                                                         */
/***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include "AIDataList.h"

///start namespace
AIBC_NAMESPACE_START

/*
  ()               -> NULL NULL
  ("")             -> () NULL
  (abc)            -> (abc) NULL
  (a b c)          -> (a) (b) (c) NULL
  ("a b" c)        -> (a b) (c) NULL
  ("a, b", c),     -> (a, b) (c) NULL
  ("a b"c d)       -> (a bc) (d) NULL
  ("\"a b\""c d)   -> ("a b"c) (d) NULL
  ("\\\"a ,b"c,d), -> (\"a ,bc) (d) NULL
*/
const char * gpcParseString (register char *asResult, int aiResultSize,
  const char *asSource, int aiIndex, const char **appcNext = NULL, char acDelimitor = 0)
{
    int liQuoted, liSize = 0, liSpec = 0 ;
    const char *lpcNext = asSource, *lpcResult = asResult, *lpcRet = NULL ;

    if (asSource != NULL) 
    {
        for (; aiIndex >= 0; aiIndex --) 
        {
            while (isspace(*lpcNext))
            {
                lpcNext ++; // skip the leading spaces of each sub string
            }
            if ((liQuoted = *lpcNext == '\"'))
            {
                lpcNext ++ ;
            }
            for (; *lpcNext != '\0' && (liQuoted || (acDelimitor ? *lpcNext != acDelimitor : !isspace(*lpcNext))); lpcNext++)
            {
                if (aiIndex == 0) 
                {
                    lpcRet = lpcResult ; /* return value here, point to the buffer */
                    if (*lpcNext == '\\' && (*(lpcNext + 1) == '\"' || *(lpcNext + 1) == '\\'))
                    {
                        liSpec = ! liSpec ;
                    }
                    else
                    {
                        liSpec = 0 ;
                    }
                    if (liSpec == 0 && liSize < aiResultSize - 1 /* the result buffer must be large enough */
                        && (*lpcNext != '\"' || lpcNext > asSource && *(lpcNext - 1) == '\\'))
                    {
                        *asResult ++ = *lpcNext, liSize ++ ; /* copy the sub string byte by byte */
                    }
                }
                if (*lpcNext == '\"' && lpcNext > asSource && *(lpcNext - 1) != '\\')
                {
                    liQuoted = (liQuoted == 0 ? 1 : 0); /* whether in a quoted sub string ? */
                }
            }
            if (*lpcNext != '\0' && *lpcNext == acDelimitor)
            {
                lpcNext ++ ;
            }
        } /* for */
        while (isspace(*lpcNext))
        {
            lpcNext ++;
        }
    }
    *asResult = '\0' ;
    if (appcNext != NULL)
    {
        *appcNext = lpcNext && *lpcNext ? lpcNext : NULL ;
    }
    return lpcRet ;
}

int clsDataList::ciGetCount(void)
{
    int     liCount;

    if (ciThreadSafe)
    {
        pthread_mutex_lock(&ctLock);
    }

    liCount = ciCount;
    
    if (ciThreadSafe)
    {
        pthread_mutex_unlock(&ctLock);
    }

    return  (liCount);
}

/***************************************************************************/
/*  Func Name  : clsDataList ::clsDataList                                 */
/*  Description: The String class constructor                              */
/*  Parameters : none                                                      */
/*  Return Val : none                                                      */
/*  Update Log :                                                           */
/*  Date     Author  Description                                           */
/*  ---------------------------------------------------------------------- */
/*  98.08.24 jeikul  First release                                         */
/*                                                                         */
/***************************************************************************/
clsDataList::clsDataList(int aiThreadSafe , int aiStep , int aiMaxMemory)
{
    ciSize = ciCount = 0;                  /* The items counter */
    ciThreadSafe = aiThreadSafe;
    ciListSize = ciStep = aiStep > 0 ? aiStep : DL_DEFAULT_STEP;
    ciMaxMemory = aiMaxMemory > 0 ? aiMaxMemory : 0;

    cpList = (DataItem_t *)malloc(ciListSize * sizeof (*cpList));
    assert(cpList != NULL);
    
    if (ciThreadSafe)
    {/* The share lock */
        int liLockRet = pthread_mutex_init(&ctLock, NULL);
        assert(0 == liLockRet); 
    }
}

/***************************************************************************/
/*  Func Name  : clsDataList ::~clsDataList                                */
/*  Description: The String class distructor                               */
/*  Parameters : none                                                      */
/*  Return Val : none                                                      */
/*  Update Log :                                                           */
/*  Date     Author  Description                                           */
/*  ---------------------------------------------------------------------- */
/*  98.08.24 jeikul  First release                                         */
/*                                                                         */
/***************************************************************************/
clsDataList::~clsDataList()
{
    cvClean();                           /* Release the chain */
    free(cpList);
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_destroy(&ctLock);
        assert(0 == liLockRet);
    }
}

/***************************************************************************/
/*  Func Name  : clsDataList ::vClean                                      */
/*  Description: Free the chain, release the memory                        */
/*  Parameters : none                                                      */
/*  Return Val : none                                                      */
/*  Update Log :                                                           */
/*  Date     Author  Description                                           */
/*  ---------------------------------------------------------------------- */
/*  98.08.24 jeikul  First release                                         */
/*                                                                         */
/***************************************************************************/
void clsDataList::cvClean(void)
{
    int     i;
    DataItem_t *lpDataItem, *lpNewList;

    if (ciThreadSafe)
    {  /* Lock the whole chain */
        int liLockRet= pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet); 
    }
        
    for (i = 0, lpDataItem = cpList; i < ciCount; i ++, lpDataItem ++)
    {
        if (lpDataItem->pBuffer != NULL)
        {
            free((void *) lpDataItem->pBuffer);
        }
    }
    
    if (ciListSize != ciStep)
    {
        if ((lpNewList = (DataItem_t *) malloc(ciStep * sizeof (*cpList))) != NULL) 
        {
            free(cpList);
            cpList = lpNewList;
            ciListSize = ciStep;
        }
    }
    
    ciSize = ciCount = 0;
    
    if (ciThreadSafe)
    { /* Release the lock */
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
}

/***************************************************************************/
/*  Func Name  : clsDataList ::iAppend, iInsert                            */
/*  Description: Add a new node to the chain                               */
/*  Parameters : asValue,char*,pointing to the node buffer                 */
/*               aiSize,int,indicating the buffer size, 0 means the buffer */
/*                          is a null terminated strings.                  */
/*               iIndex,int,position to add the new node                   */
/*  Return Val : 0,success                                                 */
/*               -1,not enough memory                                      */
/*  Update Log :                                                           */
/*  Date     Author  Description                                           */
/*  ---------------------------------------------------------------------- */
/*  98.08.24 jeikul  First release                                         */
/*                                                                         */
/***************************************************************************/
int clsDataList::ciAppend(const void *asBuffer, int aiSize)
{
    return ciInsert(asBuffer, -1, aiSize);
}

int clsDataList::ciInsert(const void *asBuffer, int aiIndex, int aiSize ,
    int aiPlaced , int aiIgnoreThread)
{
    int         liRetCode = 0;
    DataItem_t  *lpNewList, ltNewItem;

    if (asBuffer == NULL)
    {
        return -1;
    }

    if (ciThreadSafe && ! aiIgnoreThread)
    {
        int liLockRet= pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    if (aiSize == 0)
    {
        aiSize = strlen((const char *) asBuffer);
    }
    
    ltNewItem.iSize = aiSize;
    if (aiIndex < 0 || aiIndex > ciCount)
    {
        aiIndex = ciCount;
    }

    if (ciCount == ciListSize)
    {
        if ((lpNewList = (DataItem_t *) malloc((ciListSize + ciStep) * sizeof (*cpList))) == NULL)
        {
            liRetCode = -1;
        }
        else 
        {
            memmove(lpNewList, cpList, ciListSize * sizeof (*cpList));
            free(cpList);
            cpList = lpNewList;
            ciListSize += ciStep;
        }
    }
    
    if (liRetCode == 0) 
    {
        if (aiPlaced != 0)
        {
            ltNewItem.pBuffer = (void *) asBuffer;
        }
        else
        {
            ltNewItem.pBuffer = malloc(aiSize + 1);
        }

        if (ltNewItem.pBuffer != NULL) 
        {
            if (aiPlaced == 0) 
            {
                memmove((void *) ltNewItem.pBuffer, asBuffer, aiSize);
                *((char *) ltNewItem.pBuffer + aiSize) = '\0';
            }
            
            memmove(cpList + aiIndex + 1, cpList + aiIndex, (ciCount - aiIndex) * sizeof (*cpList));
            memmove(cpList + aiIndex, &ltNewItem, sizeof (ltNewItem));
            ciCount ++;
            ciSize += ltNewItem.iSize;
        } 
        else
        {
            liRetCode = -1;
        }
    }
    
    if (ciThreadSafe && ! aiIgnoreThread)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (liRetCode);
}

int clsDataList::ciPlaced(const void *asBuffer, int aiIndex, int aiSize)
{
    return ciInsert(asBuffer, aiIndex, aiSize, 1);
}
 
int clsDataList::ciUpdate(const void *asBuffer, int aiIndex, int aiSize)
{
    int     liRetCode = -1;

    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    if (aiIndex >= 0 && aiIndex < ciCount) 
    {
        free((void *) (cpList + aiIndex)->pBuffer);
        ciSize -= (cpList + aiIndex)->iSize;
        
        if (aiSize == 0)
            aiSize = strlen((char *) asBuffer);
            
        (cpList + aiIndex)->pBuffer = malloc(aiSize + 1);

        assert((cpList + aiIndex)->pBuffer != NULL);

        * ((char *) (cpList + aiIndex)->pBuffer + aiSize) = '\0';
        memmove((void *) (cpList + aiIndex)->pBuffer, asBuffer, aiSize);
        ciSize += ((cpList + aiIndex)->iSize = aiSize);
        liRetCode = 0;
    }
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (liRetCode);
}

/***************************************************************************/
/*  Func Name  : clsDataList ::ciDelete                                    */
/*  Description: Delete a specified node from the chain                    */
/*  Parameters : iIndex,int,the index of the node to be delete, 0 means the*/
/*                          first node                                     */
/*  Return Val : 0,success                                                 */
/*               -1,iIndex is not valid                                    */
/*  Update Log :                                                           */
/*  Date     Author  Description                                           */
/*  ---------------------------------------------------------------------- */
/*  98.08.24 jeikul  First release                                         */
/*                                                                         */
/***************************************************************************/
int clsDataList::ciDelete(int aiIndex)
{
    int     liRetCode = 0;

    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    if (aiIndex < 0 || aiIndex >= ciCount)
    {
        liRetCode = -1; /* iIndex is not valid */
    }
    else 
    {
        ciSize -= (cpList + aiIndex)->iSize;
        free((void *) (cpList + aiIndex)->pBuffer);
        ciCount --;
        memmove(cpList + aiIndex, cpList + aiIndex + 1, (ciCount - aiIndex) * sizeof (*cpList));
    }
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (liRetCode);
}

int clsDataList::ciDelete(const void *asBuffer, int aiSize)
{
    int     liRetCode = 0, liIndex;

    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    for (liIndex = 0; liIndex < ciCount; liIndex ++) 
    {
        if (aiSize == 0) 
        {
            if (strcmp((char *) asBuffer, (char *) (cpList + liIndex)->pBuffer) == 0)
            {
                break;
            }
        } 
        else 
        {
            if (memcmp(asBuffer, (cpList + liIndex)->pBuffer, aiSize) == 0)
            {
                break;
            }
        }
    }
    
    if (liIndex < 0 || liIndex >= ciCount)
    {
        liRetCode = -1; /* iIndex is not valid */
    }
    else 
    {
        ciSize -= (cpList + liIndex)->iSize;
        free((void *) (cpList + liIndex)->pBuffer);
        ciCount --;
        memmove(cpList + liIndex, cpList + liIndex + 1, (ciCount - liIndex) * sizeof (*cpList));
    }
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (liRetCode);
}

// add by jack ------------------------------------------ 2002-01-29

// Description:
//   To delete a batch of nodes in one time and provide moving too much data.
// Parameters:
//   apKey: the value which would used as the first parameter of the apfCompare()
//   apfCompare: would ues this function to compare each node. Would remove the node which return 0 (matched).
//   aiFirstIndex: the first Index of the area want to check 
//   aiSize: the node count of the area want to check
//   Note: if aiSize not specified then check the whole datalist
// Return: return the removed node count. -1 means error.
//
// Author: Jack Liang 2002-01-29

int clsDataList::ciBatchDelete(const void *apKey, int (*apfCompare)(const  void  *,  const  void *), int aiFirstIndex, int aiSize)
{
    if (aiSize == 0) 
    {   
        return 0;   
    }
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    // if aiSize not specify, then process the whold datalist
    if (aiSize == -1) 
    { 
        aiFirstIndex = 0; 
        aiSize = ciCount; 
    }
    
    if (aiFirstIndex < 0 || aiFirstIndex >= ciCount 
            || aiSize < 0 || aiFirstIndex + aiSize > ciCount) 
    {   
        return -1;  // the scope not correct
    }
    
    if (apfCompare == NULL) 
    {
        // the default compare function is "strcmp()"
        apfCompare = (int (*) (const void*, const void*)) strcmp;   
    }

    int     liDeleteCount = 0;
    int     liDeleteSize = 0;
    DataItem_t  *chp, *chp2;

    chp = chp2 = cpList + aiFirstIndex;
    for (int i = 0; i < aiSize; i++) 
    {
        if (((*apfCompare)(apKey, (void *)(chp->pBuffer))) == 0) // matched
        { 
            liDeleteCount++;
            liDeleteSize += chp->iSize;
            free((void *)(chp->pBuffer));
        } 
        else 
        {
            if (chp != chp2) 
            {
                memcpy(chp2, chp, sizeof(DataItem_t));
            }
            chp2++;
        }
        chp++;
    }
    
    if (chp < cpList + ciCount && chp != chp2) 
    {
        memmove(chp2, chp, (cpList + ciCount - chp) * sizeof(DataItem_t));
    }
    
    ciSize -= liDeleteSize;
    ciCount -= liDeleteCount;
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (liDeleteCount);
}
// add by jack ------------------------------------------ 2002-01-29 == end

/***************************************************************************/
/*  Func Name  : clsDataList::sGet                                         */
/*  Description: Delete a specified node from the chain                    */
/*  Parameters : iIndex,int,the index of the node to be retrieved, 0 means */
/*                          the first node                                 */
/*  Return Val : !=NULL, success & pointing to the specified node buffer   */
/*               NULL,iIndex is not valid                                  */
/*  Update Log :                                                           */
/*  Date     Author  Description                                           */
/*  ---------------------------------------------------------------------- */
/*  98.08.24 jeikul  First release                                         */
/*                                                                         */
/***************************************************************************/
const void * clsDataList::csGetItem(int aiIndex, int *apiSize)
{
    const void  *lpRet = NULL;

    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    if (aiIndex == DL_LAST_INDEX)
    {
        aiIndex = ciCount - 1;
    }

    if (aiIndex >= 0 && aiIndex < ciCount) 
    {
        if (apiSize != NULL)
        {
            *apiSize = (cpList + aiIndex)->iSize;
        }
        lpRet = (cpList + aiIndex)->pBuffer;
    }
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (lpRet);
}

// use a buffer to contain the data which in datalist, then remove the node from the datalist
int clsDataList::ciTakeItem(void *asBuffer, int aiBufferSize, int aiIndex)
{
    int     liRetCode = -1, liSize;

    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    if (aiIndex == DL_LAST_INDEX)
        aiIndex = ciCount - 1;
        
    if (aiIndex >= 0 && aiIndex < ciCount) 
    {
        liSize = aiBufferSize >
            (cpList + aiIndex)->iSize + 1 ? (cpList + aiIndex)->iSize + 1 : aiBufferSize;
        
        memmove(asBuffer, (cpList + aiIndex)->pBuffer, liSize);
        /* remove the selected node */
        ciSize -= (cpList + aiIndex)->iSize;
        free((void *) (cpList + aiIndex)->pBuffer);
        ciCount --;
        memmove(cpList + aiIndex, cpList + aiIndex + 1, (ciCount - aiIndex) * sizeof (*cpList));
        liRetCode = 0;
    }
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (liRetCode);
}

// get the buffer which is the in the datalist, and remove it from the link(not free it)
const void * clsDataList::csTakeItem(int aiIndex, int *apiSize)
{
    const void *lpRet = NULL;

    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    if (aiIndex >= 0 && aiIndex < ciCount) 
    {
        lpRet = (cpList + aiIndex)->pBuffer;
        if (apiSize != NULL)
        {
            *apiSize = (cpList + aiIndex)->iSize;
        }
        /* remove the selected node */
        ciSize -= (cpList + aiIndex)->iSize;
        ciCount --;
        memmove(cpList + aiIndex, cpList + aiIndex + 1, (ciCount - aiIndex) * sizeof (*cpList));
    }
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (lpRet);
}

/***************************************************************************/ 
/*  Func Name  : clsDataList ::ciSeek                                      */ 
/*  Description: Search the chain and return the index of the node         */
/*  Parameters : asValue,char*,the specified value                         */
/*               iCaseSens,int,0-case insensitive compare                  */
/*  Return Val : >=0, Success & index of the node returned                 */
/*               <0,No match record found                                  */
/*  Update Log :                                                           */ 
/*  Date     Author  Description                                           */ 
/*  ---------------------------------------------------------------------- */ 
/*  98.08.24 jeikul  First release                                         */ 
/*                                                                         */
/***************************************************************************/
int clsDataList::ciSeek(const char *asValue, int aiCaseSens)
{
    int     i = 0, liRetCode = -1, liResult;

    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    for (i = 0; i < ciCount; i ++) 
    {
        if (aiCaseSens)
        {
            liResult = strcmp(asValue, (const char *) (cpList + i)->pBuffer);
        }
        else
        {
            liResult = strcasecmp(asValue, (const char *) (cpList + i)->pBuffer);
        }

        if (liResult == 0) 
        {
            liRetCode = i;
            break;
        }
    }
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (liRetCode);
}


/***************************************************************************/ 
/*  Func Name  : clsDataList ::cinSeek                                      */ 
/*  Description: Search the chain and return the index of the node         */
/*  Parameters : asValue,char*,the specified value                         */
/*               iCaseSens,int,0-case insensitive compare                  */
/*  Return Val : >=0, Success & index of the node returned                 */
/*               <0,No match record found                                  */
/*  Update Log :                                                           */ 
/*  Date     Author  Description                                           */ 
/*  ---------------------------------------------------------------------- */ 
/*  2001.04.25 Cwc  Modified from ciSeek() to fix Bugfix: 633              */ 
/*                                                                         */
/***************************************************************************/
int clsDataList::cinSeek(const char *asValue, int aiCaseSens)
{
    int     i = 0, liRetCode = -1, liResult, iLen=strlen(asValue);
    const char *sTmp=NULL;

    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    for (i = 0; i < ciCount; i ++) 
    {
        //Bugfix: 633 change strcmp/strcasecmp to strncmp/strncasecmp
        sTmp = (const char *) (cpList + i)->pBuffer;
    
        if (aiCaseSens)
        {
            liResult = strncmp(asValue, sTmp , iLen);
        }
        else
        {
            liResult = strncasecmp(asValue, sTmp, iLen);
        }    

        if (liResult == 0 && (*(sTmp+iLen)==' ' || *(sTmp+iLen)=='\0')) 
        {
            liRetCode = i;
            break;
        }
    }
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (liRetCode);
}


/***************************************************************************/ 
/*  Func Name  : AddrPath::operator +=                                     */ 
/*  MT-Level   : Unsafe                                                    */
/*  Description: concat two clsDataList class type chain into one.         */
/*  Parameters : pOtherObj,class AddrPath &,the chain to be added          */
/*  Return Val : none                                                      */
/*  Update Log :                                                           */ 
/*  Date     Author  Description                                           */ 
/*  ---------------------------------------------------------------------- */ 
/*  98.08.24 jeikul  First release                                         */ 
/*                                                                         */
/***************************************************************************/
void clsDataList::operator +=(class clsDataList &apDataList)
{
    int         liSize;
    const void  *lpData;

    for (int i = 0; i < apDataList.ciGetCount (); i ++) 
    {
        lpData = apDataList.csGetItem(i, &liSize);
        ciAppend(lpData, liSize);
    }
}

const void * clsDataList::csGetMerge(int *apiSize , int aiSeparator)
{
    int         liSize, i;
    const void  *lpBuffer;
    char        *p;

    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    lpBuffer = p = (char *) malloc(ciSize + 1 + (aiSeparator != '\0' ? ciCount : 0));
    if (lpBuffer != NULL) 
    {
        for (i = 0; i < ciCount; i ++) 
        {
            liSize = (cpList + i)->iSize;
            /*q = csGetItem (i, &liSize);*/
            memmove(p, (cpList + i)->pBuffer, liSize);
            p += liSize;
            if (aiSeparator != '\0')
            {
                *p ++ = aiSeparator;
            }
        }
        
        if (apiSize != NULL)
        {
            *apiSize = ciSize + (aiSeparator != '\0' ? ciCount : 0);
        }
        if (aiSeparator != '\0' && ciCount > 0)
        {
            p--;
        }
        *p = '\0';
    }
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (lpBuffer);
}

int clsDataList::ciGetSize(void)
{
    int     liSize;

    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    liSize = ciSize;
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (liSize);
}

class clsDataList *gpStringToList(const char *asBuffer, int aiThreadMode ,
  int aiMaxItems , char const ** appcNext)
{
    char            lsClause [256];
    const char      *lpcNext;
    class clsDataList *lpRet = NULL;

    for (; gpcParseString(lsClause, sizeof (lsClause), asBuffer, 0, &lpcNext) != NULL; asBuffer = lpcNext) 
    {
        if (lpRet == NULL)
        {
            lpRet = new clsDataList(aiThreadMode);
        }

        if (lpRet != NULL && (aiMaxItems <= 0 || lpRet->ciGetCount () < aiMaxItems))
        {
            lpRet->ciAppend(lsClause);
        }

        if (lpRet == NULL || aiMaxItems > 0 && lpRet->ciGetCount () >= aiMaxItems)
        {
            break;
        }
    }
    
    if (appcNext != NULL)
    {
        *appcNext = lpcNext;
    }

    return  (lpRet);
}

// Description:
//         The ciSort() function is an implementation of the  quick-sort
//         algorithm.  It sorts the data of datalist in place.  The contents
//         of the table are sorted in ascending order according to  the
//         user-supplied comparison function.
// parameters:
//         apfCompare: a function to compare two node in the datalist
//             The function must return an integer less than, equal to,  or
//             greater than zero to indicate if the first argument is to be
//             considered less than, equal to, or greater than  the  second
//             argument.
//         aiFirstIndex: the first Index of the area want to sort
//         aiSize: the node count of the area want to sort
//   Note: if apfCompare not specify, the string compare function would be used(strcmp())
//         if aiFirstIndex & aiSize not specify, then sort the whold datalist
// Author: Jackie Liang 
void clsDataList::cvSort(int (*apfCompare) (const void *, const void *) ,
int aiFirstIndex , int aiSize)
{
    if (aiSize == -1)  // if aiSize not specify, then sort the whold datalist
    {
        assert((aiSize = ciGetCount()) >= 0);
    }

    if (aiSize < 2) 
    {
        return;
    }

    int low = aiFirstIndex, high = aiFirstIndex + aiSize - 1;
    
    if (apfCompare == NULL)
    {
        apfCompare = (int (*)(const void*, const void*)) strcmp;    // the default compare function is "strcmp()"
    }

    const void * pivot, * vp;
    int iSize, iPivot = ciGetCount();
    vp = csGetItem(low, &iSize);

    assert(NULL != vp);  // pivot <== low

    int liAppendRet = ciAppend(vp, iSize);

    assert(0 == liAppendRet);               // use the last node to hold the pivot

    pivot = csGetItem(iPivot);
    assert(NULL != pivot);

    //pivot = asBase[low];
    while(1) 
    {
        while(1) 
        {
            if (low == high)
            {
                goto lable;
            }

            if ((*apfCompare)(pivot, csGetItem(high)) <= 0)
            {
                high--;
            }
            else 
            {
                // low <== high
                vp = csGetItem(high, &iSize);

                assert(NULL != vp);

                int liUpdateRet = ciUpdate(vp, low, iSize);
                assert(0 == liUpdateRet);      
                //asBase[low] = asBase[high];
                low++;
                break;
            }
        }
        
        while(1) 
        {
            if (low == high)
            {
                goto lable;
            }

            if ((*apfCompare)(csGetItem(low), pivot) <= 0)
            //if (ciFolderCmp(asBase[low], pivot) <= 0)
            {
                low++;
            }
            else 
            {
                // high <== low 
                vp = csGetItem(low, &iSize);

                assert(NULL != vp);

                int liUpdateRet = ciUpdate(vp, high, iSize);
                
                assert(0 == liUpdateRet);
                //asBase[high] = asBase[low];
                high--;
                break;
            }
        }
    }
    
lable:
    // low <== pivot 
    vp = csTakeItem(iPivot, &iSize);
    assert(NULL != vp);

    int liUpdateRet = ciUpdate(vp, low, iSize);
    assert(0 == liUpdateRet);      
    free((void*) vp);
    //asBase[low] = pivot;  
    // at here, low == high
    if (low > aiFirstIndex + 1)
    {
        cvSort(apfCompare, aiFirstIndex, low - aiFirstIndex);
    }
        
    low = aiSize + aiFirstIndex - low -1;      // use low to store the size, high point to the pivot //
    if (low > 1)
    {
        cvSort(apfCompare, high + 1, low);
    }
}

void clsDataList::cvClsSort(int (*apfCompare)(const void *, const void *), int aiFirstIndex, int aiSize)
{
    int     liSize = aiSize;
    
    if (liSize == -1)
    {
        liSize = ciGetCount();
        assert(liSize >= 0);
    }

    if (liSize < 2)
    {
        return;
    }

    if (aiFirstIndex == 0 && liSize == ciGetCount()) 
    {
        cvDisorder();
    }

    int     low = aiFirstIndex, high = aiFirstIndex + liSize - 1;
    if (apfCompare == NULL)
    {
        apfCompare = (int(*)(const void*, const void*))strcmp;
    }

    DataItem_t lpPivot;
    lpPivot.pBuffer = (cpList + low)->pBuffer;
    lpPivot.iSize = (cpList + low)->iSize;

    while(1)
    {
        while(1)
        {
            if (low == high)
            {
                goto lable;
            }

            if ((*apfCompare)(lpPivot.pBuffer, csGetItem(high)) <= 0)
            {
                high--;
            }
            else
            {
                cpList[low] = cpList[high];
                low++;
                break;
            }
        }

        while(1)
        {
            if (low == high)
            {
                goto lable;
            }

            if ((*apfCompare)(csGetItem(low), lpPivot.pBuffer) <= 0)
            {
                low++;
            }
            else
            {
                cpList[high] = cpList[low];
                high--;
                break;
            }
        }
    }
    
lable:
    (cpList + low)->pBuffer = lpPivot.pBuffer;
    (cpList + low)->iSize = lpPivot.iSize;

    if (low > aiFirstIndex + 1)
    {
        cvClsSort(apfCompare, aiFirstIndex, low - aiFirstIndex);
    }

    low = liSize + aiFirstIndex - low - 1;

    if (low > 1)
    {
        cvClsSort(apfCompare, high + 1, low);
    }
}

void clsDataList::cvDisorder()
{
    int         li1, li2;
    const void  *lpEx;
    int         liEx;
    
    srandom(time(NULL));
    int         liHashCount = ciCount / 2;
    
    for (int i = 0; i < liHashCount; i ++) 
    {
        li1 = random() % ciCount;
        li2 = random() % ciCount;
        lpEx = (cpList + li1)->pBuffer;
        liEx = (cpList + li1)->iSize;
        (cpList + li1)->pBuffer = (cpList + li2)->pBuffer;
        (cpList + li1)->iSize = (cpList + li2)->iSize;
        (cpList + li2)->pBuffer = lpEx;
        (cpList + li2)->iSize = liEx;
    }
}

int clsDataList::ciSearch(const void *apvSample,
  int (*apfCompare) (const void *, const void *) ,
  int aiSize , int aiPlace , void **appvMatch)
{
    int     liBegin, liEnd, liCur, liResult, liRetCode = -1;

    if (appvMatch != NULL)
    {
        *appvMatch = NULL;
    }

    if (apfCompare == NULL)
    {
        apfCompare = (int (*)(const void *, const void *)) strcmp;
    }

    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_lock(&ctLock);
        assert(0 == liLockRet);
    }
    
    for (liBegin = 0, liEnd = ciCount; liBegin < liEnd;) 
    {
        liCur =(liEnd + liBegin) >> 1;
        liResult = apfCompare(apvSample, cpList[liCur].pBuffer);
        
        if (liResult == 0) 
        {
            liRetCode = liCur;
            
            if (appvMatch != NULL)
            {
                *appvMatch = (void *) cpList [liCur].pBuffer;
            }
            break;
        } 
        else if (liResult < 0) 
        {
            liEnd = liCur;
        } 
        else 
        {
            liBegin = liCur + 1;
        }
    }
    
    if (liRetCode == -1 && aiSize > 0) 
    {
        if (ciInsert(apvSample, liBegin, aiSize, aiPlace, 1) == 0)
        {
            liRetCode = liBegin;
        }
    }
    
    if (ciThreadSafe)
    {
        int liLockRet = pthread_mutex_unlock(&ctLock);
        assert(0 == liLockRet);
    }
    
    return  (liRetCode);
}

// Description:
//         ciSearch() is a dimidiate search function
//         It returns a Index into the datalist indicating where a datum
//         may  be  found  or return -1 if  the  datum  cannot 
//         be found.  The datalist must be previously sorted in increasing 
//         order(use cvSort()) according  to  a  comparison  function  
//         pointed  to  by  apfCompare.
// Parameters:
//         apKey: points to a datum instance to be sought in the datalist.
//         apfCompare: a function to compare two node in the datalist
//             The function must return an integer less than, equal to,  or
//             greater than zero to indicate if the first argument is to be
//             considered less than, equal to, or greater than  the  second
//             argument.
//         aiFirstIndex: the first Index of the area want to search 
//         aiSize: the node count of the area want to search
//         apiInsertPoint: if -1 is returned(can't find the apKey), this Pointer
//             indicating the Index point where the apKey should be insert to 
//             maintain the increasing order.
//   Note: if apfCompare not specify, the string compare function would be used(strcmp())
//         if aiFirstIndex & aiSize not specify, then search the hold datalist
// Author: Jackie Liang 
int clsDataList::ciSearch(const void *apKey, int * apiInsertPoint ,
    int (*apfCompare)(const  void  *,  const  void *) , int aiFirstIndex , int aiSize)
{
    if (aiSize == -1)  // if aiSize not specify, then sort the hold datalist
    {
        aiSize = ciGetCount();
        assert(aiSize >= 0);
    }

    if (aiSize == 0) // can't find the key
    {
        if (apiInsertPoint != NULL)
        {
            *apiInsertPoint = aiFirstIndex;    // set the insert point
        }
        return -1;  // return "can't find"
    }
    
    const void      *pivot;
    int             iCmpRet;
    
    if (apfCompare == NULL)
    {
          apfCompare = (int (*) (const void*, const void*)) strcmp; // the default compare function is "strcmp()"
    }
    
    pivot = csGetItem(aiFirstIndex + (aiSize - 1) / 2);
    assert(NULL != pivot);   // get the dimidiate point
    iCmpRet = (*apfCompare)(apKey, pivot);
    
    if (iCmpRet < 0)
    {
        return(ciSearch(apKey, apiInsertPoint, apfCompare, aiFirstIndex, (aiSize - 1) / 2));
    }
    else if (iCmpRet > 0)
    {
        return(ciSearch(apKey, apiInsertPoint, apfCompare, aiFirstIndex + (aiSize - 1) / 2 + 1,
        aiSize - (aiSize - 1) / 2 - 1));
    }
    else // find it
    {
        return(aiFirstIndex + (aiSize - 1) / 2);
    }
}

///end namespace
AIBC_NAMESPACE_END
