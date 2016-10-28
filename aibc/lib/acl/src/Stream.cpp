
#include "acl/Stream.h"

ACL_NAMESPACE_START

/////////////////////////////////////// InputStream ////////////////////////////////////////
CInputStream::~CInputStream(void)
{
    ACL_DELETE(this->mpoStream);
}

apl_ssize_t CInputStream::Read( void* apvBuffer, apl_size_t aiSize )
{
    return this->mpoStream->Read(apvBuffer, aiSize);
}

apl_ssize_t CInputStream::ReadUntil( void* apvBuffer, apl_size_t aiSize, const char* apcTag )
{
    apl_size_t liLen = apl_strlen(apcTag);
    char*      lpcBuffer  = (char*)apvBuffer;
    apl_size_t liReadSize = 0;
    apl_int_t  liI = 0;
    
    if (liLen > 2)
    {
        return this->ReadUntilKMP(apvBuffer, aiSize, apcTag, liLen);
    }
    else
    {
        for ( apl_size_t liN = 0; liN < aiSize; liN++ )
        {
            if ( liReadSize < aiSize
                && liI < (apl_int_t)liLen
                && this->mpoStream->Read(&lpcBuffer[liReadSize], 1) == 1 )
            {
                while(true)
                {
                    if (lpcBuffer[liReadSize] == apcTag[liI])
                    {
                        liI++;
                        break;
                    }
                    else if (liI > 0)
                    {
                        liI = 0;
                    }
                    else
                    {
                        break;
                    }
                }
                
                liReadSize++;
            }
            else
            {
                break;
            }
        }
        
        return liReadSize;
    }
}

apl_ssize_t CInputStream::ReadUntilKMP( void* apvBuffer, apl_size_t aiSize, const char* apcTag, apl_size_t aiTagLen )
{
    apl_size_t liLen  = aiTagLen;
    apl_int_t* lpiVal = NULL;
    char*      lpcBuffer  = (char*)apvBuffer;
    apl_size_t liReadSize = 0;
    apl_int_t  liI = 0;
    
    ACL_NEW_N_ASSERT(lpiVal, apl_int_t, liLen);
    
    this->GetNext(apcTag, lpiVal, liLen);
    
    for ( apl_size_t liN = 0; liN < aiSize; liN++ )
    {
        if ( liReadSize < aiSize
            && liI < (apl_int_t)liLen
            && this->mpoStream->Read(&lpcBuffer[liReadSize], 1) == 1 )
        {
            while(true)
            {
                if (liI == -1 || lpcBuffer[liReadSize] == apcTag[liI])
                {
                    liI++;
                    break;
                }
                else
                {
                    liI = lpiVal[liI];
                }
            }
            
            liReadSize++;
        }
        else
        {
            break;
        }
    }
    
    ACL_DELETE_N(lpiVal);
    
    return liReadSize;
}

void CInputStream::GetNext( char const* apcTag, apl_int_t* apiVal, apl_size_t aiN )
{
	apl_int_t liI = 0;
	apl_int_t liJ = -1;
	apiVal[0] = -1;

	while( liI < (apl_int_t)aiN - 1 )
	{
		if (liJ == -1 || apcTag[liI] == apcTag[liJ])
		{
			++liI;
			++liJ;

			if (apcTag[liI] != apcTag[liJ])
			{
				apiVal[liI] = liJ;
			}
			else
			{
				apiVal[liI] = apiVal[liJ];
			}
		}
		else
		{
			liJ = apiVal[liJ];
		}
	}
}
    
apl_ssize_t CInputStream::ReadToH( apl_uint16_t& au16Val )
{
    if ( this->mpoStream->Read( &au16Val, sizeof(au16Val) ) != sizeof(au16Val) )
    {
        return -1;
    }
    else
    {
        au16Val = apl_ntoh16(au16Val);
        return sizeof(au16Val);
    }
}
    
apl_ssize_t CInputStream::ReadToH( apl_uint32_t& au32Val )
{
    if ( this->mpoStream->Read( &au32Val, sizeof(au32Val) ) != sizeof(au32Val) )
    {
        return -1;
    }
    else
    {
        au32Val = apl_ntoh32(au32Val);
        return sizeof(au32Val);
    }
}

