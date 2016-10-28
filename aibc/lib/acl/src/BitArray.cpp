
#include "acl/BitArray.h"

ACL_NAMESPACE_START

CBitArray::CBitArray(void)
{
    this->Init(0, APL_NULL, 0, DONT_COPY);
}
    
CBitArray::CBitArray( apl_size_t auSize )
{
    this->Init(auSize, APL_NULL, 0, DONT_COPY);
}
 
CBitArray::CBitArray( char const* apcBitStr, apl_size_t auLength, EOption aeOpt )
{
    this->Init(0, apcBitStr, auLength, aeOpt);
}

CBitArray::~CBitArray(void)
{
    if (this->mbIsDelete)
    {
        ACL_DELETE_N(this->mpcStart);
    }
}

void CBitArray::Init( apl_size_t auSize, char const* apcBitStr, apl_size_t auLength, EOption aeOpt )
{
    this->mpcStart = APL_NULL;
    this->muSize = 0;
    this->muBitSize = 0;
    this->muCapacity = 0;
    this->mbIsDelete = false;

    if (apcBitStr != APL_NULL)
    {
        if (aeOpt == DO_COPY)
        {
            this->Resize(auLength * 8);
            apl_memcpy(this->mpcStart, apcBitStr, auLength);
        }
        else
        {
            this->mpcStart = (unsigned char*)apcBitStr;
            this->muSize = auLength;
            this->muBitSize = auLength * 8;
            this->muCapacity = auLength;
            this->mbIsDelete = false;
        }
    }
    else
    {
        this->Resize(auSize);
    }
}
    
void CBitArray::Resize( apl_size_t auSize )
{
    apl_size_t luNewSize = auSize / 8 + (auSize % 8 > 0 ? 1 : 0);
    apl_size_t luOldSize = this->muSize;

    if (this->muCapacity < luNewSize )
    {
        unsigned char* lpcTemp = APL_NULL;
        ACL_NEW_N_ASSERT(lpcTemp, unsigned char, luNewSize);

        if (this->mpcStart != APL_NULL)
        {
            apl_memcpy(lpcTemp, this->mpcStart, luOldSize);
            apl_memset(lpcTemp + luOldSize, 0, (luNewSize - luOldSize) );
        }
        else
        {
            apl_memset(lpcTemp, 0, luNewSize);
        }
        
        if (this->mbIsDelete)
        {
            ACL_DELETE_N(this->mpcStart);
        }

        this->mpcStart = lpcTemp;
        this->muCapacity = luNewSize;
    }

    this->muSize = luNewSize;
    this->muBitSize = auSize;
}

void CBitArray::SetAll( bool abX )
{
    apl_size_t luSize = this->muBitSize / 8;
    apl_size_t luBits = this->muBitSize % 8;
    
    if (this->muSize > 0)
    {
        if (abX)
        {
            apl_memset(this->mpcStart, 0xFF, this->muSize);
        }
        else
        {
            apl_memset(this->mpcStart, 0x00, this->muSize);
        }
    }

    if (luBits > 0)
    {
        unsigned char& lcTemp = this->mpcStart[luSize];

        lcTemp = lcTemp >> (8 - luBits);
        lcTemp = lcTemp << (8 - luBits);
    }
}
    
void CBitArray::Set( apl_size_t auPos, bool abX )
{
    apl_size_t luIndex = auPos / 8;
    unsigned char lcBit = 0x80 >> (auPos % 8);

    ACL_ASSERT(auPos < this->muBitSize);

    if (abX)
    {
        ACL_SET_BITS(this->mpcStart[luIndex], lcBit);
    }
    else
    {
        ACL_CLR_BITS(this->mpcStart[luIndex], lcBit);
    }
}

bool CBitArray::Get( apl_size_t auPos )
{
    apl_size_t luIndex = auPos / 8;
    unsigned char lcBit = 0x80 >> (auPos % 8);

    ACL_ASSERT(auPos < this->muBitSize);

    return ACL_BIT_ENABLED(this->mpcStart[luIndex], lcBit);
}

apl_size_t CBitArray::Find( apl_size_t auPos, bool abX )
{
    apl_size_t luIndex = auPos / 8;
    apl_size_t luOffset = auPos % 8;
    unsigned char lcSkip = abX ? 0x00 : 0xFF;

    for (apl_size_t luN = luIndex; luN < this->muSize; luN++)
    {
        unsigned char& lcTemp = this->mpcStart[luN];
        
        if (lcTemp == lcSkip)
        {
            continue;
        }

        for (apl_size_t luB = luOffset; luB < 8; luB++)
        {
            unsigned char lcBit = 0x80 >> luB;
            if (ACL_BIT_ENABLED(lcTemp, lcBit) == abX)
            {
                return luN * 8 + luB;
            }
        }
    }

    return INVALID_POS;
}

apl_size_t CBitArray::Find( bool abX )
{
    return this->Find(0, abX);
}

apl_size_t CBitArray::GetSize(void)
{
    return this->muBitSize;
}

char const* CBitArray::GetCStr(void)
{
    return this->mpcStart == APL_NULL ? "" : (char const*)this->mpcStart;
}

apl_size_t CBitArray::GetCLength(void)
{
    return this->muSize;
}

ACL_NAMESPACE_END
