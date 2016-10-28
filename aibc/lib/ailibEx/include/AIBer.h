/** @file AIBer.h
*/  
 
#ifndef __AILIBEX__AIBER_H__
#define __AILIBEX__AIBER_H__

#include <stdio.h>
//#include <stdint.h> 
#include <string.h>
#include <stdlib.h>
#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

#define AI_BER_LONG_LEN			0x80

#define AI_BER_EXTENSION_ID    	        0x1F
#define AI_IS_EXTENSION_ID(byte)	(((byte) & AI_BER_EXTENSION_ID) == AI_BER_EXTENSION_ID)

#define AI_BER_INTEGER                  0x02
#define AI_BER_BIT_STR                  0x03
#define AI_BER_OCTET_STR                0x04
#define AI_BER_NULL                     0x05
#define AI_BER_OBJECT_ID                0x06
#define AI_BER_SEQUENCE                 0x30
#define AI_BER_IPADDRESS                0x40
#define AI_BER_COUNTER                  0x41
#define AI_BER_GAUGE                    0x42
#define AI_BER_TIMETICKS                0x43
#define AI_BER_OPAQUE                   0x44

/**
* @brief Build an BER header for a length with length specified.
*
* @param apcBuffer          IN - pointer to start of object
* @param apiMaxSize         IN/OUT - number of valid bytes left in buffer
* @param aiValueLen	    IN - length of object
*
* @return Return a pointer to the first byte of the contents of this object.
*         Returns NULL on any error.
*/
unsigned char *BerBuildLength(unsigned char * apcBuffer, size_t * apiMaxSize,size_t aiValueLen);

/**
* @brief Build an BER header for a sequence with the ID and length specified.
* 
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            IN - ber type of object
* @param aiValueLen        IN - length of object
*
* @return Returns a pointer to the first byte of the contents of this object.
*         Returns NULL on any error.
*/
unsigned char * BerBuildSequence(unsigned char * apcBuffer,size_t * apiMaxSize, unsigned char acType, size_t aiValueLen);

/**
* @brief builds an BER header for an object with the ID and length specified.
* 
* @param apcBuffer        IN - pointer to start of object
* @param apiMaxSize       IN/OUT - number of valid bytes left in buffer
* @param acType           IN - ber type of object
* @param aiValueLen       IN - length of object
*
* @return Returns a pointer to the first byte of the contents of this object.
          Returns NULL on any error.
*/
unsigned char *BerBuildHeader(unsigned char * apcBuffer,size_t * apiMaxSize, unsigned char acType, size_t aiValueLen);

/**
* @brief  Build an BER object containing an integer.
*
* @param apcBuffer        IN - pointer to start of output buffer
* @param apiMaxSize       IN/OUT - number of valid bytes left in buffer
* @param acType           IN  - ber type of objec
* @param apiValue         IN - pointer to start of long integer
* @param aiValueLen       IN - size of input buffer
* 
* @return  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
*          Returns NULL on any error.
*/
unsigned char *BerBuildInt(unsigned char * apcBuffer, size_t * apiMaxSize, const unsigned char acType , const long *apiValue, size_t aiValueLen);

/**
* @brief Build an BER octet string object containing the input string.
*
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            IN - ber type of object
* @param apcValue          IN - pointer to start of input buffer
* @param aiValueLen        IN - size of input buffer
*
* @return  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
           Returns NULL on any error.
*/
unsigned char * BerBuildString(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char acType, const char * apcValue, size_t aiValueLen);

/**
* @brief Build an BER object identifier object containing the input string.
*
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            IN - ber type of object
* @param apiValue          IN - pointer to start of input buffer
* @param aiValueLen        IN - number of sub-id's in objid
*
* @return   Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
            Returns NULL on any error.
*/
unsigned  char *BerBuildObjid(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char acType, unsigned long *apiValue, size_t aiValueLen) ;

/**
* @brief Build an BER object containing an integer.
*
* @param apcBuffer         IN - pointer to start of output buffer
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            IN  - ber type of objec
* @param apiValue          IN - pointer to start of long integer
* @param aiValueLen        IN - size of input buffer
* 
* @return  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
           Returns NULL on any error.
*/
unsigned char *BerBuildUnsignedInt(unsigned char  * apcBuffer , size_t  * apiMaxSize,unsigned char acType, const unsigned long  * apiValue , int aiValueLen);

/**
* @brief Build an BER null object.
*
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            IN - ber type of object
*
* @retun  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
*         Returns NULL on any error.
*/
unsigned char *BerBuildNull(unsigned char * apcBuffer ,size_t * apiMaxSize, unsigned char acType);

