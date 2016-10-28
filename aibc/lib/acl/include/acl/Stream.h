/**
 * @file Stream.h
 */

#ifndef ACL_STREAM_H
#define ACL_STREAM_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

///////////////////////////////////////// InputStream /////////////////////////////////////////
/**
 * class CInputStream
 */
class CInputStream
{
public:
    /**
     * @brief A constructor.
     *
     * @param [in] atStream    the specified stream, include file stream,socket stream,mem string
     */
    template<typename TStreamType>
    CInputStream( TStreamType& atStream );
    
    /**
     * @brief A destructor.
     */
    ~CInputStream(void);
    
    /**
     * @brief Read from stream.
     *
     * @param [out] atVal    store value from the stream.
     *
     * @retval >0    Successful, the number of bytes actually read shall be returned.
     * @retval -1    Fail.
     */
    template<typename TValueType>
    apl_ssize_t Read( TValueType& atVal );
    
    /**
     * @brief Read from stream.
     *
     * @param [out] apvBuffer    the buffer to store data from the stream
     * @param [in]  aiSize       the bytes to read
     *
     * @retval >0    Successful, the number of bytes actually read shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t Read( void* apvBuffer, apl_size_t aiSize );
    
    /**
     * @brief read data from stream until the first occurrence of \e apcTag.
     *
     * @param [out] apvBuffer    the buffer to store data from the stream
     * @param [in]  aiSize       the bytes to read
     * @param [in]  apcTag       the tag string
     *
     * @retval >0    Successful, the number of bytes actually read shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t ReadUntil( void* apvBuffer, apl_size_t aiSize, const char* apcTag );
    
    /**
     * @brief Read from stream and convert uint16_t quantities from network byte order to host byte order.
     *
     * @param [out] au16Val    the uint16_t quantities
     *
     * @retval >0    Successful, \e sizeof(au16Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_uint16_t& au16Val );
    
    /**
     * @brief Read from stream and convert uint32_t quantities from network byte order to host byte order.
     *
     * @param [out] au32Val    the uint32_t quantities
     *
     * @retval >0    Successful, \e sizeof(au32Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_uint32_t& au32Val );
    
    /**
     * @brief Read from stream and convert uint64_t quantities from network byte order to host byte order.
     *
     * @param [out] au64Val    the uint64_t quantities
     *
     * @retval >0    Successful, \e sizeof(au64Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_uint64_t& au64Val );
    
    /**
     * @brief Read from stream and convert int16_t quantities from network byte order to host byte order.
     *
     * @param [in] ai16Val    the int16_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai16Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_int16_t& ai16Val );
    
    /**
     * @brief Read from stream and convert int32_t quantities from network byte order to host byte order.
     *
     * @param [out] ai32Val    the int32_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai32Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_int32_t& ai32Val );
    
    /**
     * @brief Read from stream and convert int64_t quantities from network byte order to host byte order.
     *
     * @param [out] ai64Val    the int64_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai64Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_int64_t& ai64Val );

protected:
    apl_ssize_t ReadUntilKMP( void* apvBuffer, apl_size_t aiSize, const char* apcTag, apl_size_t aiTagLen );
    void GetNext( char const* apcTag, apl_int_t* apiVal, apl_size_t aiN );
    
protected:
    class CPlaceHolder
    {
    public:
        virtual ~CPlaceHolder()
        {
        };
        
        virtual apl_ssize_t Read( void* apcBuffer, apl_size_t aiSize ) = 0;
    };
    
    template<typename TStreamType>
    class CHolder : public CPlaceHolder
    {
    public:
        CHolder( TStreamType& aoValue )
            : mtHeld(aoValue)
        {
        }
        virtual apl_ssize_t Read( void* apcBuffer, apl_size_t aiSize )
        {
            return mtHeld.Read( apcBuffer, aiSize );
        }
        
    private:
        TStreamType& mtHeld;
    };
    
    CPlaceHolder* mpoStream;
};

template<typename TStreamType>
CInputStream::CInputStream( TStreamType& atStream )
    : mpoStream(NULL)
{
    ACL_NEW_ASSERT(this->mpoStream, CHolder<TStreamType>(atStream) );
}

template<typename TValueType>
apl_ssize_t CInputStream::Read( TValueType& atVal )
{
    return this->Read( &atVal, sizeof(atVal) );
}

///////////////////////////////////////// InputStream /////////////////////////////////////////
/**
 * class COutputStream
 */
class COutputStream
{
public:
    /**
     * @brief A constructor.
     *
     * @param [in] atStream    the specified stream, include file stream,socket stream,mem string
     */
    template<typename TStreamType>
    COutputStream( TStreamType& atStream );
    
