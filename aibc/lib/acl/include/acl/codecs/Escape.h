
#ifndef ACL_CODECS_ESCAPE_H
#define ACL_CODECS_ESCAPE_H

#include "acl/Utility.h"
#include "acl/MemoryBlock.h"
#include "acl/HashArray.h"

ACL_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Escape
///Escape charset
template<typename NameType, typename HashFuncType>
class TEscapeCharset
{
public:
    TEscapeCharset(void) : moCharacters(512)
    {
    }
    
    void SetCharacter( unsigned char ac, char const* apcName )
    {
        this->moNames[ac].mpcName = apcName;
        this->moNames[ac].muLength = apl_strlen(apcName);
        this->moCharacters[apcName] = ac;
    }

    apl_int_t FindName( unsigned char ac, char const*& apcName, apl_size_t& auLength ) const
    {
        const CNameNode& loNode = this->moNames[ac];

        if (loNode.mpcName != NULL)
        {
            auLength = loNode.muLength;
            apcName = loNode.mpcName;
            return 0;
        }
        else
        {
            return -1;
        }
    }

    apl_int_t FindCharacter( char const* apoName, apl_size_t auLength, unsigned char& ac ) const
    {
        NameType loName(apoName, auLength);

        typename MapType::const_iterator loIter = this->moCharacters.find(loName);
    
        if (loIter != this->moCharacters.end() )
        {
            ac = loIter->second;
            return 0;
        }
        else
        {
            return -1;
        }
    }

private:
    struct CNameNode
    {
        CNameNode(void) : mpcName(NULL) , muLength(0)
        {
        }

        char const* mpcName;
        apl_size_t  muLength;
    };
    
    CNameNode moNames[256];

    typedef THashArray<NameType, unsigned char, HashFuncType> MapType;
    MapType moCharacters;
};

///encoder
template<typename CharsetType>
class TEscapeEncoder
{
public:
    static CharsetType coCharset;
    
public:
    /**
     * Default constuctor
     */
    TEscapeEncoder(void);

    /**
     * Constuctor
     * Accept user defined buffer, if buffer is no enough and Encoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     */
    TEscapeEncoder( char* apcBuffer, apl_size_t auSize );

    /**
     * Constuctor
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Encoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     */
    TEscapeEncoder( CMemoryBlock const& aoMB );

    /**
    * Escape characters block update operation. Continues an Escape characters encode
    * operation, processing another message block, and updating the context.
    *
    * @param [in] apcInput Escaped characters data in string.
    *
    * @return Upon successful completion, shall return 0; otherwise, it shall return -1
    */
    apl_int_t Update( char const* apcInput );
    
    /**
    * Escape characters block update operation. Continues an Escape characters encode
    * operation, processing another message block, and updating the context.
    *
    * @param [in] apcInput Escaped characters data in string.
    * @param [in] auLength Length of data in string.
    *
    * @return Upon successful completion, shall return 0; otherwise, it shall return -1
    */
    apl_int_t Update( char const* apcInput, apl_size_t auLength );

    /**
    * Escape characters finalization. Padding the end character '=' and zeroizing the context.
    *
    * @return Upon successful completion, shall return 0; otherwise, it shall return -1
    */
    apl_int_t Final(void);
    
    /**
    * Escape characters Update and Final operation.
    *
    * @param [in] apcInput Escaped characters data in string.
    *
    * @return Upon successful completion, shall return 0; otherwise, it shall return -1
    */
    apl_int_t Final( char const* apcInput );

    /**
    * Escape characters Update and Final operation.
    *
    * @param [in] apcInput Escaped characters data in string.
    * @param [in] auLength Length of data in string.
    *
    * @return Upon successful completion, shall return 0; otherwise, it shall return -1
    */
    apl_int_t Final( char const* apcInput, apl_size_t auLength );

    /**
    * @return Escape characters output context.
    */
    char const* GetOutput(void);

    /**
    * @return Escape characters context length.
    */
    apl_size_t GetLength(void);

