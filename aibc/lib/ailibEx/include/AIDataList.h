/* Header file for AIDataList.cpp
   Copyright AsiaInfo 1998, all rights reserved.
   First released by Jeikul, 98.10.15, guangzhou

   this file is provided to be included by the programs
   linked with AIDataList.cpp
*/

#ifndef __AILIBEX__AIDATALIST_H__
#define __AILIBEX__AIDATALIST_H__
#include <pthread.h>
#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////
//
#define CASE_SENS_SEEK         1 
#define CASE_INSENS_SEEK       0  /*These 2 macros are defined for Strings.iSeek()*/

#define DL_THREAD_SAFE         1
#define DL_SINGLE_THREAD       0

#define DL_LAST_INDEX          -1
#define DL_DEFAULT_STEP        128 /* The size each time increase to the list */

////////////////////////////////////////////////////////////////////
//
typedef struct _stDataItem 
{ 
	const void	*pBuffer ;    /* The pointer to a saved memory block */
	int			iSize ;       /* The size of a saved memory block */
} DataItem_t ;                /* The definition of the memory chain */
 
////////////////////////////////////////////////////////////////////
//
class clsDataList 
{
  protected :
    int		ciCount;               /* How many nodes in the chain */
	int		ciStep;
	int		ciSize;                /* Byte size of the whole chain */
	int		ciThreadSafe;
	int		ciMaxMemory;		   /* Reserved */
	int		ciListSize;
        
    pthread_mutex_t ctLock;        /* The share lock of the chain */
    DataItem_t *cpList;            /* Point to the first node of the chain */

  public:
    int ciGetCount( void );
    int ciGetSize( void );
    int ciAppend( const void *asBuffer, int aiSize = 0 );
    int ciDelete( int aiIndex );
    int ciDelete( const void *asBuffer, int aiSize = 0 );
    int ciInsert( const void *asBuffer, int aiIndex,
      int aiSize = 0, int aiPlaced = 0, int aiIgnoreThread = 0 );
      
    int ciUpdate( const void *asBuffer, int aiIndex, int aiSize = 0 );
    int ciPlaced( const void *asBuffer, int aiIndex, int aiSize = 0 );
    int ciTakeItem( void *asBuffer, int aiBufferSize, int aiIndex );
    const void * csTakeItem ( int aiIndex, int *apiSize = NULL );
    const void * csGetItem ( int aiIndex, int *apiSize = NULL );
    const void * csGetMerge ( int *apiSize = NULL, int aiSeparator = '\0' );
    int ciSeek( const char *asValue, int aiCaseSens );
    int cinSeek( const char *asValue, int aiCaseSens );
    void cvClean( void );
    void operator += ( class clsDataList &apDataList );

    virtual void cvSort(int (*compar)(const void *, const void *) = NULL,
      int aiFirstIndex = 0, int aiSize = -1);
    virtual void cvClsSort(int (*apfCompare)(const void *, const void *) = NULL, \
      int aiFirstIndex = 0, int aiSize = -1);

	void cvDisorder();
    int ciSearch( const void *apvSample,
	int (*apfCompare) (const void *, const void *) = NULL, int aiSize = 0, int aiPlace = 0, void **appvMatch = NULL );
    int ciSearch( const void *apKey, int * apiInsertPoint ,
      int (*apfCompare)(const  void  *,  const  void *) = NULL, int aiFirstIndex = 0, int aiSize = -1 );
	int ciBatchDelete( const void *apKey, int (*apfCompare)(const  void  *,  const  void *) = NULL, int aiFirstIndex = 0, int aiSize = -1 );

public:
	clsDataList( int aiThreadSafe = 1, int aiStep = 0, int aiMaxMemory = 0 );
	virtual ~clsDataList();
};

////////////////////////////////////////////////////////////////////
//
class clsDataList *gpStringToList( const char *asBuffer, int aiThreadMode = DL_THREAD_SAFE,
	int aiMaxItems = 0, char const ** appcNext = NULL );

///end namespace
AIBC_NAMESPACE_END

#endif //__AILIBEX__AIDATALIST_H__

