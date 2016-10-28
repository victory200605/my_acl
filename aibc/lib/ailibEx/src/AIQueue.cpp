#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AIQueue.h"

///start namespace
AIBC_NAMESPACE_START

///////////////////////////////////////////////////////////
//
typedef struct QueueNode
{
	LISTHEAD	coChild;
	int			ciDataSize;
	char		csBuffer[1];
} QueueNode;


///////////////////////////////////////////////////////////
//
AIQueue::AIQueue( void )
{
	ciCount = 0;
	ai_init_list_head( &coHead );
}

AIQueue::~AIQueue( )
{
	AIQueue::Clear();
}

/** 
	\brief  clear queue
	\return 
*/
void AIQueue::Clear( )
{
	LISTHEAD	*lpoTemp = NULL;
	LISTHEAD	*lpoHead = &(coHead);
	AISmartLock	loTempLock( coLock );

	while( !ai_list_is_empty( lpoHead ) )
	{
		QueueNode	*lpoBlock = NULL;
	
		ai_list_del_head( lpoTemp, lpoHead );
		lpoBlock = AI_GET_STRUCT_PTR( lpoTemp, QueueNode, coChild );
		
		free( lpoBlock );
		ciCount -= 1;
	}
}

int AIQueue::GetSize( void )
{
	return  (ciCount);
}

/** 
	\brief  add one QueueNode to Queue tail
	\param apoData:the data to add 
	\param aiSize:size of apoData
	\return 
*/
int AIQueue::InQueue( const void *apoData, const int aiSize )
{
	QueueNode	*lpoBlock = NULL;

	if( (lpoBlock = (QueueNode *)malloc( aiSize + sizeof(QueueNode) )) == NULL )
		return  (0);

	lpoBlock->ciDataSize = aiSize;
	memcpy( lpoBlock->csBuffer, apoData, aiSize );
	
	coLock.Lock();
	ciCount += 1;
	ai_list_add_tail( &(lpoBlock->coChild), &coHead );
	coLock.Unlock();

	return  (aiSize);
}

/** 
	\brief  delete the first QueueNode and get  it's data to apoData
	\param apoData:the buffer to save data 
	\param aiMaxSize:size of apoData
	\return 
*/
int AIQueue::OutQueue( void *apoData, const int aiMaxSize )
{
	LISTHEAD	*lpoTemp = NULL;
	QueueNode	*lpoBlock = NULL;

	coLock.Lock();
	ai_list_del_head( lpoTemp, &coHead );
	if( lpoTemp )  ciCount -= 1;
	coLock.Unlock();

	if( lpoTemp )
	{
		int		liSize = aiMaxSize;

		lpoBlock = AI_GET_STRUCT_PTR( lpoTemp, QueueNode, coChild );
		if( liSize > lpoBlock->ciDataSize )  liSize = lpoBlock->ciDataSize;
		memcpy( apoData, lpoBlock->csBuffer, liSize );

		free( lpoBlock );
		return  (liSize);
	}

	return  (0);
}

/** 
	\brief  delete the first QueueNode and get  it's data to aoBlock
	\param aoBlock:the chunk to save data 
	\return 
*/
int AIQueue::OutQueue( AIChunk &aoBlock )
{
	LISTHEAD	*lpoTemp = NULL;
	QueueNode	*lpoBlock = NULL;

	coLock.Lock();
	ai_list_del_head( lpoTemp, &coHead );
	if( lpoTemp )  ciCount -= 1;
	coLock.Unlock();

	if( lpoTemp )
	{
		lpoBlock = AI_GET_STRUCT_PTR( lpoTemp, QueueNode, coChild );
		aoBlock.Resize( lpoBlock->ciDataSize );

		memcpy( aoBlock.GetPointer(), lpoBlock->csBuffer, lpoBlock->ciDataSize );
		free( lpoBlock );

		return  (aoBlock.GetSize());
	}

	return  (0);
}

///end namespace
AIBC_NAMESPACE_END