    /**
    * Reset, zeroizing the context.
    */
    void Reset(void);

protected:
    struct CEndPred1
    {
        inline bool operator () ( char const* apcInput ) const
        {
            return *apcInput == '\0';
        }
    };
    
    struct CEndPred2
    {
        CEndPred2( char const* apcLast ) : mpcLast(apcLast)
        {
        }
        
        inline bool operator () ( char const* apcInput ) const
        {
            return apcInput == this->mpcLast;
        }
        
        char const* mpcLast;
    };
    
    template<typename EndPredType>
    apl_int_t Update( char const* apcInput, EndPredType const& aoEndPred );
    
private:
    acl::CMemoryBlock moBuffer;

    apl_size_t muLength;
};

////implement
template<typename CharsetType> CharsetType TEscapeEncoder<CharsetType>::coCharset;
    
template<typename CharsetType>
TEscapeEncoder<CharsetType>::TEscapeEncoder(void)
    : moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
{
}

template<typename CharsetType>
TEscapeEncoder<CharsetType>::TEscapeEncoder( char* apcBuffer, apl_size_t auSize )
    : moBuffer(apcBuffer, auSize, acl::CMemoryBlock::DONT_DELETE)
{
}

template<typename CharsetType>
TEscapeEncoder<CharsetType>::TEscapeEncoder( CMemoryBlock const& aoMB )
    : moBuffer(aoMB.GetDataBlock()->Duplicate() )
{
}

template<typename CharsetType>
apl_int_t TEscapeEncoder<CharsetType>::Update( char const* apcInput )
{
    return this->Update(apcInput, CEndPred1() );
}

template<typename CharsetType>
apl_int_t TEscapeEncoder<CharsetType>::Update( char const* apcInput, apl_size_t auLength )
{
    return this->Update(apcInput, CEndPred2(apcInput + auLength) );
}

template<typename CharsetType>
template<typename EndPredType>
apl_int_t TEscapeEncoder<CharsetType>::Update( char const* apcInput, EndPredType const& aoEndPred )
{
    apl_int_t   liRetCode = 0;
    char        lacBuffer[1024];
    apl_size_t  luIndex = 0;
    apl_size_t  luLength = 0;
    char const* lpcName = NULL;

    while( !aoEndPred(apcInput) )
    {
        if ( (liRetCode = coCharset.FindName(*apcInput, lpcName, luLength) ) != 0)
        {
            luLength = 1;
        }
        
        if (sizeof(lacBuffer) - luIndex < luLength)
        {
            if (this->moBuffer.GetSpace() < luIndex)
            {
                this->moBuffer.Resize(this->moBuffer.GetLength() + sizeof(lacBuffer) * 2 + 1);
            }

            apl_memcpy(this->moBuffer.GetWritePtr(), lacBuffer, luIndex);
            this->moBuffer.SetWritePtr(luIndex);
            
            luIndex = 0;

            ACL_ASSERT(sizeof(lacBuffer) > luLength);
        }
        
        if (liRetCode == 0)
        {
            apl_memcpy(&lacBuffer[luIndex], lpcName, luLength);
            luIndex += luLength;
        }
        else
        {
            lacBuffer[luIndex++] = *apcInput;
        }

        apcInput++;
    }

    if (luIndex > 0)
    {
        if (this->moBuffer.GetSpace() < luIndex)
        {
            this->moBuffer.Resize(this->moBuffer.GetLength() + luIndex + 1);
        }
        
        apl_memcpy(this->moBuffer.GetWritePtr(), lacBuffer, luIndex);
        this->moBuffer.SetWritePtr(luIndex);
    }

    return 0;
}

template<typename CharsetType>
apl_int_t TEscapeEncoder<CharsetType>::Final(void)
{
    this->muLength = this->moBuffer.GetLength();

    *(this->moBuffer.GetWritePtr() ) = '\0';

    this->Reset();

    return 0;
}

template<typename CharsetType>
apl_int_t TEscapeEncoder<CharsetType>::Final( char const* apcInput )
{
    if (this->Update(apcInput) != 0)
    {
        return -1;
    }

    return this->Final();
}

