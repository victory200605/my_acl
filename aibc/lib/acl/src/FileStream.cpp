
#include "acl/FileStream.h"
#include "acl/MemoryStream.h"

ACL_NAMESPACE_START

CFileStream::CFileStream(void)
    : moBuffer(4096)
{
}

CFileStream::CFileStream( apl_size_t auBufferSize )
    : moBuffer(auBufferSize)
{
}

CFileStream::~CFileStream(void)
{ 
}

void CFileStream::SetBufferSize( apl_size_t auBufferSize )
{
    this->moBuffer.Resize(auBufferSize);
}

void CFileStream::ClearBuffer(void)
{
    this->moBuffer.Reset();
}

apl_ssize_t CFileStream::ReadLine( char* apcBuffer, apl_size_t auSize )
{
    apl_ssize_t liResult   = 0;
	apl_ssize_t liBufferSize = auSize;
	char*       lpcBuffer  = apcBuffer;
	acl::CMemoryStream loStream(&this->moBuffer);

	while(liBufferSize > 0)
	{
	    apl_ssize_t liLength = this->moBuffer.GetLength();
	    if (liLength > 0)
	    {
    	    liResult = loStream.ReadLine(lpcBuffer, liBufferSize);
            if (liResult >= 0)
            {
                //Completed
                liBufferSize -= liResult;
                break;
            }

            if (liLength > liBufferSize - 1)
            {
                //Truncation
                apl_memcpy(lpcBuffer, this->moBuffer.GetReadPtr(), liBufferSize - 1 );
                this->moBuffer.SetReadPtr(liBufferSize - 1);
                lpcBuffer[liBufferSize - 1] = '\0';
                liBufferSize = 1;
                break;
            }
            else if (*(this->moBuffer.GetReadPtr() + liLength - 1) == '\r')
            {
                //Alignment line end character
                apl_memcpy(lpcBuffer, this->moBuffer.GetReadPtr(), liLength - 1);
                lpcBuffer += (liLength - 1);
                liBufferSize -= (liLength - 1);
                this->moBuffer.Reset();
                this->moBuffer.Write("\r", 1);
            }
            else
            {
                //Uncomplete line
                apl_memcpy(lpcBuffer, this->moBuffer.GetReadPtr(), liLength);
                lpcBuffer += liLength;
                liBufferSize -= liLength;
                this->moBuffer.SetReadPtr(liLength);
                this->moBuffer.Reset();
            }
        }
        else
        {
            this->moBuffer.Reset();
        }
        
	    ACL_RETRY_CHK( (liResult = apl_read(
		        this->GetHandle(),
		        this->moBuffer.GetWritePtr(),
		        this->moBuffer.GetSpace() ) ) < 0, 5 );
		if (liResult <= 0)
		{
			//EOF
            liResult = -1;
			break;
		}
		
		this->moBuffer.SetWritePtr(liResult);
	}
	
	return auSize - liBufferSize > 0 ? (apl_ssize_t)auSize - liBufferSize : liResult;
}

apl_ssize_t CFileStream::ReadLine( std::string& aoOutput )
{
    apl_ssize_t liResult   = 0;
	acl::CMemoryStream loStream(&this->moBuffer);

    aoOutput = "";

	while(true)
	{
	    apl_ssize_t liLength = this->moBuffer.GetLength();
	    if (liLength > 0)
	    {
            if (aoOutput.size() > 0)
            {
                std::string loTemp;
                if ( (liResult = loStream.ReadLine(loTemp) ) >= 0)
                {
                    aoOutput.append(loTemp);
                    break;
                }
            }
            else
            {
                if ( (liResult = loStream.ReadLine(aoOutput) ) >= 0)
                {
                    break;
                }
            }

            if (*(this->moBuffer.GetReadPtr() + liLength - 1) == '\r')
            {
                //Alignment line end character
                aoOutput.append(this->moBuffer.GetReadPtr(), liLength - 1);
                this->moBuffer.Reset();
                this->moBuffer.Write("\r", 1);
            }
            else
            {
                //Uncomplete line
                aoOutput.append(this->moBuffer.GetReadPtr(), liLength);
            }
        }
        
        this->moBuffer.Reset();
        
	    ACL_RETRY_CHK( (liResult = apl_read(
		        this->GetHandle(),
		        this->moBuffer.GetWritePtr(),
		        this->moBuffer.GetSpace() ) ) < 0, 5 );
		if (liResult <= 0)
		{
			//EOF
			liResult = -1;
            break;
		}
		
		this->moBuffer.SetWritePtr(liResult);
	}
	
	return aoOutput.size() > 0 ? (apl_ssize_t)aoOutput.size() : liResult;
}

apl_ssize_t CFileStream::Read( void* apvBuffer, apl_size_t auSize )
{
	return CFile::Read(apvBuffer, auSize);
}

apl_ssize_t CFileStream::Read( apl_int64_t ai64Offset, void* apvBuffer, apl_size_t auSize )
{
    return CFile::Read(ai64Offset, apvBuffer, auSize);
}

apl_ssize_t CFileStream::ReadV( apl_iovec_t apoIoVec[], apl_size_t auN )
{
	return CFile::ReadV(apoIoVec, auN);
}

apl_ssize_t CFileStream::ReadV( apl_int64_t ai64Offset, apl_iovec_t apoIoVec[], apl_size_t auN )
{
    return CFile::ReadV(ai64Offset, apoIoVec, auN);
}

apl_ssize_t CFileStream::Write( void const* apvBuffer, apl_size_t auSize)
{
	return CFile::Write(apvBuffer, auSize);
}

apl_ssize_t CFileStream::Write( apl_int64_t ai64Offset, void const* apvBuffer, apl_size_t auSize )
{
	return CFile::Write(ai64Offset, apvBuffer, auSize);
}

apl_ssize_t CFileStream::WriteV( apl_iovec_t const apoIoVec[], apl_size_t auN)
{
	return CFile::WriteV(apoIoVec, auN);
}

apl_ssize_t CFileStream::WriteV( apl_int64_t ai64Offset, apl_iovec_t const apoIoVec[], apl_size_t auN )
{
    return CFile::WriteV(ai64Offset, apoIoVec, auN);
}

apl_int64_t CFileStream::Tell(void)
{
    return CFile::Tell() - this->moBuffer.GetLength();
}

bool CFileStream::IsEof(void)
{
    apl_int64_t liFileSize = this->GetFileSize();
    apl_int64_t li64CurrPosition = this->Tell();

    return liFileSize == li64CurrPosition ? true : false;
}

ACL_NAMESPACE_END