    /**
     * @brief A destructor.
     */
    ~COutputStream(void);
    
    /**
     * @brief Write to the stream.
     *
     * @param [in] atVal    the value to write to the stream.
     *
     * @retval >0    Successful, the number of bytes actually write shall be returned.
     * @retval -1    Fail.
     */
    template<typename TValueType>
    apl_ssize_t Write( TValueType const& atVal );
    
    /**
     * @brief Write to the stream.
     *
     * @param [in] apvBuffer   the buffer to write
     * @param [in] aiSize      the bytes to write
     *
     * @retval >0    Successful, the number of bytes actually write shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t Write( void const* apvBuffer, apl_size_t aiSize );
    
    /**
     * @brief Write to the stream.
     *
     * @param [in] apcPtr    the string to write to the stream.
     *
     * @retval >0    Successful, the number of bytes actually write shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t Write( char const* apcPtr );
    
    /**
     * @brief Write to the stream and convert uint16_t quantities from host byte order to network byte order. 
     *
     * @param [in] au16Val    the uint16_t quantities
     *
     * @retval >0    Successful, \e sizeof(au16Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_uint16_t au16Val );
    
    /**
     * @brief Write to the stream and convert uint32_t quantities from host byte order to network byte order. 
     *
     * @param [in] au32Val    the uint32_t quantities
     *
     * @retval >0    Successful, \e sizeof(au32Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_uint32_t au32Val );
    
    /**
     * @brief Write to the stream and convert uint64_t quantities from host byte order to network byte order. 
     *
     * @param [in] au64Val    the uint64_t quantities
     *
     * @retval >0    Successful, \e sizeof(au64Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_uint64_t au64Val );
    
    /**
     * @brief Write to the stream and convert int16_t quantities from host byte order to network byte order. 
     *
     * @param [in] ai16Val    the int16_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai16Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_int16_t ai16Val );
    
    /**
     * @brief Write to the stream and convert int32_t quantities from host byte order to network byte order. 
     *
     * @param [in] ai32Val    the int32_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai32Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_int32_t ai32Val );
    
    /**
     * @brief Write to the stream and convert int64_t quantities from host byte order to network byte order. 
     *
     * @param [in] ai64Val    the int64_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai64Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_int64_t ai64Val );
    
protected:
    class CPlaceHolder
    {
    public:
        virtual ~CPlaceHolder()
        {
        }
        
        virtual apl_ssize_t Write( void const* apcBuffer, apl_size_t aiSize ) = 0;
    };
    
    template<typename TStreamType>
    class CHolder : public CPlaceHolder
    {
    public:
        CHolder( TStreamType& aoValue )
            : mtHeld(aoValue)
        {
        }
        virtual apl_ssize_t Write( void const* apcBuffer, apl_size_t aiSize )
        {
            return mtHeld.Write( apcBuffer, aiSize );
        }
        
    private:
        TStreamType& mtHeld;
    };
    
    CPlaceHolder* mpoStream;
};

template<typename TStreamType>
COutputStream::COutputStream( TStreamType& atStream )
    : mpoStream(NULL)
{
    ACL_NEW_ASSERT(this->mpoStream, CHolder<TStreamType>(atStream) );
}

template<typename TValueType>
apl_ssize_t COutputStream::Write( TValueType const& atVal )
{
    return this->Write( &atVal, sizeof(atVal) );
}

ACL_NAMESPACE_END

#endif//ACL_STREAM_H
