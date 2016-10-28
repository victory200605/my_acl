
#include "CmdLineMessage.h"

CCmdLineMessage::CCmdLineMessage(void)
{
}

CCmdLineMessage::~CCmdLineMessage(void)
{
}

apl_int_t CCmdLineMessage::Decode( acl::CMemoryBlock* apoBuffer )
{
    char const* lpcBuffer = apoBuffer->GetReadPtr();
    apl_size_t  luLength = apoBuffer->GetLength();

    if (luLength >= 2 
        && lpcBuffer[luLength - 2] == '\r' 
        && lpcBuffer[luLength - 1] == '\n')
    {
        if (luLength >= sizeof(this->macBuffer) )
        {
            return -1;
        }
        else
        {
            apl_memcpy(this->macBuffer, lpcBuffer, luLength);
            this->macBuffer[luLength] = '\0';

            this->moSpliter.Parse(this->macBuffer);

            return 0;
        }
    }
    else
    {
        //Continue
        return 1;
    }
}

apl_int_t CCmdLineMessage::Encode( acl::CMemoryBlock* apoBuffer )
{
    apl_size_t luLength = apl_strlen(this->macBuffer);
    apl_size_t luPaddingLen =  0;
   
    if (luLength > 2 
        && this->macBuffer[luLength - 2] == '\r'
        && this->macBuffer[luLength - 1] == '\n')
    {
        luPaddingLen = 2;
    }

    if (apoBuffer->GetSpace() < luLength + luPaddingLen)
    {
        apoBuffer->Resize(apoBuffer->GetLength() + luLength + luPaddingLen);
    }

    apl_memcpy(apoBuffer->GetWritePtr(), this->macBuffer, luLength);
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

    apl_vsnprintf(this->macBuffer, sizeof(this->macBuffer), apcPattern, loVaList);

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


