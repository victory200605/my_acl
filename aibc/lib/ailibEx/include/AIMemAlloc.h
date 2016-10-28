/** @file AIMemAlloc.h
 */

#ifndef __AILIBEX__AIMEMALLOC_H__
#define __AILIBEX__AIMEMALLOC_H__

#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

/**
 * @brief This is class AIChunk.
 */
class AIChunk
{
public:
    AIChunk(size_t aiInitSize=0);
    AIChunk(const AIChunk&);
    ~AIChunk();

    /**
     * @brief Override =
     */ 
    AIChunk& operator= (AIChunk const& aoOther);

    /**
     * @brief Get chunk size.
     */   
    size_t GetSize() const;

    /**
     * @brief Get chunk pointer.
     */ 
    char* GetPointer();
    const char* GetPointer() const;

    /**
     * @brief Change the chunk size.
     */ 
    void Resize(size_t);

    /**
     * @brief Swap two AIChunk.
     */ 
    void Swap(AIChunk&); 

    /**
     * @brief Use input char fill the chunk.
     */ 
    void Fill(char);

private:
    size_t	ciSize;
    char*	cpcData;
};

///end namespace
AIBC_NAMESPACE_END

#endif  // __AILIBEX__AIMEMALLOC_H__

