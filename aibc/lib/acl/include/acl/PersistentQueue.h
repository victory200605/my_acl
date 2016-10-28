/**
 * @file PersistenQueue.h
 */

#ifndef ACL_PERSISTENQUEUE_H
#define ACL_PERSISTENQUEUE_H

#include "acl/Utility.h"
#include "acl/File.h"
#include "acl/Dir.h"
#include "acl/Synch.h"
#include "acl/MemMap.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"

ACL_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////////////
template<typename ValueType>
class TPersistentQueueSegment
{
    struct CHeader
    {
        CHeader(void) : muPopPos(0), muPushPos(0), muValueSize(sizeof(ValueType) ) {}

        apl_uint32_t muPopPos;
        apl_uint32_t muPushPos;
        apl_uint32_t muValueSize;
    };

public:
    TPersistentQueueSegment(void);

    ~TPersistentQueueSegment(void);

    /** 
     * @brief Init queue segment, if storage file existed and loading current file, otherwise create it
     * 
     * @param apcName segment name
     * @param auBufferSize buffer size, if buffer size non-zero, 
     *    segment will cache @auBufferSize node in memory, instead of flushing file immediately
     * @param auCapacity segment capacity
     *
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Init( char const* apcName, apl_size_t auCapacity, char* apcZero );

    /** 
     * @brief Destory segment, and release all related resource include storage file
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Destroy(void);

    /** 
     * @brief Is segment empty
     * 
     * @returns is empty will return true
     */
    bool IsEmpty(void);

    /** 
     * @brief Is segment full
     * 
     * @returns is full will return true
     */
    bool IsFull(void);

    /** 
     * @brief Is segment End
     * 
     * @returns is full will return true
     */
    bool IsEnd(void);

    /** 
     * @brief get segment size
     * 
     * @returns segment size
     */
    apl_size_t GetSize(void);

    /**
     * @brief Push @aoValue into segment and it will be persistent to file immediately or later
     * 
     * @param aoValue push value
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Push( ValueType const& aoValue );
    
    /** 
     * @brief Get all values of segment  
     * 
     * @param apcValue output values and aiTotal output counts of values
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t FrontAll( char* apcValue, apl_size_t &aiTotal );

    /** 
     * @brief Get the first value of segment  
     * 
     * @param aoValue output value
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Front( ValueType& aoValue );
    
    /** 
     * @brief Pop and Get a value from segment and will be erase the value from storage file
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Pop( ValueType& aoValue );

    /** 
     * @brief Pop a value from segment and will be erase the value from storage file
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Pop(void);

    /** 
     * @brief Flush all buffer value into storage file immediately
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    //apl_int_t Flush(void);

    /** 
     * @brief finish Flush and close file description
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    //apl_int_t FinishFlush(void);

    /** 
     * @brief check whether file already mapped to memory
     * 
     * @returns if successful and return ture, otherwise false share be return  
     */
    bool IsMap(void);

    /** 
     * @brief  file map to memory
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t DoMap(apl_handle_t aiFileHandle = -1);

    /** 
     * @brief release memory map
     * 
     * @returns NULL
     */
    void UnMap(void);

protected:
    apl_int_t Load( char const* apcName );

    apl_int_t Create( char const* apcName );

private:
    apl_size_t muPopPos;
    apl_size_t muPushPos;
    apl_size_t muCapacity;

    CHeader* mpoHeader;
    char* mpcMapAddr;
    char* mpcZero;

    std::string moFileName;
    acl::CMemMap moMemMap;
};

///////////////////////////////////////////////////////////////////////////////////
template<typename ValueType, typename LockType = acl::CNullLock>
class TPersistentQueue
{
    typedef TPersistentQueueSegment<ValueType> SegmentType;

public:
    TPersistentQueue(void);

    ~TPersistentQueue(void);

    void SetBufferSize( apl_size_t auBufferSize );

    void SetHomeDir( char const* apcPath );

