/** 
 * @file AIQueue.h
 */

#ifndef __AILIBEX__AIQUEUE_H__
#define __AILIBEX__AIQUEUE_H__

#include "AILib.h"
#include "AIMemAlloc.h"
#include "AIDualLink.h"
#include "AISynch.h"

///start namespace
AIBC_NAMESPACE_START

/**
 * @brief This is class AIQueue.
 */     
class AIQueue
{
private:
	int		ciCount;
	LISTHEAD	coHead;
	AIMutexLock	coLock;

public:
	AIQueue( void );    ///<initialize a queue
	~AIQueue( );        ///<destroy a queue 

public:
        /**
         * @brief Clear queue.
         */   
	void Clear( );

        /**
         * @brief Get the size of the queue.
         * 
         * @return Return the size of the queue.
         */
	int GetSize( void );

        /**
         * @brief add one QueueNode to queue tail
         *
         * @param apoData    IN - the node to add
         * @param aiSize     IN - size of apoData
         *
         * @return Return the size of apoData.
         */
	int InQueue( const void *apoData, const int aiSize );

        /**
         * @brief Delete the first QueueNode and get it's data to apoDate.
         *
         * @param apoData    OUT - the buffer to save data
         * @param aiMaxSize  IN  - size of apoData
         *
         * @return Return the size of apoData.
         */
	int OutQueue( void *apoData, const int aiMaxSize );

        /**
         * @brief Delete the first QueueNode and get it's data to aoBlock.
         *
         * @param aoBlock    IN/OUT - the chunk to save data
         *
         * @return Return the size of aoBlock.
         */
	int OutQueue( AIChunk &aoBlock );
};

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AIQUEUE_H__