apl_ssize_t CInputStream::ReadToH( apl_uint64_t& au64Val )
{
    if ( this->mpoStream->Read( &au64Val, sizeof(au64Val) ) != sizeof(au64Val) )
    {
        return -1;
    }
    else
    {
        au64Val = apl_ntoh64(au64Val);
        return sizeof(au64Val);
    }
}

apl_ssize_t CInputStream::ReadToH( apl_int16_t& ai16Val )
{
    if ( this->mpoStream->Read( &ai16Val, sizeof(ai16Val) ) != sizeof(ai16Val) )
    {
        return -1;
    }
    else
    {
        ai16Val = apl_ntoh16(ai16Val);
        return sizeof(ai16Val);
    }
}
    
apl_ssize_t CInputStream::ReadToH( apl_int32_t& ai32Val )
{
    if ( this->mpoStream->Read( &ai32Val, sizeof(ai32Val) ) != sizeof(ai32Val) )
    {
        return -1;
    }
    else
    {
        ai32Val = apl_ntoh32(ai32Val);
        return sizeof(ai32Val);
    }
}

apl_ssize_t CInputStream::ReadToH( apl_int64_t& ai64Val )
{
    if ( this->mpoStream->Read( &ai64Val, sizeof(ai64Val) ) != sizeof(ai64Val) )
    {
        return -1;
    }
    else
    {
        ai64Val = apl_ntoh64(ai64Val);
        return sizeof(ai64Val);
    }
}

/////////////////////////////////////// OutputStream ////////////////////////////////////////
COutputStream::~COutputStream(void)
{
    ACL_DELETE(this->mpoStream);
}

apl_ssize_t COutputStream::Write( void const* apvBuffer, apl_size_t aiSize )
{
    return this->mpoStream->Write(apvBuffer, aiSize);
}

apl_ssize_t COutputStream::Write( char const* apcPtr )
{
    return this->mpoStream->Write( apcPtr, apl_strlen(apcPtr) + 1 );
}
    
apl_ssize_t COutputStream::WriteToN( apl_uint16_t au16Val )
{
    au16Val = apl_hton16(au16Val);
    
    if ( this->mpoStream->Write( &au16Val, sizeof(au16Val) ) != sizeof(au16Val) )
    {
        return -1;
    }
    else
    {
        return sizeof(au16Val);
    }
}
    
apl_ssize_t COutputStream::WriteToN( apl_uint32_t au32Val )
{
    au32Val = apl_hton32(au32Val);
    
    if ( this->mpoStream->Write( &au32Val, sizeof(au32Val) ) != sizeof(au32Val) )
    {
        return -1;
    }
    else
    {
        return sizeof(au32Val);
    }
}

apl_ssize_t COutputStream::WriteToN( apl_uint64_t au64Val )
{
    au64Val = apl_hton64(au64Val);
    
    if ( this->mpoStream->Write( &au64Val, sizeof(au64Val) ) != sizeof(au64Val) )
    {
        return -1;
    }
    else
    {
        return sizeof(au64Val);
    }
}

apl_ssize_t COutputStream::WriteToN( apl_int16_t ai16Val )
{
    ai16Val = apl_hton16(ai16Val);
    
    if ( this->mpoStream->Write( &ai16Val, sizeof(ai16Val) ) != sizeof(ai16Val) )
    {
        return -1;
    }
    else
    {
        return sizeof(ai16Val);
    }
}
    
apl_ssize_t COutputStream::WriteToN( apl_int32_t ai32Val )
{
    ai32Val = apl_hton32(ai32Val);
    
    if ( this->mpoStream->Write( &ai32Val, sizeof(ai32Val) ) != sizeof(ai32Val) )
    {
        return -1;
    }
    else
    {
        return sizeof(ai32Val);
    }
}

apl_ssize_t COutputStream::WriteToN( apl_int64_t ai64Val )
{
    ai64Val = apl_hton64(ai64Val);
    
    if ( this->mpoStream->Write( &ai64Val, sizeof(ai64Val) ) != sizeof(ai64Val) )
    {
        return -1;
    }
    else
    {
        return sizeof(ai64Val);
    }
}

ACL_NAMESPACE_END