    /** 
     * @brief Init queue segment, if storage file existed and loading current file, otherwise create it
     * 
     * @param apcName queue name
     * @param auSegmentSize single segment storage capacity
     * @param auBufferSize buffer size, if buffer size non-zero, 
     *    segment will cache @auBufferSize node in memory, instead of flushing file immediately
     * @param apcHomeDir segment storage file home dir
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Initialize( 
        char const* apcName, 
        apl_size_t auSegmentSize = 10000, 
        apl_size_t auBufferSize = 0, 
        char const* apcHomeDir = ".",
        apl_size_t auCapacity = APL_UINT_MAX );

    /** 
     * @brief Close queue
     */
    void Close(void);
        
    /**
     * @brief Push @aoValue into segment and it will be persistent to file immediately or later
     * 
     * @param aoValue push value
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Push( ValueType const& aoValue );

    /** 
     * @brief Get the first value of segment  
     * 
     * @param aoValue output value
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Front( ValueType& aoValue );

    /** 
     * @brief Pop and Get a value from segment and will be erase the value from storage file
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Pop( ValueType& aoValue );
    
    /** 
     * @brief Pop a value from segment and will be erase the value from storage file
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Pop(void);

    /** 
     * @brief Flush all buffer value into storage file immediately
     * 
     * @returns if successful and return 0, otherwise -1 share be return  
     */
    apl_int_t Flush(void);

    /** 
     * @brief get segment size
     */
    apl_size_t GetSize(void) const;

    /** 
     * @brief get queue name
     */
    char const* GetName(void) const;

    /** 
     * @brief get queue home dir path
     */
    char const* GetHomeDir(void) const;

protected:
    bool CacheIsFull();

    bool CacheIsEmpty();
    
    apl_int_t CachePush( ValueType const& aoValue );
    
    apl_int_t SegmentPush( ValueType const& aoValue );

    apl_int_t CacheFront( ValueType& aoValue );

    apl_int_t CachePop( ValueType& aoValue );
    
    apl_int_t CachePop();

    apl_int_t SegmentPop();
    
    apl_int_t AllocateSegmentNode( char const* apcName );

private:
    enum { PERSISTENT_QUEUE_MIN_SEG = 10000, PERSISTENT_QUEUE_MAX_SEG = 1000000 };//segment range
    
    std::string moName;
    
    apl_size_t muBufferSize;
   
    std::string moHomeDir;

    apl_size_t muSegmentSize;
    
    apl_size_t muSize;

    apl_size_t muCapacity;

    apl_int_t miMinIndex;

    apl_int_t miMaxIndex;
    
    apl_size_t muPopPos;
    
    apl_size_t muPushPos;

    apl_size_t muCurrSegmentSize;
    
    apl_size_t muMaxCache;
    
    bool mbCircled;
    
    bool mbShutdown;
    
    char* mpcCache;
    
    char* mpcZero;

    LockType moLock;
    std::vector<SegmentType*> moMap;
};

///////////////////////////////////////////////////////////////////////////////////////////////
//Implement
template<typename ValueType>
TPersistentQueueSegment<ValueType>::TPersistentQueueSegment(void)
    : muPopPos(0)
    , muPushPos(0)
    , muCapacity(0)
    , mpoHeader(APL_NULL)
    , mpcMapAddr(APL_NULL)
{
}

template<typename ValueType>
TPersistentQueueSegment<ValueType>::~TPersistentQueueSegment(void)
{
    if ( this->IsMap() )
    {
        this->UnMap();
    }
}

template<typename ValueType>
apl_int_t TPersistentQueueSegment<ValueType>::Init( char const* apcName, apl_size_t auCapacity, char* apcZero )
{
    this->moFileName = apcName;
    this->muPopPos = 0;
    this->muPushPos = 0;
    this->muCapacity = auCapacity;
    this->mpoHeader = APL_NULL;
    this->mpcMapAddr = APL_NULL;
    this->mpcZero = apcZero;

    if (acl::CFile::Access(apcName, APL_F_OK) != 0)
    {
        return this->Create(apcName);
    }
    else
    {
        return this->Load(apcName);
    }
}

template<typename ValueType>
apl_int_t TPersistentQueueSegment<ValueType>::Destroy(void)
{
    if (this->moFileName.length() > 0)
    {
        if( this->IsMap() )
        {
            this->UnMap();
        }

        return acl::CFile::Remove(this->moFileName.c_str() );
    }

    return 0;
}

template<typename ValueType>
bool TPersistentQueueSegment<ValueType>::IsEmpty(void)
{
    return (this->muPopPos < this->muCapacity && this->muPopPos >= this->muPushPos);
}