/**
* @brief Builds an BER bit string object containing the input string.
*
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            IN - ber type of object
* @param apcValue          IN - pointer to start of input buffer
* @param aiValueLen        IN - size of input buffer
*
* @return Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
          Returns NULL on any error.
*/
unsigned char *BerBuildBitstring(unsigned char *apcBuffer,size_t * apiMaxSize,unsigned char acType, const  char * apcValue, size_t aiValueLen);

/**
* @brief Interpret the length of the current object.
*
* @param apcBuffer         IN - pointer to start of length field
* @param aiValueLen        OUT - value of length field
*
*  @return Returns a pointer to the first byte after this length field.
           Returns NULL on any error.
*/
unsigned char *BerParseLength(unsigned char * apcBuffer, unsigned long * aiValueLen);

/**
* @brief  Interpret the ID and length of the current object.
*
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param apcType           OUT - ber type of object
*
* @return  Returns a pointer to the first byte of the contents of this object.
           Returns NULL on any error.
*/
unsigned char *BerParseHeader(unsigned char * apcBuffer, size_t * apiMaxSize, unsigned char * apcType);

/**
* @brief Interpret the ID and length of the current object.
*
* @param apcBuffer          IN - pointer to start of object
* @param apiMaxSize         IN/OUT - number of valid bytes left in buffer
* @param apcType            OUT - ber type of object
* @param acExpectedType     IN - expected type
*
* @return  Returns a pointer to the first byte of the contents of this object.
           Returns NULL on any error.
*/
unsigned char *BerParseSequence(unsigned char * apcBuffer, size_t * apiMaxSize, unsigned char * apcType, unsigned char acExpectedType);

/**
* @brief Pull an object indentifier out of an BER object identifier type.
*
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param apcType           OUT - ber type of object
* @param apiObjid          IN/OUT - pointer to start of output buffer
* @param apiObjidLen       IN/OUT - number of sub-id's in objid
*
*  @return Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
           Returns NULL on any error.
*/
unsigned char *BerParseObjid(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char * apcType, unsigned long * apiObjid, int *apiObjidLen);

/**
* @brief Pull a long out of an int type.
*
* @param apcBuffer       IN - pointer to start of object
* @param apiMaxSize      IN/OUT - number of valid bytes left in buffer
* @param apcType         OUT - ber type of object
* @param apiValue        IN/OUT - pointer to start of output buffer
* @param aiValueLen      IN - size of output buffer
* 
* @return Returns pointer to the first byte past the end of this object (i.e. the start of the next object).
          Returns NULL on any error.
*/
unsigned char *BerParseInt(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char * apcType, long *apiValue, int aiValueLen);

/**
* @brief Pull an octet string out of an BER octet string type.
*
* @param apcBUffer        IN - pointer to start of object
* @param apiMaxSize       IN/OUT - number of valid bytes left in buffer
* @param acType           OUT - ber type of object 
* @param apcValue         IN/OUT - pointer to start of output buffer
* @param aiValueLen       IN/OUT - size of output buffer
* 
* @return  Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
           Returns NULL on any error.
*/
unsigned char *BerParseString (unsigned char *apcBuffer, size_t *apiMaxSize, unsigned char *apcType, char *apcValue, int aiValueLen);

/**
* @brief Interpret an BER null type.
*
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            OUT - ber type of object
*
*  @return Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
           Returns NULL on any error.
*/
unsigned char *BerParseNull(unsigned char * apcBuffer, size_t * apiMaxSize, unsigned char * apcType);

/**
* @brief Pull a bitstring out of an BER bitstring type.
*
* @param apcBuffer         IN - pointer to start of object
* @param apiMaxSize        IN/OUT - number of valid bytes left in buffer
* @param acType            OUT - ber type of object
* @param apcValue          IN/OUT - pointer to start of output buffer
* @param apiValueLen       IN/OUT - size of output buffer
*
* @return Returns a pointer to the first byte past the end of this object (i.e. the start of the next object).
          Returns NULL on any error.
*/
unsigned char *BerParseBitstring(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char * apcType, char * apcValue, int * apiValueLen);

/**
* @brief Pull an unsigned long out of an BER int type.
*
* @param apcBuffer       IN - pointer to start of object
* @param apiMaxSize      IN/OUT - number of valid bytes left in buffer
* @param acType          OUT - ber type of object
* @param apiValue        IN/OUT - pointer to start of output buffer
* @param aiValueLen      IN - size of output buffer
* 
* @return Returns pointer to the first byte past the end of this object (i.e. the start of the next object).
          Returns NULL on any error
*/
unsigned char *BerParseUnsignedInt(unsigned char * apcBuffer,size_t * apiMaxSize,unsigned char * apcType, unsigned long * apiValue, int aiValueLen);

unsigned char *BerParseAgentAddr (unsigned char *apcBuffer, size_t *apiMaxSize, unsigned char *apcType, char *apcValue, int aiValueLen);

///end namespace
AIBC_NAMESPACE_END

#endif    //__AILIBEX__AIBER_H__

