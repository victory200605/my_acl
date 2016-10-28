
#include "acl/Log.h"
#include "acl/Timestamp.h"
#include "acl/DateTime.h"

ACL_NAMESPACE_START

apl_size_t __DefaultLogFormat::operator () (
    apl_int_t aiLevel,
    char const* apcFormat,
    va_list& aoVl,
    char* apcBuffer,
    apl_size_t aiBufferSize )
{
    apl_size_t liLen = 0;
    char*      lpcCurrBuffer = apcBuffer;
    apl_size_t liCurrSize = aiBufferSize;
    CTimestamp loTimestamp;
    CDateTime  loDataTime;
    
    loTimestamp.Update(CTimestamp::PRC_USEC);
    loDataTime.Update(loTimestamp);
    
    apl_snprintf(lpcCurrBuffer, liCurrSize, "%s", loDataTime.Format("%Y-%m-%d %H:%M:%S") );
    liLen = apl_strlen(lpcCurrBuffer);
    lpcCurrBuffer += liLen;
    liCurrSize -= liLen;
    
    apl_snprintf(
        lpcCurrBuffer,
        liCurrSize,
        ".%-6"PRId32"#%"PRId64,
        (apl_int32_t)(loTimestamp.Usec() - loTimestamp.Sec() * 1000000),
        (apl_int64_t)apl_thread_self() );
    liLen = apl_strlen(lpcCurrBuffer);
    lpcCurrBuffer += liLen;
    liCurrSize -= liLen;
    
    switch(aiLevel)
    {
        case LOG_LVL_SYSTEM:
        {
            apl_strncpy(lpcCurrBuffer, " SYSTEM: ", liCurrSize);
            break;
        }
        case LOG_LVL_ERROR:
        {
            apl_strncpy(lpcCurrBuffer, " ERROR: ", liCurrSize);
            break;
        }
        case LOG_LVL_WARN:
        {
            apl_strncpy(lpcCurrBuffer, " WARN: ", liCurrSize);
            break;
        }
        case LOG_LVL_INFO:
        {
            apl_strncpy(lpcCurrBuffer, " INFO: ", liCurrSize);
            break;
        }
        case LOG_LVL_DEBUG:
        {
            apl_strncpy(lpcCurrBuffer, " DEBUG: ", liCurrSize);
            break;
        }
        case LOG_LVL_TRACE:
        {
            apl_strncpy(lpcCurrBuffer, " TRACE: ", liCurrSize);
            break;
        }
        default:
        {
            apl_strncpy(lpcCurrBuffer, " ANY: ", liCurrSize);
            break;
        }
    };
    liLen = apl_strlen(lpcCurrBuffer);
    lpcCurrBuffer += liLen;
    liCurrSize -= liLen;
    
    //HP-UX will return -1 when buffer is no enough
    liLen = apl_vsnprintf(lpcCurrBuffer, liCurrSize, apcFormat, aoVl);
    if ( liLen < 0 || liLen >= liCurrSize )
    {
        liLen = apl_strlen(lpcCurrBuffer);
    }
    lpcCurrBuffer += liLen;
    liCurrSize -= liLen;
    
    if(*(lpcCurrBuffer - 1) != '\n')
    {
        *lpcCurrBuffer++ = '\n';
        liCurrSize -= 1;
    }
    
    return aiBufferSize - liCurrSize;
}

//////////////////////////////////////////////////////////////////////////////////////
CHexFormatter::CHexFormatter(apl_size_t auColumn, apl_size_t auGroup)
    : muColumn(auColumn)
    , muGroup(auGroup)
    , moBuffer(NULL, 0, CMemoryBlock::DONT_DELETE)
{
}
    
CHexFormatter::CHexFormatter( char const* apcInput, apl_size_t auInputLen, apl_size_t auColumn, apl_size_t auGroup)
    : muColumn(auColumn)
    , muGroup(auGroup)
    , moBuffer(NULL, 0, CMemoryBlock::DONT_DELETE)
{
    this->Format(apcInput, auInputLen);
}

