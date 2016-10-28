
#ifndef __AI_FILE_CACHE_CONFIG_H__
#define __AI_FILE_CACHE_CONFIG_H__

#include <stdlib.h>

#define AI_CACHE_NAMESPACE AICache
#define AI_CACHE_NAMESPACE_START namespace AI_CACHE_NAMESPACE {
#define AI_CACHE_NAMESPACE_END };

// start namespace
//AI_BILL_CACHE_NAMESPACE
namespace AI_CACHE_NAMESPACE
{

// const variable define
// ip addr length
const size_t AI_IP_ADDR_LEN       = 20;

//max message key length
const size_t AI_MAX_KEY_LEN     = 256;

//max data buffer length
const size_t AI_MAX_VALUE_LEN      = 2048;

//max file name length
const size_t AI_MAX_FILENAME_LEN  = 128;

const size_t AI_MAX_NAME_LEN      = 128;

//hash size
const size_t AI_HASH_SIZE         = 25165843;

const size_t AI_SERVER_HASH_SIZE  = 100;

// single file max size
const size_t AI_MAX_S_FILE_SIZE   = 0x7FFFFFFF; //2GB
// end variable define

const size_t AI_MAX_VERSION_LEN   = 40;

//#define AI_MAX_FILENAME_LEN AI_MAX_FILENAME_LEN

};

#endif //__AI_FILE_CACHE_CONFIG_H__
