#include "apl/apl.h"
#include "acl/Utility.h"
#include "acl/MemoryBlock.h"
#include "acl/MemoryStream.h"
#include "acl/IndexDict.h"

ACL_NAMESPACE_START

static inline apl_size_t U32ToStr( apl_uint32_t auNumber, char* apcOutput )
{
    apl_int32_t liTemp = 0;
    apl_size_t  liN = 0;

    do
    {
        liTemp = auNumber % 10;
        apcOutput[liN++] = liTemp + 48;
    }while (auNumber /= 10);
    
    for (apl_size_t k = 0; k < liN / 2; k++)
    {
        char lcTemp;
        lcTemp = apcOutput[k];
        apcOutput[k] = apcOutput[liN - k -1];
        apcOutput[liN - k -1] = lcTemp;
    }

    return liN;
}

static inline apl_size_t StrToU32( char const* apcInput, apl_size_t auLen, apl_uint32_t* apuNumber )
{
    char const*  lpcFirst = apcInput;
    char const*  lpcLast = apcInput + auLen;
    apl_uint32_t luTemp = 0;

    while (lpcFirst < lpcLast && *lpcFirst <= '9'
           && *lpcFirst >= '0' )
    {
        luTemp *= 10;
        luTemp += *lpcFirst++ - 48;
    }

    *apuNumber = luTemp;

    return lpcFirst - apcInput;
}

static inline apl_ssize_t BReadInt(char const* apcBuf, apl_size_t auLen, apl_uint32_t* apu32Value)
{
    // "i" \d+ "e"
    if (auLen < 2 || *apcBuf != 'i' )
    {
        return -1;
    }

    apl_size_t liResult = StrToU32(apcBuf + 1, auLen - 1, apu32Value);
    if (apcBuf[1 + liResult] != 'e')
    {
        return -1;
    }

    return liResult + 2;
}


static inline apl_ssize_t BReadStr(char const* apcBuf, apl_size_t auLen, char const** appcValue, apl_size_t* apuValueLen)
{
    apl_uint32_t luLen;
    apl_size_t liResult = StrToU32(apcBuf, auLen, &luLen);
    if (apcBuf[liResult] != ':')
    {
        return -1;
    }

    if (auLen < liResult + 1 + luLen)
    {
        return -1;
    }

    *appcValue = apcBuf + liResult + 1;
    *apuValueLen = luLen;

    return liResult + 1 + luLen;
}

//////////////////////////////////////////////////////////////////////////////////////////////
CIndexDictAllocatee::CIndexDictAllocatee( apl_size_t auSmallSize, apl_size_t auBlockSize )
    : muSmallSize(auSmallSize)
    , muBlockSize(auBlockSize)
    , moBlocks(64)
    , mpoCurrAllocPtr(APL_NULL)
    , muSize(0)
{
    ACL_ASSERT(this->muSmallSize < this->muBlockSize);
    this->Init();
}

CIndexDictAllocatee::~CIndexDictAllocatee(void)
{
    this->Release();
}

void* CIndexDictAllocatee::Allocate( apl_size_t auSize )
{
    if (auSize > this->muBlockSize || auSize > this->muSmallSize)
    {
        char* lpoNew = APL_NULL;
        ACL_NEW_N_ASSERT(lpoNew, char, auSize);
        this->moBlocks.push_back(lpoNew);

        return (void*)lpoNew;
    }
    else if (auSize > this->muSize)
    {
        this->Init();
    }
    
    char* lpoTemp = this->mpoCurrAllocPtr;

    this->mpoCurrAllocPtr += auSize;
    this->muSize -= auSize;

    return (void*)lpoTemp;
}

void CIndexDictAllocatee::Reset(void)
{
    this->Release();
    this->Init();
}

void CIndexDictAllocatee::Swap( CIndexDictAllocatee& aoRhs )
{
    std::swap(this->muSmallSize, aoRhs.muSmallSize);
    std::swap(this->muBlockSize, aoRhs.muBlockSize);
    this->moBlocks.swap(aoRhs.moBlocks);
    std::swap(this->mpoCurrAllocPtr, aoRhs.mpoCurrAllocPtr);
    std::swap(this->muSize, aoRhs.muSize);
}