template<typename ValueType>
bool TPersistentQueueSegment<ValueType>::IsFull(void)
{
    return this->muPushPos >= this->muCapacity;
}

template<typename ValueType>
bool TPersistentQueueSegment<ValueType>::IsEnd(void)
{
    return this->muPopPos >= this->muCapacity;
}

template<typename ValueType>
apl_size_t TPersistentQueueSegment<ValueType>::GetSize(void)
{
    return this->muPushPos - this->muPopPos;
}

template<typename ValueType>
apl_int_t TPersistentQueueSegment<ValueType>::Push( ValueType const& aoValue )
{
    if ( !this->IsMap() )
    {
        if( this->DoMap() != 0 )
        {
            return -1;
        }
    }

    apl_uint32_t luOff = this->muPushPos*(sizeof(ValueType));

    //push
    apl_memcpy(this->mpcMapAddr+luOff, &aoValue, sizeof(ValueType));
    this->muPushPos++;
    this->mpoHeader->muPushPos = this->muPushPos;
    
    return 0;
}

template<typename ValueType>
apl_int_t TPersistentQueueSegment<ValueType>::FrontAll( char* apcValue, apl_size_t &aiTotal )
{
    if( apcValue == APL_NULL )
    {
        return -1;
    }
    
    if ( !this->IsMap() )
    {
        if( this->DoMap() != 0 )
        {
            return -1;
        }
    }

    if( !( this->IsEnd() || this->IsEmpty() ) )
    {
        if( this->muPushPos <= this->muPopPos )
        {
            return -1;
        }
        
        apl_uint32_t luOff = this->muPopPos*(sizeof(ValueType));
        apl_uint32_t luTotal = this->muPushPos - this->muPopPos;
        
        apl_memcpy(apcValue, this->mpcMapAddr+luOff, luTotal*sizeof(ValueType));
        this->muPopPos = this->muPushPos;
        this->mpoHeader->muPopPos = this->muPushPos;
        
        aiTotal = luTotal;
        
        return 0;
    }

    return -1;
}

template<typename ValueType>
apl_int_t TPersistentQueueSegment<ValueType>::Front( ValueType& aoValue )
{
    if ( !this->IsMap() )
    {
        if( this->DoMap() != 0 )
        {
            return -1;
        }
    }
    
    if( !( this->IsEnd() || this->IsEmpty() ) )
    {
        apl_uint32_t luOff = this->muPopPos*sizeof(ValueType);
        
        //pop
        apl_memcpy(&aoValue, this->mpcMapAddr+luOff, sizeof(ValueType));
        
        return 0;
    }

    return -1;
}

template<typename ValueType>
apl_int_t TPersistentQueueSegment<ValueType>::Pop( ValueType& aoValue )
{
    if ( !this->IsMap() )
    {
        if( this->DoMap() != 0 )
        {
            return -1;
        }
    }
    
    if( !( this->IsEnd() || this->IsEmpty() ) )
    {
        apl_uint32_t luOff = this->muPopPos*sizeof(ValueType);
        
        //pop
        apl_memcpy(&aoValue, this->mpcMapAddr+luOff, sizeof(ValueType));
        
        this->muPopPos++;
        this->mpoHeader->muPopPos = this->muPopPos;
        
        return 0;
    }

    return -1;
}

template<typename ValueType>
apl_int_t TPersistentQueueSegment<ValueType>::Pop(void)
{
    if ( !this->IsMap() )
    {
        if( this->DoMap() != 0 )
        {
            return -1;
        }
    }
    
    while( !( this->IsEnd() || this->IsEmpty() ) )
    {
        apl_uint32_t luOff = this->muPopPos*sizeof(ValueType);
        
        //pop
        this->muPopPos++;
        this->mpoHeader->muPopPos = this->muPopPos;
        
        return 0;
    }

    return -1;
}

template<typename ValueType>
bool TPersistentQueueSegment<ValueType>::IsMap(void)
{
    return (this->mpoHeader != APL_NULL && this->mpcMapAddr != APL_NULL);
}