template<typename CharsetType>
apl_int_t TEscapeEncoder<CharsetType>::Final( char const* apcInput, apl_size_t auLength )
{
    if (this->Update(apcInput, auLength) != 0)
    {
        return -1;
    }

    return this->Final();
}

template<typename CharsetType>
char const* TEscapeEncoder<CharsetType>::GetOutput(void)
{
    return this->moBuffer.GetBase();
}

template<typename CharsetType>
apl_size_t TEscapeEncoder<CharsetType>::GetLength(void)
{
    return this->muLength;
}

template<typename CharsetType>
void TEscapeEncoder<CharsetType>::Reset(void)
{
    this->moBuffer.Reset();
}

//////////////////////////////////////////////////////////////////////////////////////////
//decoder
template<typename CharsetType>
class TEscapeDecoder
{
public:
    static CharsetType coCharset;
    
public:
    /**
     * Default constuctor
     */
    TEscapeDecoder(void);
    
    /**
     * Constuctor
     * Accept user defined buffer, if buffer is no enough and Decoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     */
    TEscapeDecoder( char* apcBuffer, apl_size_t auSize );

    /**
     * Constuctor
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Decoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     */
    TEscapeDecoder( CMemoryBlock const& aoMB );

    /**
     * Escape characters block update operation. Continues an Escape characters encode
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apcInput Escaped characters data in string.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( char const* apcInput );
    
    /**
     * Escape characters block update operation. Continues an Escape characters encode
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apcInput Escaped characters data in string.
     * @param [in] auLength Length of data in string.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( char const* apcInput, apl_size_t auLength );

    /**
     * Escape characters finalization. Padding the end character '=' and zeroizing the context.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final(void);

    /**
     * Escape characters Update and Final operation.
     *
     * @param [in] apcInput Escape characters data in string.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( char const* apcInput );

    /**
     * Escape characters Update and Final operation.
     *
     * @param [in] apvInput Escape characters data in string.
     * @param [in] auInputLen Length of data in string.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( char const* apcInput, apl_size_t auLength );

    /**
     * @return Escape characters output context.
     */
    char const* GetOutput(void);

    /**
     * @return Escape characters context length.
     */
    apl_size_t GetLength(void);

    /**
     * Reset, zeroizing the context.
     */
    void Reset(void);

private:
    acl::CMemoryBlock moBuffer;

    apl_size_t muLength;
};

///implement
template<typename CharsetType> CharsetType TEscapeDecoder<CharsetType>::coCharset;

template<typename CharsetType>
TEscapeDecoder<CharsetType>::TEscapeDecoder(void)
    : moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
{
}

template<typename CharsetType>
TEscapeDecoder<CharsetType>::TEscapeDecoder( char* apcBuffer, apl_size_t auSize )
    : moBuffer(apcBuffer, auSize, acl::CMemoryBlock::DONT_DELETE)
{
}

template<typename CharsetType>
TEscapeDecoder<CharsetType>::TEscapeDecoder( CMemoryBlock const& aoMB )
    : moBuffer(aoMB.GetDataBlock()->Duplicate() )
{
}

template<typename CharsetType>
apl_int_t TEscapeDecoder<CharsetType>::Update( char const* apcInput )
{
    return this->Update(apcInput, apl_strlen(apcInput) );
}

