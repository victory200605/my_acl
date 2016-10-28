
#include "acl/MemMap.h"
#include "acl/File.h"

ACL_NAMESPACE_START

CMemMap::CMemMap(void)
    : mpvAddr(NULL)
    , muSize(0)
{
}
    
CMemMap::~CMemMap(void)
{
    this->Unmap();
}

apl_int_t CMemMap::Map(
    char const* apcFileName,
    apl_int_t aiProt,
    apl_int_t aiFlag,
    apl_size_t auLen,
    apl_int64_t ai64Off )
{
    CFile loFile;

    if (loFile.Open(apcFileName, O_RDWR) != 0)
    {
        return -1;
    }
    
    return this->Map(loFile.GetHandle(), aiProt, aiFlag, auLen, ai64Off);
}

apl_int_t CMemMap::ShmMap(
    char const* apcFileName,
    apl_int_t aiProt,
    apl_int_t aiFlag,
    apl_size_t auLen,
    apl_int64_t ai64Off )
{
    CFile loFile;

    if (loFile.ShmOpen(apcFileName, O_RDWR) != 0)
    {
        return -1;
    }
    
    return this->Map(loFile.GetHandle(), aiProt, aiFlag, auLen, ai64Off);
}

apl_int_t CMemMap::Map(
    apl_handle_t aiFileHandle,
    apl_int_t aiProt,
    apl_int_t aiFlag,
    apl_size_t auLen,
    apl_int64_t ai64Off )
{
    CFile loFile;
    CFile::FileInfoType loFileInfo;
    
    if (this->IsMaped() )
    {
        //Maped
        apl_set_errno(APL_EBUSY);
        return -1;
    }
    
    loFile.SetHandle(aiFileHandle);
    
    do
    {
        if ( loFile.GetFileInfo(&loFileInfo) != 0 )
        {
            break;
        }
        
        if ( ai64Off < 0
            || auLen + ai64Off > loFileInfo.GetSize() )
        {
            apl_set_errno(APL_EINVAL);
            break;
        }
    
        if (auLen == 0)
        {        
            auLen = loFileInfo.GetSize() - ai64Off;
        }
        
        // Map file here
        this->mpvAddr = apl_mmap( NULL, auLen, aiProt, aiFlag, aiFileHandle, ai64Off );
        if ( this->mpvAddr == (void*)-1 )
        {
            this->mpvAddr = NULL;
            break;
        }
        
        this->muSize = auLen;
        
        loFile.SetHandle(ACL_INVALID_HANDLE);
        
        return 0;
        
    }while(false);
    
    loFile.SetHandle(ACL_INVALID_HANDLE);
    
    return -1;
}

apl_int_t CMemMap::Sync( apl_int_t aiFlag )
{
    return apl_msync(this->mpvAddr, this->muSize, aiFlag);
}

apl_int_t CMemMap::Unmap( void )
{
    apl_int_t liRetCode = 0;
    
    if (this->IsMaped() )
    {
        liRetCode = apl_munmap(this->mpvAddr, this->muSize);
        
        this->mpvAddr = NULL;
        this->muSize = 0;
    }
    
    return liRetCode;
}

apl_int_t CMemMap::MLock(void)
{
    return apl_mlock(this->mpvAddr, this->muSize);
}

apl_int_t CMemMap::MUnlock(void)
{
    return apl_munlock(this->mpvAddr, this->muSize);
}

bool CMemMap::IsMaped() const
{
    return this->mpvAddr != NULL;
}

void const* CMemMap::GetAddr() const
{
    return this->mpvAddr;
}

void* CMemMap::GetAddr()
{
    return this->mpvAddr;
}

apl_size_t CMemMap::GetSize() const
{
    return this->muSize;
}

ACL_NAMESPACE_END