template<typename ValueType>
apl_int_t TPersistentQueueSegment<ValueType>::DoMap(apl_handle_t aiFileHandle)
{   
    acl::CFile   loFile;
    apl_handle_t liFd = aiFileHandle;
    apl_uint32_t luMapSize = 0;

    if( this->IsMap() )
    {
        return 0;
    }
    
    if( liFd < 0 )
    {
        if (loFile.Open(this->moFileName.c_str(), APL_O_CREAT|APL_O_RDWR, 0755) != 0)
        {
            apl_errprintf("[%s] PersistentQueue open file(%s) fail, %s\n", __FUNCTION__, this->moFileName.c_str(), apl_strerror(apl_get_errno()) );
            return -1;
        }
        liFd = loFile.GetHandle();
    }

    luMapSize = sizeof(CHeader) + this->muCapacity*sizeof(ValueType);
    if (this->moMemMap.Map(liFd, APL_PROT_WRITE|APL_PROT_READ, APL_MAP_SHARED, luMapSize ) != 0)
    {
        apl_errprintf("[%s] PersistentQueue MemMap file(%s) fail, %s\n", __FUNCTION__, this->moFileName.c_str(), apl_strerror(apl_get_errno()) );
        return -1;
    }

    //apl_errprintf("[%s] PersistentQueue MemMap file(%s) OK!\n", __FUNCTION__, this->moFileName.c_str());

    this->mpoHeader = (CHeader*)this->moMemMap.GetAddr();
    this->mpcMapAddr = (char*)this->moMemMap.GetAddr()+sizeof(CHeader);

    this->muPushPos = this->mpoHeader->muPushPos;
    this->muPopPos = this->mpoHeader->muPopPos;
    
    return 0;
}

template<typename ValueType>
void TPersistentQueueSegment<ValueType>::UnMap(void)
{
    apl_int_t liRet = 0;
    if( this->IsMap() )
    {
        liRet = this->moMemMap.Unmap();
        if( liRet != 0 )
        {
            apl_errprintf("[%s] PersistentQueue MemUnmap file(%s) ret = %d(errno = %d)\n", __FUNCTION__, this->moFileName.c_str(), liRet, apl_get_errno());
        }
    }
    this->mpoHeader = APL_NULL;
    this->mpcMapAddr = APL_NULL;
}

template<typename ValueType>
apl_int_t TPersistentQueueSegment<ValueType>::Load( char const* apcName )
{
    acl::CFile loFile;
    CHeader loHeader;
    apl_ssize_t liFileSize = 0;
    apl_uint32_t luMapSize = 0;

    //Open segment storage file
    if (loFile.Open(apcName, APL_O_CREAT | APL_O_RDWR, 0755) != 0)
    {
        apl_errprintf("[%s] PersistentQueue open file(%s) fail\n", __FUNCTION__, apcName);
        return -1;
    }

    luMapSize = sizeof(CHeader) + this->muCapacity*sizeof(ValueType);
    if ( (liFileSize = loFile.GetFileSize() ) != (apl_ssize_t)luMapSize )
    {
        apl_errprintf("[%s] PersistentQueue invalid segment storage file(%s)," 
            "(FileSize=%"APL_PRIdINT",HeaderSize=%"APL_PRIdINT",ValueSize=%"APL_PRIdINT",Capacity=%"APL_PRIdINT")\n", 
            __FUNCTION__,
            apcName,
            (apl_int_t)liFileSize,
            (apl_int_t)sizeof(CHeader),
            (apl_int_t)sizeof(ValueType),
            (apl_int_t)this->muCapacity );
        
        return -1;
    }
    
    if (loFile.Read(&loHeader, sizeof(CHeader) ) != (apl_ssize_t)sizeof(CHeader) )
    {
        apl_errprintf("[%s] PersistentQueue read file(%s) header fail, %s\n", __FUNCTION__, apcName, apl_strerror(apl_get_errno()) ); 
        return -1;
    }

    if (loHeader.muValueSize != sizeof(ValueType) )
    {
        apl_errprintf("[%s] PersistentQueue invalid segment storage file(%s), (Header::ValueSize=%"APL_PRIdINT",ValueSize=%"APL_PRIdINT")\n", 
            __FUNCTION__,
            apcName,
            (apl_int_t)loHeader.muValueSize,
            (apl_int_t)sizeof(ValueType) );
        
        return -1;
    }
    
    if (loHeader.muPopPos > loHeader.muPushPos )
    {
        apl_errprintf("[%s] PersistentQueue invalid segment storage file(%s), PopPos(%"APL_PRIdINT") > PushPos(%"APL_PRIdINT")\n", 
            __FUNCTION__, apcName, loHeader.muPushPos, loHeader.muPopPos );
        
        return -1;
    }

    this->muPushPos = loHeader.muPushPos;
    this->muPopPos = loHeader.muPopPos;
            
    return 0;
}