void CIndexDictAllocatee::Init(void)
{
    char* lpcTemp = APL_NULL;
    ACL_NEW_N_ASSERT(lpcTemp, char, this->muBlockSize);
    this->moBlocks.push_back(lpcTemp);

    this->mpoCurrAllocPtr = lpcTemp;
    this->muSize = this->muBlockSize;
}

void CIndexDictAllocatee::Release(void)
{
    for (apl_size_t luN = 0; luN < this->moBlocks.size(); luN++)
    {
        char* lpcBlock = this->moBlocks[luN];
        ACL_DELETE_N(lpcBlock);
    }

    this->moBlocks.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////
CIndexDictValue::CIndexDictValue(void)
    : mpcStart(APL_NULL)
    , muLength(0)
{
}

CIndexDictValue::CIndexDictValue( char const* apcInput, CIndexDictAllocatee& aoAllocatee )
    : mpcStart(APL_NULL)
    , muLength(0)
{
    this->Assign(apcInput, apl_strlen(apcInput), aoAllocatee);
}

CIndexDictValue::CIndexDictValue( void const* apvInput, apl_size_t auLen, CIndexDictAllocatee& aoAllocatee )
    : mpcStart(APL_NULL)
    , muLength(0)
{
    this->Assign(apvInput, auLen, aoAllocatee);
}

CIndexDictValue::CIndexDictValue( void const* apvInput, apl_size_t auLen )
    : mpcStart((char*)apvInput)
    , muLength(auLen)
{
}

CIndexDictValue::~CIndexDictValue(void)
{
}

void CIndexDictValue::Assign( char const* apcInput, CIndexDictAllocatee& aoAllocatee )
{
    this->Assign(apcInput, apl_strlen(apcInput), aoAllocatee);
}

void CIndexDictValue::Assign( void const* apvInput, apl_size_t auLen, CIndexDictAllocatee& aoAllocatee )
{
    this->mpcStart = (char*)aoAllocatee.Allocate(auLen + 1);
    this->muLength = auLen;

    apl_memcpy(this->mpcStart, apvInput, auLen);
    this->mpcStart[auLen] = '\0';
}

/*inline*/ bool CIndexDictValue::operator == ( CIndexDictValue const& aoRhs ) const
{
    if (this->muLength != aoRhs.muLength || this->muLength == 0)
    {
        return false;
    }

    if (apl_memcmp(this->mpcStart, aoRhs.mpcStart, this->muLength) != 0)
    {
        return false;
    }

    return true;
}

/*inline*/ bool CIndexDictValue::operator != ( CIndexDictValue const& aoRhs ) const
{
    return !(*this == aoRhs);
}

/*inline*/ bool CIndexDictValue::operator == ( char const* apcRhs ) const
{
    if (this->mpcStart == APL_NULL)
    {
        return false;
    }
    else if (apl_strcmp(this->mpcStart, apcRhs) != 0)
    {
        return false;
    }

    return true;
}

/*inline*/ bool CIndexDictValue::operator != ( char const* apcRhs ) const
{
    return !(*this == apcRhs);
}

/*inline*/ bool CIndexDictValue::operator == ( std::string const& aoRhs ) const
{
    if (this->mpcStart == APL_NULL)
    {
        return false;
    }
    else if (this->muLength != aoRhs.length() 
        || apl_memcmp(this->mpcStart, aoRhs.c_str(), this->muLength) != 0)
    {
        return false;
    }

    return true;
}

/*inline*/ bool CIndexDictValue::operator != ( std::string const& aoRhs ) const
{
    return !(*this == aoRhs);
}

/*inline*/ char const* CIndexDictValue::GetStr(void) const
{
    return this->mpcStart == APL_NULL ? "" : this->mpcStart;
}

/*inline*/ apl_size_t CIndexDictValue::GetLength(void) const
{
    return this->muLength;
}

/*inline*/ bool CIndexDictValue::IsEmpty(void) const
{
    return this->mpcStart == APL_NULL || this->muLength == 0;
}

/*inline*/ void CIndexDictValue::Clear(void)
{
    this->mpcStart = APL_NULL;
    this->muLength = 0;
}

/*inline*/ char const* CIndexDictValue::c_str(void) const
{
    return this->mpcStart == APL_NULL ? "" : this->mpcStart;
}

/*inline*/ apl_size_t CIndexDictValue::size(void) const
{
    return this->muLength;
}

/*inline*/ apl_size_t CIndexDictValue::length(void) const
{
    return this->muLength;
}

/*inline*/ bool CIndexDictValue::empty(void) const
{
    return this->mpcStart == APL_NULL || this->muLength == 0;
}

//////////////////////////////// public ////////////////////////////////
const std::string CIndexDict::NULL_VAL = "";

CIndexDict::CIndexDict(
    apl_size_t auSize, 
    apl_size_t auItemBlockSize,
    apl_size_t auSmallValueSize, 
    apl_size_t auValueBlockSize )
    : muBlockSize(auSize)
    , moDict(auSize)
    , moItemAllocatee(auItemBlockSize == 0 ? auSize : auItemBlockSize)
    , moAllocatee(auSmallValueSize, auValueBlockSize)
{
    //this->moDict.resize(auSize);
}

CIndexDict::CIndexDict( CIndexDict const& aoRhs )
    : muBlockSize(aoRhs.muBlockSize)
    , moDict(aoRhs.muBlockSize)
    , moItemAllocatee(aoRhs.moItemAllocatee.GetBlockSize() )
    , moAllocatee(aoRhs.moAllocatee.GetSmallSize(), aoRhs.moAllocatee.GetBlockSize() )
{
    this->CopyFrom(aoRhs);
}

CIndexDict& CIndexDict::operator = ( CIndexDict const& aoRhs )
{
    this->Clear();
    this->CopyFrom(aoRhs);

    return *this;
}

CIndexDict::~CIndexDict(void)
{
}

bool CIndexDict::operator == ( CIndexDict const& aoRhs ) const
{
    if (this->moDict.size() != aoRhs.moDict.size() )
    {
        return false;
    }
    
    //compare all
    for (apl_size_t luN = 0; luN < this->moDict.size(); ++luN)
    {
        if (this->moDict[luN] == APL_NULL && aoRhs.moDict[luN] == APL_NULL)
        {
            continue;
        }

        if ( this->moDict[luN] == APL_NULL 
             || aoRhs.moDict[luN] == APL_NULL
             || this->moDict[luN]->size() != aoRhs.moDict[luN]->size() )
        {
            return false;
        }

        for (apl_size_t luK = 0; luK < this->moDict[luN]->size(); ++luK)
        {
            ValueType& loLhs = this->moDict[luN]->operator [] (luK);
            ValueType& loRhs = aoRhs.moDict[luN]->operator [] (luK);
            
            if (loLhs != loRhs)
            {
                return false;
            }
        }
    }

    return true;
}

bool CIndexDict::operator != (CIndexDict const& aoRhs) const
{
    return !(*this == aoRhs);
}

apl_int_t CIndexDict::Decode(CMemoryBlock* apoMB)
{
    char const* lpcChr = apoMB->GetReadPtr();
    apl_size_t luLen = apoMB->GetLength();
    apl_ssize_t liResult;

    if (luLen < 2 || *lpcChr != 'd')
    { // d <...> e
        apl_errprintf("error <dict> begin\n");
        return -1;
    }     

    ++lpcChr;
    --luLen;

    while (luLen && *lpcChr != 'e') // end of dict
    {
        apl_uint32_t lu32Out;

        liResult = BReadInt(lpcChr, luLen, &lu32Out);

        if (liResult < 0)
        {     
            apl_errprintf("error <integer>\n");
            return -1;
        }     

        assert((apl_size_t)liResult <= luLen);

        lpcChr += liResult;
        luLen -= liResult;

        if (!luLen || lpcChr[0] != 'l')
        { // l <...> e
            apl_errprintf("error <list> begin\n");
            return -1;
        }     

        ++lpcChr;
        --luLen;

        //ITEMS& loItems = loDict.AcquireItems(lu32Out);

        while (luLen && *lpcChr != 'e') // end of list and end of dict
        {     
            char const* lpcOut;
            apl_size_t luOut;

            liResult = BReadStr(lpcChr, luLen, &lpcOut, &luOut);

            if (liResult < 0)
            {     
                apl_errprintf("error <string>\n");
                return -1;
            }     

            assert((apl_size_t)liResult <= luLen);

            lpcChr += liResult;
            luLen -= liResult;

            this->Add(lu32Out, lpcOut, luOut);
            //loItems.push_back(VALUE(lpcOut, luOut));
        }

        if (luLen < 1 || *lpcChr != 'e')
        {
            apl_errprintf("error <list> end\n");
            return -1;
        }

        ++lpcChr;
        --luLen;
    }

    if (luLen < 1 || *lpcChr != 'e')
    {
        apl_errprintf("error <dict> end\n");

        return -1;
    }

    ++lpcChr;
    --luLen;

    apoMB->SetReadPtr(apoMB->GetReadPtr() + (apoMB->GetLength() - luLen));

    return 0;
}

apl_int_t CIndexDict::Encode( CMemoryBlock* apoBuffer )
{
    apl_size_t luIndex = 0;
    char* lpcWritePtr  = apoBuffer->GetWritePtr();
    apl_size_t luSpace = apoBuffer->GetSpace();

    APL_RETURN_IF(-1, luSpace < 2);//base space

    *lpcWritePtr++ = 'd';
    luSpace--;

    for (apl_size_t i = 0; i < this->moDict.size(); ++luIndex, ++i)
    {
        if (this->moDict[i] == APL_NULL)
        {
            continue;
        }
    
        APL_RETURN_IF(-1, luSpace < 14); //1 + 10(uint32 max length) + 1 + 1 + 2(last end char 'ee')
        
        *lpcWritePtr++ = 'i';
        apl_int_t liResult = U32ToStr(luIndex, lpcWritePtr);
        lpcWritePtr   += liResult;
        *lpcWritePtr++ = 'e';
        *lpcWritePtr++ = 'l';
        luSpace       -= (3 + liResult);
        
        for (apl_size_t k = 0; k < this->moDict[i]->size(); ++k)
        {
            ValueType& loValue = (*this->moDict[i])[k];

            APL_RETURN_IF(-1, luSpace < 12 + loValue.size() );//10(uint32 max length) + 1 + 2(last end char 'ee')

            apl_int_t liN  = U32ToStr(loValue.size(), lpcWritePtr);
            lpcWritePtr   += liN;
            *lpcWritePtr++ = ':';
            apl_memcpy(lpcWritePtr, loValue.c_str(), loValue.size() );
            lpcWritePtr   += loValue.size();
            luSpace       -= (liN + 1 + loValue.size() );
        }

        *lpcWritePtr++ = 'e';
        luSpace       -= 1;
    }

    *lpcWritePtr++ = 'e';
    apoBuffer->SetWritePtr(lpcWritePtr);

    return 0;
}

char const* CIndexDict::Get(apl_size_t auTag, char const* apcDefault) const
{
    return Get2(auTag, 0, apcDefault);
}

CIndexDict::ValueType CIndexDict::GetStr(apl_size_t auTag, std::string const& aoDefault) const
{
    return GetStr2(auTag, 0, aoDefault);
}

apl_intmax_t CIndexDict::GetInt(apl_size_t auTag, apl_intmax_t aimDefault) const
{
    return GetInt2(auTag, 0, aimDefault);
}

char const* CIndexDict::Get2(apl_size_t auTag, apl_size_t auIndex, char const* apcDefault) const
{
    ValueType* lpoValue = this->GetValue(auTag, auIndex);
    if (lpoValue == APL_NULL)
    {
        return apcDefault;
    }

    return lpoValue->GetStr();
}

CIndexDict::ValueType CIndexDict::GetStr2(apl_size_t auTag, apl_size_t auIndex, std::string const& aoDefault) const
{
    ValueType* lpoValue = this->GetValue(auTag, auIndex);
    if (lpoValue == APL_NULL)
    {
        return ValueType(aoDefault.c_str(), aoDefault.length() );
    }

    return *lpoValue;
}

apl_intmax_t CIndexDict::GetInt2(apl_size_t auTag, apl_size_t auIndex, apl_intmax_t aimDefault) const
{
    char const* lpcResult = this->Get2(auTag, auIndex, APL_NULL);
    if (lpcResult == APL_NULL)
    {
        return aimDefault;
    }

    return apl_strtoimax(lpcResult, APL_NULL, 10);
}
    
apl_int_t CIndexDict::Set(apl_size_t auTag, char const* apcValue)
{
    ValueType& loValue = this->Insert(auTag, 0);

    loValue.Assign(apcValue, this->moAllocatee);

    return 0;
}

apl_int_t CIndexDict::Set(apl_size_t auTag, void const* apvValue, apl_size_t auLen)
{
    ValueType& loValue = this->Insert(auTag, 0);

    loValue.Assign(apvValue, auLen, this->moAllocatee);

    return 0;
}

apl_int_t CIndexDict::Set(apl_size_t auTag, std::string const& aoValue)
{
    ValueType& loValue = this->Insert(auTag, 0);

    loValue.Assign(aoValue.c_str(), aoValue.length(), this->moAllocatee);

    return 0;
}

apl_int_t CIndexDict::Set(apl_size_t auTag, apl_intmax_t aimValue)
{
    char lacBuffer[24];
    apl_snprintf(lacBuffer, sizeof(lacBuffer), "%"APL_PRIdMAX, aimValue);

    return this->Set(auTag, lacBuffer);
}

apl_int_t CIndexDict::Set2(apl_size_t auTag, apl_size_t auIndex, char const* apcValue)
{
    ValueType& loValue = this->Insert(auTag, auIndex);

    loValue.Assign(apcValue, this->moAllocatee);

    return 0;
}

apl_int_t CIndexDict::Set2(apl_size_t auTag, apl_size_t auIndex, void const* apvValue, apl_size_t auLen)
{
    ValueType& loValue = this->Insert(auTag, auIndex);

    loValue.Assign(apvValue, auLen, this->moAllocatee);

    return 0;
}

apl_int_t CIndexDict::Set2(apl_size_t auTag, apl_size_t auIndex, std::string const& aoValue)
{
    ValueType& loValue = this->Insert(auTag, auIndex);

    loValue.Assign(aoValue.c_str(), aoValue.length(), this->moAllocatee);

    return 0;
}

apl_int_t CIndexDict::Set2(apl_size_t auTag, apl_size_t auIndex, apl_intmax_t aimValue)
{
    char lacBuf[24];
    apl_snprintf(lacBuf, sizeof(lacBuf), "%"APL_PRIdMAX, aimValue);

    return this->Set2(auTag, auIndex, lacBuf);
}

apl_int_t CIndexDict::Add(apl_size_t auTag, char const* apcValue)
{
    ValueType& loValue = this->PushBack(auTag);

    loValue.Assign(apcValue, this->moAllocatee);

    return 0;
}

apl_int_t CIndexDict::Add(apl_size_t auTag, void const* apvValue, apl_size_t auLen)
{
    ValueType& loValue = this->PushBack(auTag);

    loValue.Assign(apvValue, auLen, this->moAllocatee);

    return 0;
}

apl_int_t CIndexDict::Add(apl_size_t auTag, std::string const& aoValue)
{
    ValueType& loValue = this->PushBack(auTag);

    loValue.Assign(aoValue.c_str(), aoValue.length(), this->moAllocatee);

    return 0;
}

apl_int_t CIndexDict::Add(apl_size_t auTag, apl_intmax_t aimValue)
{
    char lacBuf[24];

    apl_snprintf(lacBuf, sizeof(lacBuf), "%"APL_PRIdMAX, aimValue);
    
    return this->Add(auTag, lacBuf);
}

apl_int_t CIndexDict::Del(apl_size_t auTag)
{
    if (auTag < this->moDict.size() )
    {
        this->moDict[auTag]->clear();
        return 0;
    }
    else
    {
        return -1;
    }
}

apl_int_t CIndexDict::Del(apl_size_t auTag, apl_size_t auIndex)
{
    if (auTag < this->moDict.size() )
    {
        ItemType*  lpoItem  = this->moDict[auTag];
        if (auIndex < lpoItem->size() )
        {
            lpoItem->erase(auIndex);

            return 0;
        }
    }

    return -1;
}

void CIndexDict::Clear(void)
{
    this->moDict.clear();
    this->moAllocatee.Reset();
    this->moItemAllocatee.Reset();
}

void CIndexDict::Swap( CIndexDict& aoRhs )
{
    std::swap(this->muBlockSize, aoRhs.muBlockSize);
    this->moDict.swap(aoRhs.moDict);
    this->moAllocatee.Swap(aoRhs.moAllocatee);
    this->moItemAllocatee.Swap(aoRhs.moItemAllocatee);
}

apl_size_t CIndexDict::Count(apl_size_t auTag) const
{
    if (auTag < this->moDict.size() && this->moDict[auTag] != APL_NULL)
    {
        return this->moDict[auTag]->size();
    }
    else
    {
        return 0;
    }
}

bool CIndexDict::Has(apl_size_t auTag) const
{
    if (auTag < this->moDict.size() && this->moDict[auTag] != APL_NULL)
    {
        return !this->moDict[auTag]->empty();
    }
    else
    {
        return false;
    }
}

apl_size_t CIndexDict::NextTag( apl_size_t auIndex ) const
{
    for (apl_size_t luN = auIndex; luN < this->moDict.size(); luN++)
    {
        if (this->moDict[luN] == APL_NULL || this->moDict[luN]->empty() )
        {
            continue;
        }

        return luN;
    }

    return ~(apl_size_t)0;
}

apl_size_t CIndexDict::TotalTags(void) const
{
    apl_size_t luSum = 0;

    for (apl_size_t luN = 0; luN < this->moDict.size(); luN++)
    {
        if (this->moDict[luN] == APL_NULL || this->moDict[luN]->empty() )
        {
            continue;
        }
        
        luSum++;
    }

    return luSum;
    
}

apl_size_t CIndexDict::MinTag(void) const
{
    if (this->moDict[0] == APL_NULL)
    {
        return this->NextTag(0);
    }
    else
    {
        return 0;
    }
}

void CIndexDict::Dump( apl_size_t auLevel ) const
{
    apl_errprintf("================ Dumping CIndexDict(%p) ================\n", this);

    for (apl_size_t luI = 0; luI < this->moDict.size(); luI++)
    {
        if (auLevel < 1)
        {
            continue;
        }

        if (this->moDict[luI] == APL_NULL)
        {
            continue;
        }

        apl_errprintf("ITEMS[%"APL_PRIuINT"]:%"APL_PRIuINT"\n", luI, this->moDict[luI]->size() );

        for (apl_size_t luK = 0; luK < this->moDict[luI]->size(); luK++)
        {
            if (auLevel < 2)
            {
                continue;
            }
            
            ValueType& loValue = this->moDict[luI]->operator [] (luK);
            apl_errprintf("(%"APL_PRIuINT",%"APL_PRIuINT"):%.*s\n", luI, luK, loValue.length(), loValue.c_str() );
        }
    }
}

void CIndexDict::CopyFrom( CIndexDict const& aoRhs )
{
    for (apl_size_t luI = 0; luI < aoRhs.moDict.size(); luI++)
    {
        ItemType* lpoItem = aoRhs.moDict[luI];
        if (lpoItem == APL_NULL)
        {
            continue;
        }

        for (apl_size_t luK = 0; luK < lpoItem->size(); luK++)
        {
            ValueType& loValue = lpoItem->operator [] (luK);

            this->Add(luI, loValue.c_str(), loValue.length() );
        }
    }
}

CIndexDict::ValueType& CIndexDict::Insert( apl_size_t auTag, apl_size_t auIndex )
{
    if (auTag >= this->moDict.size() )
    {
        this->moDict.resize(auTag + 1);
    }
    
    if (this->moDict[auTag] == APL_NULL)
    {
        this->moDict[auTag] = this->moItemAllocatee.Allocate();
    }

    if (auIndex >= this->moDict[auTag]->size() )
    {
        if (auIndex == 0)
        {
            return this->moDict[auTag]->fill_front();
        }
        else
        {
            this->moDict[auTag]->resize(auIndex + 1);
        }
    }
    
    ValueType& loValue = this->moDict[auTag]->operator [] (auIndex);

    return loValue;
}

CIndexDict::ValueType& CIndexDict::PushBack( apl_size_t auTag )
{
    if (auTag >= this->moDict.size() )
    {
        this->moDict.resize(auTag + 1);
    }
    
    if (this->moDict[auTag] == APL_NULL)
    {
        this->moDict[auTag] = this->moItemAllocatee.Allocate();
    }
    
    this->moDict[auTag]->push_back(ValueType() );

    return this->moDict[auTag]->back();
}

CIndexDict::ValueType* CIndexDict::GetValue( apl_size_t auTag, apl_size_t auIndex ) const
{
    if (auTag >= this->moDict.size() )
    {
        return APL_NULL;
    }

    if (this->moDict[auTag] == APL_NULL)
    {
        return APL_NULL;
    }

    if (auIndex >= this->moDict[auTag]->size() )
    {
        return APL_NULL;
    }

    return &this->moDict[auTag]->operator [] (auIndex);
}

ACL_NAMESPACE_END