template<typename CharsetType>
apl_int_t TEscapeDecoder<CharsetType>::Update( char const* apcInput, apl_size_t auLength )
{
    char        lacBuffer[1024];
    apl_size_t  luIndex = 0;
    char const* lpcFirst = apcInput;
    char const* lpcLast = lpcFirst + auLength;
    char const* lpcName = NULL;
    char const* lpcNameLast = NULL;

    while(true)
    {
        lpcName = apl_strchr(lpcFirst, '&');
        if (lpcName == NULL)
        {
            break;
        }
        
        lpcNameLast = apl_strchr(lpcName, ';');
        if (lpcNameLast == NULL)
        {
            break;
        }

        if (luIndex + (lpcNameLast - lpcFirst + 1) > sizeof(lacBuffer) )
        {
            if (this->moBuffer.GetSpace() < luIndex)
            {
                this->moBuffer.Resize(this->moBuffer.GetLength() + sizeof(lacBuffer) * 2 + 1);
            }
            
            apl_memcpy(this->moBuffer.GetWritePtr(), lacBuffer, luIndex);
            this->moBuffer.SetWritePtr(luIndex);

            luIndex = 0;

            ACL_ASSERT(sizeof(lacBuffer) > (apl_size_t)(lpcNameLast - lpcFirst + 1) );
        }
        
        unsigned char lcCh;
        if ( coCharset.FindCharacter(lpcName, lpcNameLast - lpcName + 1, lcCh) == 0)
        {
            apl_memcpy(&lacBuffer[luIndex], lpcFirst, lpcName - lpcFirst);
            luIndex += (lpcName - lpcFirst);
            lacBuffer[luIndex++] = lcCh;
        }
        else
        {
            apl_memcpy(&lacBuffer[luIndex], lpcFirst, lpcNameLast - lpcFirst + 1);
            luIndex += (lpcNameLast - lpcFirst + 1);
        }
        
        lpcFirst = lpcNameLast + 1;
    }

    if (this->moBuffer.GetSpace() < luIndex + (lpcLast - lpcFirst) + 1)
    {
        this->moBuffer.Resize(this->moBuffer.GetLength() + luIndex + (lpcLast - lpcFirst) + 1);
    }

    if (luIndex > 0)
    {
        apl_memcpy(this->moBuffer.GetWritePtr(), lacBuffer, luIndex);
        this->moBuffer.SetWritePtr(luIndex);
    }
    
    if (lpcLast - lpcFirst > 0)
    {
        apl_memcpy(this->moBuffer.GetWritePtr(), lpcFirst, lpcLast - lpcFirst);
        this->moBuffer.SetWritePtr(lpcLast - lpcFirst);
    }

    return 0;
}

template<typename CharsetType>
apl_int_t TEscapeDecoder<CharsetType>::Final(void)
{
    this->muLength = this->moBuffer.GetLength();

    *(this->moBuffer.GetWritePtr() ) = '\0';

    this->Reset();

    return 0;
}

template<typename CharsetType>
apl_int_t TEscapeDecoder<CharsetType>::Final( char const* apcInput )
{
    if (this->Update(apcInput) != 0)
    {
        return -1;
    }

    return this->Final();
}

template<typename CharsetType>
apl_int_t TEscapeDecoder<CharsetType>::Final( char const* apcInput, apl_size_t auLength )
{
    if (this->Update(apcInput, auLength) != 0)
    {
        return -1;
    }

    return this->Final();
}

template<typename CharsetType>
char const* TEscapeDecoder<CharsetType>::GetOutput(void)
{
    return this->moBuffer.GetBase();
}

template<typename CharsetType>
apl_size_t TEscapeDecoder<CharsetType>::GetLength(void)
{
    return this->muLength;
}

template<typename CharsetType>
void TEscapeDecoder<CharsetType>::Reset(void)
{
    this->moBuffer.Reset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Xml reserved characters
//Entity name class
struct CEscapeEntityName
{
    CEscapeEntityName(void);
    
    CEscapeEntityName(char const* apcName);
    
    CEscapeEntityName(char const* apcName, apl_size_t auLength);
    
    CEscapeEntityName(CEscapeEntityName const& aoName);
    
    bool operator == (CEscapeEntityName const& aoName) const;

    const char* mpcName;
    apl_size_t  mpuLength;
};

struct CEscapeReservedEntityNameHash
{
    ///reserved-characters hash
    apl_size_t operator()( CEscapeEntityName const& aoName ) const;
};

//reserved-characters
class CEscapeReservedCharacters : public TEscapeCharset<CEscapeEntityName, CEscapeReservedEntityNameHash>
{
public:
    CEscapeReservedCharacters(void);
};

typedef TEscapeEncoder<CEscapeReservedCharacters> CXmlEscapeEncoder;
typedef TEscapeDecoder<CEscapeReservedCharacters> CXmlEscapeDecoder;

ACL_NAMESPACE_END

#endif //ACL_CODECS_ESCAPE_H