char const* CHexFormatter::Format(char const* apcInput, apl_size_t auInputLen, apl_size_t* apuOutputLen)
{
    apl_size_t luColumn    = this->muColumn;
	apl_size_t luGroup     = this->muGroup;
	apl_size_t luDelta     = auInputLen % ( luColumn * luGroup );
	apl_size_t luRow       = auInputLen / ( luColumn * luGroup ) + (luDelta? 1 : 0);
	apl_size_t luRowLenght = 9 /*header adder*/ + luColumn * ( luGroup * 2 + 1) + 2 /* del char*/ + luGroup * luColumn /* printable char */ + 1 /*\n*/;
	apl_size_t luNeedSize  = luRowLenght * luRow + 1 /* 1 end */;
	apl_size_t luStep      = luColumn * luGroup;

	this->moBuffer.Resize(luNeedSize + this->moBuffer.GetLength() );

	apl_memset( this->moBuffer.GetWritePtr(), ' ', this->moBuffer.GetSpace() );
    
    char*  lpcStart          = this->moBuffer.GetWritePtr();
	char*  lpcCurrRow        = lpcStart;
	unsigned char*  lpcCurr  = (unsigned char*)apcInput;
	apl_size_t& luOutputLen  = (apuOutputLen != NULL ? *apuOutputLen : luNeedSize);
	apl_size_t  luCurrPos    = 0;
	apl_size_t  luRowIdx     = 0;
	char*  lpcHeader   = lpcStart;
	char*  lpcHex      = lpcStart;
	char*  lpcPrint    = lpcStart;

#define __CHAR_TO_HEX(ch, ret)                           \
    {                                                    \
        unsigned char lcChLow  = ch & 0x0F;              \
    	unsigned char lcChHigh = ch >> 4;                \
    	                                                 \
    	lcChLow  += lcChLow  < 10 ? '0' : ('A' - 10);    \
        lcChHigh += lcChHigh < 10 ? '0' : ('A' - 10);    \
                                                         \
        *(ret++) = lcChHigh;                             \
        *(ret++) = lcChLow;                              \
        *(ret)   = '\0';                                 \
    }

	for ( apl_size_t luIt = 0; luIt < luRow; luIt++ )
	{
		lpcHeader = lpcCurrRow;
		lpcHex    = lpcHeader + 9;
		lpcPrint  = lpcHeader + 9 /*header adder*/ + luColumn * ( luGroup * 2 + 1) + 2 /* del char*/;

		apl_snprintf( lpcHeader, 10, "%08X:", luRowIdx );

		for ( apl_size_t luCol = 0; luCol < luColumn; luCol++ )
		{
			*lpcHex++ = ' ';

			for ( apl_size_t luIdx = 0; luIdx < luGroup; luIdx++ )
			{
				if ( luCurrPos < auInputLen )
				{
				    __CHAR_TO_HEX(lpcCurr[luCurrPos], lpcHex);

					*lpcPrint++ = ( apl_isprint(lpcCurr[luCurrPos]) ? lpcCurr[luCurrPos] : '.' );
				}
				else
				{
				    __CHAR_TO_HEX(0, lpcHex);
				    
					*lpcPrint++ = '.';
				}

				luCurrPos++;
			}

			if ( luCurrPos >= auInputLen )
			{
			    break;
			}
		}
		
		*lpcHex     = ' ';
		*lpcPrint++ = '\n';
		
		lpcCurrRow = lpcPrint;
		luRowIdx   = luRowIdx + luStep;
	}

	*lpcPrint = '\0';
	
	//full size
	luOutputLen = lpcPrint - lpcStart + 1;
	
	this->moBuffer.SetWritePtr(luOutputLen);

#undef __CHAR_TO_HEX

	return lpcStart;
}
    
char const* CHexFormatter::GetStr(void) const
{
    return this->moBuffer.GetReadPtr();
}

char const* CHexFormatter::GetOutput(void) const
{
    return this->moBuffer.GetReadPtr();
}

apl_size_t CHexFormatter::GetLength(void) const
{
    return this->moBuffer.GetLength();
}

void CHexFormatter::Clear(void)
{
    this->moBuffer.Reset();
}

ACL_NAMESPACE_END
