
#include "anf/protocol/CmdLineMessage.h"

ANF_NAMESPACE_START

CCmdLineMessage::CCmdLineMessage(void)
    : moSpliter(" ")
    , mpcBuffer(APL_NULL)
    , muLineMaxLength(1024)
{
    ACL_NEW_N_ASSERT(this->mpcBuffer, char, this->muLineMaxLength);
}

CCmdLineMessage::CCmdLineMessage( apl_size_t auLineMaxLength )
    : moSpliter(" ")
    , mpcBuffer(APL_NULL)
    , muLineMaxLength(auLineMaxLength)
{
    ACL_NEW_N_ASSERT(this->mpcBuffer, char, this->muLineMaxLength);
}

CCmdLineMessage::~CCmdLineMessage(void)
{
    ACL_DELETE_N(this->mpcBuffer);
}

apl_int_t CCmdLineMessage::Decode( acl::CMemoryBlock* apoBuffer )
{
    char const* lpcBuffer = apoBuffer->GetReadPtr();
    apl_size_t  luLength = apoBuffer->GetLength();

    char* lpcEnd = (char*)apl_memchr(lpcBuffer, '\n', luLength);
    if (lpcEnd != APL_NULL)
    {
        apl_size_t luLineLen = lpcEnd - lpcBuffer;
        
        if (luLineLen > 0 && *(lpcEnd - 1) == '\r')
        {
            luLineLen -= 1;
        }

        //Skip blank line
        if (luLineLen == 0)
        {
            apoBuffer->SetReadPtr(lpcEnd + 1);
            return 1;
        }
    
        //Cmd line to long
        if (luLineLen >= this->muLineMaxLength)
        {
            return -1;
        }

        //Get a cmdline
        apl_memcpy(this->mpcBuffer, lpcBuffer, luLineLen);
        this->mpcBuffer[luLineLen] = '\0';

        this->moSpliter.Parse(this->mpcBuffer);

        apoBuffer->SetReadPtr(lpcEnd + 1);

        return 0;
    }
    else
    {
        //Continue
        return 1;
    }
}

apl_int_t CCmdLineMessage::Encode( acl::CMemoryBlock* apoBuffer )
{
    apl_size_t luLength = apl_strlen(this->mpcBuffer);
    apl_size_t luPaddingLen =  2;
   
    //Must end of CRLF
    if (luLength >= 2 
        && this->mpcBuffer[luLength - 2] == '\r'
        && this->mpcBuffer[luLength - 1] == '\n')
    {
        luPaddingLen = 0;
    }

    //Check buffer space
    if (apoBuffer->GetSpace() < luLength + luPaddingLen)
    {
        apoBuffer->Resize(apoBuffer->GetLength() + luLength + luPaddingLen);
    }

    apl_memcpy(apoBuffer->GetWritePtr(), this->mpcBuffer, luLength);
    apoBuffer->SetWritePtr(luLength);

    if (luPaddingLen > 0)
    {
        apl_memcpy(apoBuffer->GetWritePtr(), "\r\n", 2);
        apoBuffer->SetWritePtr(luPaddingLen);
    }

    return 0;
}

void CCmdLineMessage::CmdLine( char const* apcPattern, ... )
{
    va_list loVaList;

    va_start(loVaList, apcPattern);

    apl_vsnprintf(this->mpcBuffer, this->muLineMaxLength, apcPattern, loVaList);

    va_end(loVaList);
}

apl_size_t CCmdLineMessage::GetArgc(void)
{
    return this->moSpliter.GetSize();
}

char const* CCmdLineMessage::GetArgv( apl_size_t auN )
{
    return this->moSpliter.GetField(auN);
}

ANF_NAMESPACE_END