template<typename ValueType>
apl_int_t TPersistentQueueSegment<ValueType>::Create( char const* apcName )
{
    CHeader loHeader;
    acl::CFile loFile;
        
    //Open segment storage file
    if (loFile.Open(apcName, APL_O_CREAT|APL_O_RDWR, 0755) != 0)
    {
        apl_errprintf("[%s] PersistentQueue open file(%s) fail, %s\n", __FUNCTION__, apcName, apl_strerror(apl_get_errno()) );
        return -1;
    }

    //Init segment header
    if (loFile.Write(&loHeader, sizeof(loHeader) ) != (apl_ssize_t)sizeof(loHeader) )
    {
        apl_errprintf("[%s] PersistentQueue write segment file(%s) header fail, %s\n", __FUNCTION__, apcName, apl_strerror(apl_get_errno()) );
        return -1;
    }

    //Init segment data
    if (loFile.Write(this->mpcZero, this->muCapacity*sizeof(ValueType) ) != (apl_ssize_t)(this->muCapacity*sizeof(ValueType)) )
    {
        apl_errprintf("[%s] PersistentQueue write segment file(%s) data fail, %s\n", __FUNCTION__, apcName, apl_strerror(apl_get_errno()) );
        
        return -1;
    }
    
    if( this->DoMap(loFile.GetHandle()) != 0)
    {
        apl_errprintf("[%s] PersistentQueue MemMap file(%s) fail, %s\n", __FUNCTION__, apcName, apl_strerror(apl_get_errno()));
        return -1;
    }
                    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
template<typename ValueType, typename LockType>
TPersistentQueue<ValueType, LockType>::TPersistentQueue(void)
    : muBufferSize(0)
    , muSegmentSize(0)
    , muSize(0)
    , muCapacity(0)
    , muPopPos(0)
    , muPushPos(0)
    , muCurrSegmentSize(0)
    , muMaxCache(0)
    , mbCircled(false)
    , mbShutdown(true)
    , mpcCache(APL_NULL)
    , mpcZero(APL_NULL)
{
}

template<typename ValueType, typename LockType>
TPersistentQueue<ValueType, LockType>::~TPersistentQueue(void)
{
    this->Close();
}

template<typename ValueType, typename LockType>
void TPersistentQueue<ValueType, LockType>::SetBufferSize( apl_size_t auBufferSize )
{
    this->muBufferSize = auBufferSize;
}

template<typename ValueType, typename LockType>
void TPersistentQueue<ValueType, LockType>::SetHomeDir( char const* apcPath )
{
    this->moHomeDir = apcPath;
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::Initialize( 
    char const* apcName, 
    apl_size_t auSegmentSize, 
    apl_size_t auBufferSize, 
    char const* apcHomeDir,
    apl_size_t auCapacity )
{
    char lacTemp[APL_PATH_MAX];

    acl::TSmartLock<LockType> loGuard(this->moLock);
    
    this->moName = apcName;
    this->muSegmentSize = auSegmentSize;
    this->muBufferSize = auBufferSize;
    this->moHomeDir = apcHomeDir;
    this->muCapacity = auCapacity;
    this->miMinIndex = APL_INT32_MAX;
    this->miMaxIndex = 0;
    this->muPopPos = 0;
    this->muPushPos = 0;
    this->muCurrSegmentSize = 0;
    this->mbCircled = false;
    this->mbShutdown = true;
    
    if( this->muSegmentSize < PERSISTENT_QUEUE_MIN_SEG )
    {
        this->muSegmentSize = PERSISTENT_QUEUE_MIN_SEG;
    }

    if( this->muSegmentSize > PERSISTENT_QUEUE_MAX_SEG )
    {
        this->muSegmentSize = PERSISTENT_QUEUE_MAX_SEG;
    }
    this->muMaxCache = this->muSegmentSize;//MaxCache define one Completed SegmentSize, purpose to memcopy all values of Segment to Cache when  Cache is Empty

    ACL_NEW_N_ASSERT(this->mpcCache, char, this->muMaxCache*sizeof(ValueType));

    ACL_NEW_N_ASSERT(this->mpcZero, char, this->muMaxCache*sizeof(ValueType));
    apl_memset(this->mpcZero, 0, this->muMaxCache*sizeof(ValueType));

    apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s", this->moHomeDir.c_str(), this->moName.c_str() );
    acl::CDir::Make(lacTemp);
    acl::CDir loDir;

    if (loDir.Open(lacTemp) != 0)
    {
        return -1;
    }

    // Search segment storage file
    while(loDir.Read() == 0)
    {
        if (loDir.IsFile() )
        {
            apl_int_t liIndex = apl_strtoi32(loDir.GetName(), NULL, 10);
            if (liIndex < this->miMinIndex)
            {
                this->miMinIndex = liIndex;
            }

            if (liIndex > this->miMaxIndex)
            {
                this->miMaxIndex = liIndex;
            }
        }
    }

    // Load sgement storage file
    acl::CFileInfo loFileInfo;
    for (apl_int_t liN = this->miMinIndex; liN <= this->miMaxIndex; liN++)
    {
        apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s/%"APL_PRIdINT, 
            this->moHomeDir.c_str(), this->moName.c_str(), liN);

        if (acl::CFile::GetFileInfo(lacTemp, loFileInfo) != 0)
        {
            apl_errprintf("PersistentQueue get file info fail,%s\n", apl_strerror(apl_get_errno() ) );

            return -1;
        }

        if (loFileInfo.GetSize() == 0)
        {
            //Skip empty file and remove it
            acl::CFile::Remove(lacTemp);

            continue;
        }
        
        if (this->AllocateSegmentNode(lacTemp) != 0)
        {
            return -1;
        }
    }
    
    this->mbShutdown = false;
    return 0;
}

template<typename ValueType, typename LockType>
void TPersistentQueue<ValueType, LockType>::Close(void)
{
    acl::TSmartLock<LockType> loGuard(this->moLock);
    
    this->mbShutdown = true;

    ValueType loValue;

    //cache to segment when close
    while(true)
    {
        if( !this->CacheIsEmpty() )
        {
            if( this->CachePop(loValue) == 0 )
            {
                this->SegmentPush(loValue);
                continue;
            }
        }

        break;
    }

    for( typename std::vector<SegmentType*>::iterator loIter = this->moMap.begin();
         loIter != this->moMap.end(); ++loIter )
    {
        SegmentType* lpoTemp = *loIter;
        ACL_DELETE(lpoTemp);
    }

    this->moMap.clear();

    if( mpcCache != APL_NULL )
    {
        ACL_DELETE_N(mpcCache);
    }
    
    if( mpcZero != APL_NULL )
    {
        ACL_DELETE_N(mpcZero);
    }
}

template<typename ValueType, typename LockType>
bool TPersistentQueue<ValueType, LockType>::CacheIsFull(void)
{
    if( (!this->mbCircled && this->muPopPos == 0 && this->muPushPos == this->muMaxCache) ||
        (this->mbCircled && this->muPopPos == this->muPushPos) )
    {
        return true;
    }

    return false;
}

template<typename ValueType, typename LockType>
bool TPersistentQueue<ValueType, LockType>::CacheIsEmpty(void)
{
    if( !this->mbCircled && this->muPopPos == this->muPushPos )
    {
        return true;
    }

    return false;
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::CachePush( ValueType const& aoValue )
{
    if(this->muPushPos >= this->muMaxCache && this->muPopPos > 0)
    {
        this->muPushPos = 0;
        this->mbCircled = true;
    }

    if( muPushPos < muMaxCache )
    {  
        apl_uint32_t luOff = this->muPushPos*sizeof(ValueType);
                
        apl_memcpy(this->mpcCache+luOff, &aoValue, sizeof(ValueType));
                
        this->muPushPos++;
        
        if(this->muPushPos >= this->muMaxCache && this->muPopPos > 0)
        {
            this->muPushPos = 0;
            this->mbCircled = true;
        }

        return 0;
    }
    
    return -1;
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::SegmentPush( ValueType const& aoValue )
{
    if (this->moMap.size() == 0 || this->moMap.back()->IsFull() )
    {
        char lacTemp[APL_PATH_MAX];
        apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s/%"APL_PRIdINT, 
            this->moHomeDir.c_str(), this->moName.c_str(), ++this->miMaxIndex);
        
        if (this->moMap.size() > 0)
        {
            if (this->moMap.back()->IsMap() )
            {
                this->moMap.back()->UnMap();
            }
        }
        
        if (this->AllocateSegmentNode(lacTemp) != 0)
        {
            return -1;
        }
    }

    return this->moMap.back()->Push(aoValue);
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::Push( ValueType const& aoValue )
{
    acl::TSmartLock<LockType> loGuard(this->moLock);

    if( !this->mbShutdown )
    {
        if (this->muSize >= this->muCapacity)
        {
            apl_errprintf("[%s] PersistentQueue Full, Capacity =%"APL_PRIdINT"\n", __FUNCTION__, this->muCapacity);     
            return -1;
        }
        
        if( this->muCurrSegmentSize <= 0 )
        {
            //cache
            if( !this->CacheIsFull() )
            {
                if(this->CachePush(aoValue) == 0)
                {
                    this->muSize++;
                    return 0;
                }
        
                apl_errprintf("[%s] PersistentQueue CachePush fail, CurrentSize = %"APL_PRIdINT", Capacity =%"APL_PRIdINT", PushPos = %"APL_PRIdINT", PopPos = %"APL_PRIdINT", MaxCahce = %"APL_PRIdINT"\n", 
                    __FUNCTION__, this->muSize, this->muCapacity, this->muPushPos, this->muPopPos, this->muMaxCache );
            
                return -1;
            }
        }
        
        //segment
        if(this->SegmentPush(aoValue) == 0)
        {
            this->muCurrSegmentSize++;
            this->muSize++;
            return 0;
        }
        
        apl_errprintf("[%s] PersistentQueue SegmentPush fail, CurrentSize = %"APL_PRIdINT", Capacity =%"APL_PRIdINT"\n", __FUNCTION__, this->muSize, this->muCapacity );
    }

    return -1;
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::CacheFront( ValueType& aoValue )
{   
    if( this->muPopPos < this->muMaxCache )
    {
        if( !this->CacheIsEmpty() )
        {
            apl_uint32_t luOff = this->muPopPos*sizeof(ValueType);
            
            apl_memcpy(&aoValue, this->mpcCache+luOff, sizeof(ValueType));
            
            return 0;
        }
    }
    
    return -1;
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::CachePop( ValueType& aoValue )
{   
    if( this->muPopPos < this->muMaxCache )
    {
        if( !this->CacheIsEmpty() )
        {
            apl_uint32_t luOff = this->muPopPos*sizeof(ValueType);
            
            apl_memcpy(&aoValue, this->mpcCache+luOff, sizeof(ValueType));
            
            this->muPopPos++;
            
            if( this->mbCircled && this->muPopPos == this->muMaxCache )
            {
                this->muPopPos = 0;
                this->mbCircled = false;
            }
    
            if( this->CacheIsEmpty() )
            {
                this->muPopPos = 0;
                this->muPushPos = 0;
            }
            
            return 0;
        }
    }
    
    return -1;
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::CachePop()
{   
    if( this->muPopPos < this->muMaxCache )
    {
        if( !this->CacheIsEmpty() )
        {           
            this->muPopPos++;
            
            if( this->mbCircled && this->muPopPos == this->muMaxCache )
            {
                this->muPopPos = 0;
                this->mbCircled = false;
            }
            
            if( this->CacheIsEmpty() )
            {
                this->muPopPos = 0;
                this->muPushPos = 0;
            }
            
            return 0;
        }
    }
    
    return -1;
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::SegmentPop()
{   
    apl_size_t liCurrSegmentSize = 0;
    apl_size_t liTotal = 0;
    
    while(true)
    {
        if (this->moMap.size() == 0)
        {
            return -1;
        }

        SegmentType* lpoFront = this->moMap.front();
        if(lpoFront->FrontAll(this->mpcCache, liTotal) != 0)
        {           
            if (lpoFront->IsEnd() || (lpoFront->IsEmpty() && this->moMap.size() > 1) )
            {
                //destroy segment
                lpoFront->Destroy();
                
                this->moMap.erase(this->moMap.begin() );
                
                ACL_DELETE(lpoFront);
                continue;
            }

            return -1;
        }

        this->mbCircled = false;
        this->muPushPos = liTotal;
        this->muPopPos = 0;

        //delete if End
        if (lpoFront->IsEnd() || (lpoFront->IsEmpty() && this->moMap.size() > 1) )
        {
            //destroy segment
            lpoFront->Destroy();
            
            this->moMap.erase(this->moMap.begin() );
            
            ACL_DELETE(lpoFront);
        }

        liCurrSegmentSize = this->muCurrSegmentSize - (this->muPushPos - this->muPopPos);
        this->muCurrSegmentSize = liCurrSegmentSize >= 0 ? liCurrSegmentSize : 0;
        
        return 0;
    }

    return -1;
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::Front( ValueType& aoValue )
{
    acl::TSmartLock<LockType> loGuard(this->moLock);

    if( !this->mbShutdown )
    {
        if( this->muCurrSegmentSize > 0 )
        {
            if( this->CacheIsEmpty() )
            {
                //segment move
                if( this->SegmentPop() == 0 )
                {
                    if ( this->CacheFront(aoValue) == 0)
                    {
                        this->muSize--;
                        return 0;
                    }
                }
            
                return -1;
            }
        }
        
        //cache
        if( this->CacheFront(aoValue) == 0 )
        {
            this->muSize--;
            return 0;
        }
    }
    
    return -1;
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::Pop( ValueType& aoValue )
{
    acl::TSmartLock<LockType> loGuard(this->moLock);

    if( !this->mbShutdown )
    {
        if( this->muCurrSegmentSize > 0 )
        {
            if( this->CacheIsEmpty() )
            {
                //segment move
                if( this->SegmentPop() == 0 )
                {
                    if ( this->CachePop(aoValue) == 0)
                    {
                        this->muSize--;
                        return 0;
                    }
                }
            
                return -1;
            }
        }
        
        //cache
        if( this->CachePop(aoValue) == 0 )
        {
            this->muSize--;
            return 0;
        }
    }
    
    return -1;
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::Pop(void)
{
    acl::TSmartLock<LockType> loGuard(this->moLock);

    if( !this->mbShutdown )
    {
        if( this->muCurrSegmentSize > 0 )
        {
            if( this->CacheIsEmpty() )
            {
                //segment move
                if( this->SegmentPop() == 0 )
                {
                    if ( this->CachePop() == 0)
                    {
                        this->muSize--;
                        return 0;
                    }
                }
            
                return -1;
            }
        }
        
        //cache
        if( this->CachePop() == 0 )
        {
            this->muSize--;
            return 0;
        }
    }
    
    return -1;
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::Flush(void)
{    
    return 0;
}

template<typename ValueType, typename LockType>
apl_size_t TPersistentQueue<ValueType, LockType>::GetSize(void) const
{
    return this->muSize;
}

template<typename ValueType, typename LockType>
char const* TPersistentQueue<ValueType, LockType>::GetName(void) const
{
    return this->moName.c_str();
}

template<typename ValueType, typename LockType>
char const* TPersistentQueue<ValueType, LockType>::GetHomeDir(void) const
{
    return this->moHomeDir.c_str();
}

template<typename ValueType, typename LockType>
apl_int_t TPersistentQueue<ValueType, LockType>::AllocateSegmentNode( char const* apcName )
{
    // Create segment node
    SegmentType* lpoSegment = NULL;
    ACL_NEW_ASSERT(lpoSegment, SegmentType);

    if (lpoSegment->Init(apcName, this->muSegmentSize, this->mpcZero) != 0)
    {
        ACL_DELETE(lpoSegment);

        return -1;
    }

    this->moMap.push_back(lpoSegment);

    this->muSize += lpoSegment->GetSize();
    this->muCurrSegmentSize += lpoSegment->GetSize();

    return 0;
}

ACL_NAMESPACE_END

#endif

