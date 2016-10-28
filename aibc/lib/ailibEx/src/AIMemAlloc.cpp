#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AIMemAlloc.h"

///start namespace
AIBC_NAMESPACE_START

AIChunk::AIChunk(size_t aiInitSize)
    :ciSize(aiInitSize)
    ,cpcData(0)
{
    AI_NEW_N_ASSERT(cpcData, char, ciSize);
}

AIChunk::AIChunk(const AIChunk& aoOther)
    :ciSize(aoOther.ciSize)
    ,cpcData(0)
{
    AI_NEW_N_ASSERT(cpcData, char, aoOther.ciSize); 
    memcpy(cpcData, aoOther.cpcData, aoOther.ciSize);
}

AIChunk::~AIChunk()
{
    AI_DELETE_N(cpcData);
}

AIChunk& AIChunk::operator= (AIChunk const& aoOther)
{
    if(this != &aoOther)
    {
        AIChunk	loTemp(aoOther);
        Swap(loTemp);
    }
    
    return  (*this);
}

size_t AIChunk::GetSize() const
{
    return  (ciSize);
}

void AIChunk::Resize(size_t aiNewSize)
{
    AIChunk		loTemp(aiNewSize);
    loTemp.Swap(*this);
}

char* AIChunk::GetPointer()
{
    //assert(cpcData);
    return  (cpcData);
}

char const* AIChunk::GetPointer() const
{
    //assert(cpcData);
    return  (cpcData);
}

void AIChunk::Swap(AIChunk& aoOther) 
{
    AI_SWAP(ciSize, aoOther.ciSize, size_t);
    AI_SWAP(cpcData, aoOther.cpcData, char*);
}

void AIChunk::Fill(char acChar)
{
    memset(cpcData, acChar, ciSize);
}

///end namespace
AIBC_NAMESPACE_END
