/** 
 * @file AICrypt.h
 */

#ifndef  __AILIBEX__AICRYPT_H__
#define  __AILIBEX__AICRYPT_H__

#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

enum
{
    AI_B64_ERR_INSIZE   = -1,
    AI_B64_ERR_OUTSIZE  = -2,
    AI_B64_ERR_PADDING  = -3,
    AI_B64_ERR_INPUT    = -4
};

////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief MD5 encode
 *
 * @param apcInData      IN - the original string
 * @param aiLength       IN - the encode buffer length
 * @param apcOutData     OUT - the ouput encode buffer
 *   
 * @return Return the lenght of encode buffer
 */ 
extern  size_t MD5Encode(void const* apcInData, size_t aiLength, void* apcOutData);

////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief BCD encode
 *
 * @param apcInData      IN - the original string
 * @param aiLength       IN - the encode buffer length
 * @param apcOutData     OUT - the ouput encode buffer
 *
 * @return Return the lenght of encode buffer
 */ 
extern  size_t BCDEncode(void const* apcInData, size_t aiLength, void* apcOutData);

/**
 * @brief BCD decode
 *
 * @param apcInData      IN - the original string
 * @param aiLength       IN - the encode buffer length
 * @param apcOutData     OUT - the output decode buffer
 *
 * @return Return the lenght of decode buffer
 */ 
extern  size_t BCDDecode(void const* apcInData, size_t aiLength, void* apcOutData);

////////////////////////////////////////////////////////////////////////////////////
#define BASE64_CALC_ENCODE_SIZE(l) (l / 3 * 4 + (l % 3 ? 4 : 0) + 1)
#define BASE64_CALC_DECODE_SIZE(l) (l / 4 * 3)

/**
 * @brief Base64 encode
 * 
 * @param apcInData      IN - the original string
 * @param aiInLength     IN - the length of original string
 * @param apcOutData     OUT - the output encode buffer
 * @param aiOutLength    IN - the output buffer size
 *
 * @return Return the lenght of encode buffer.
 */ 
extern  ssize_t BASE64Encode(void const* apcInData, size_t aiInLength, void* apcOutData, size_t aiOutLength);

/**
 * @brief Base64 decode
 *
 * @param apcInData      IN - the original base64ed string
 * @param aiInLength     IN - the length of original base64ed string
 * @param apcOutData     OUT - the output decode buffer
 * @param aiOutLength    IN - the output buffer size
 *
 * @return Retrun the decode buffer length.
 */ 
extern  ssize_t BASE64Decode(void const* apcInData, size_t aiInLength, void* apcOutData, size_t aiOutLength);

/**
 * @brief Test whether the input data is base64ed or not.
 *
 * @param apData         IN - the input data
 * @param aiLength       IN - the data size
 *
 * @retrun >0 is base64ed,otherwise,is not
 */ 
extern  ssize_t BASE64Check(void const* apData, size_t aiLength);

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AICRYPT_H__

