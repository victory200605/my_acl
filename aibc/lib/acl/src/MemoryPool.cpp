
#include "acl/MemoryPool.h"

ACL_NAMESPACE_START

CMemoryPool::CMemoryPool(void)
    : muPoolSize(0)
    , muUsedSize(0)
    , mpcData(NULL)
    , mpcEnd(NULL)
{
    ACL_RING_INIT(&this->moHead, CBlockNode, moLink);
}

CMemoryPool::~CMemoryPool(void)
{
    this->Release();
}

apl_int_t CMemoryPool::Initialize( apl_size_t aiPoolSize )
{
    apl_size_t luPoolSize = ACL_DOALIGN(aiPoolSize, 8);
    
    if (luPoolSize > APL_INT32_MAX || luPoolSize < sizeof(CBlockNode) )
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    if (this->mpcData != NULL)
    {
        apl_set_errno(APL_EBUSY);
        return -1;
    }

    ACL_NEW_N_ASSERT( this->mpcData, unsigned char, luPoolSize );
    this->mpcEnd = this->mpcData + luPoolSize;
    this->muPoolSize = luPoolSize;
    this->muUsedSize = 0;
    
    this->Reset();
    
    return 0;
}
    
void* CMemoryPool::Allocate( apl_size_t aiSize )
{
    CBlockNode* lpoCurr = NULL;
    CBlockNode* lpoNext = NULL;
    
    if ( ACL_RING_IS_EMPTY(&this->moHead, CBlockNode, moLink) )
    {
        //No enough space
        return NULL;
    }
    
    if (aiSize < sizeof(CBlockNode) - CBlockNode::HEAD_SIZE)
    {
        //Min allocate size
        aiSize = sizeof(CBlockNode) - CBlockNode::HEAD_SIZE;
    }
    
    aiSize = ACL_DOALIGN(aiSize, 8);

    //Foreach ring element
    ACL_RING_FOREACH_SAFE(lpoCurr, lpoNext, &this->moHead, CBlockNode, moLink)
    {
        if ( (apl_size_t)lpoCurr->miLength >= aiSize + sizeof(CBlockNode) )
        {
            //Is splitable
            CBlockNode* lpoSplitNode = 
                (CBlockNode*)( (char*)lpoCurr + CBlockNode::HEAD_SIZE + aiSize );
            lpoSplitNode->miLength = lpoCurr->miLength - aiSize - CBlockNode::HEAD_SIZE;
            lpoSplitNode->miPrevLength = aiSize;
            
            //Replace element
            ACL_RING_INSERT_BEFORE(lpoCurr, lpoSplitNode, moLink);
            ACL_RING_REMOVE(lpoCurr, moLink);
            
            lpoCurr->miLength = -aiSize;
            
            this->muUsedSize += aiSize;
            
            return (char*)lpoCurr + CBlockNode::HEAD_SIZE;
        }
        else if ( (apl_size_t)lpoCurr->miLength >= aiSize )
        {
            ACL_RING_REMOVE(lpoCurr, moLink);
            
            this->muUsedSize += lpoCurr->miLength;
            
            lpoCurr->miLength = -lpoCurr->miLength;
            
            return (char*)lpoCurr + CBlockNode::HEAD_SIZE;
        }
    }

    return NULL;
}

void CMemoryPool::Deallocate( void* apvPtr )
{
    CBlockNode* lpoCurr = (CBlockNode*)((char*)apvPtr - CBlockNode::HEAD_SIZE);
    lpoCurr->miLength  = lpoCurr->miLength < 0 ? -lpoCurr->miLength : lpoCurr->miLength;
    
    //Next & Prev node
    CBlockNode* lpoNext = (CBlockNode*)((char*)apvPtr + lpoCurr->miLength);
    CBlockNode* lpoPrev = (CBlockNode*)((char*)apvPtr - lpoCurr->miPrevLength - CBlockNode::HEAD_SIZE);;

    this->muUsedSize -= lpoCurr->miLength;
    
    if ( (unsigned char*)lpoPrev >= this->mpcData 
        && lpoCurr->miPrevLength > 0
        && lpoPrev->miLength > 0 )
    {
        //Splice prev node
        lpoPrev->miLength += lpoCurr->miLength + CBlockNode::HEAD_SIZE;
        lpoCurr = lpoPrev;
        lpoPrev = NULL;
    }

    if ( (unsigned char*)lpoNext < this->mpcEnd && lpoNext->miLength > 0 )
    {
        //Splice next node
        lpoCurr->miLength += (lpoNext->miLength + CBlockNode::HEAD_SIZE);
        
        ACL_RING_REMOVE(lpoNext, moLink);
        
        lpoNext = (CBlockNode*)((char*)lpoCurr + lpoCurr->miLength + CBlockNode::HEAD_SIZE);
    }
    
    //Reset next node size
    if ( (unsigned char*)lpoNext < this->mpcEnd )
    {
        lpoNext->miPrevLength = lpoCurr->miLength;
    }
    
    if ( lpoPrev != NULL )
    {
        //Insert onto ring
        ACL_RING_INSERT_HEAD(&this->moHead, lpoCurr, CBlockNode, moLink);
    }
}

void CMemoryPool::Release()
{
    ACL_DELETE_N(this->mpcData);
    
    this->mpcEnd = NULL;
    this->muPoolSize = 0;
    this->muUsedSize = 0;
    ACL_RING_INIT(&this->moHead, CBlockNode, moLink);
}
    
void CMemoryPool::Reset()
{
    ACL_RING_INIT(&this->moHead, CBlockNode, moLink);
    
    this->muUsedSize = 0;
    
    CBlockNode* lpoNode = reinterpret_cast<CBlockNode*>(this->mpcData);
    lpoNode->miPrevLength = 0;
    lpoNode->miLength = this->muPoolSize - CBlockNode::HEAD_SIZE;
    
    //Initialize element
    ACL_RING_ELEM_INIT(lpoNode, moLink);
    
    //Tail element onto ring
    ACL_RING_INSERT_TAIL(&this->moHead, lpoNode, CBlockNode, moLink);
}
    
apl_size_t CMemoryPool::GetSize(void)
{
    return this->muUsedSize;
}
    
apl_size_t CMemoryPool::GetSpace(void)
{
    return this->muPoolSize - this->muUsedSize;
}

ACL_NAMESPACE_END
