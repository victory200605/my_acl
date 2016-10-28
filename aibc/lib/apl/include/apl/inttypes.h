/**
 * @file inttypes.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.18 $
 */
#ifndef APL_INTTYPES_H
#define APL_INTTYPES_H

#include "aibc_config.h"
#include "apl/sys.h"

#if defined(__cplusplus)
#   define  __STDC_LIMIT_MACROS
#   define  __STDC_FORMAT_MACROS
#   define  __STDC_CONSTANT_MACROS
#endif

#if defined(HAVE_STDINT_H)
#   include <stdint.h>
#endif

#include <inttypes.h>
#include <signal.h>

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/** 
 * designates a signed integer type with width 8 bits
 */
typedef int8_t          apl_int8_t;

/** 
 * designates a signed integer type with width 16 bits 
 */
typedef int16_t         apl_int16_t;

/** 
 * designates a signed integer type with width 32 bits 
 */
typedef int32_t         apl_int32_t;

/** 
 * designates a signed integer type with width 64 bits 
 */
typedef int64_t         apl_int64_t;

/** 
 * designates a signed integer type capable of representing any value of any 
 * signed integer type 
 * */
typedef intmax_t        apl_intmax_t;

/** 
 * designates signed integer type with the property that any valid pointer to 
 * void can be converted to this type 
 */
typedef intptr_t        apl_int_t;

/** 
 * designates a unsigned integer type with width 8 bits 
 */
typedef uint8_t         apl_uint8_t;

/** 
 * designates a unsigned integer type with width 16 bits 
 */
typedef uint16_t        apl_uint16_t;

/** 
 * designates a unsigned integer type with width 32 bits 
 */
typedef uint32_t        apl_uint32_t;

/** 
 * designates a unsigned integer type with width 64 bits 
 */
typedef uint64_t        apl_uint64_t;

/** 
 * designates a unsigned integer type capable of representing any value of 
 * any unsigned integer type.
 */
typedef uintmax_t       apl_uintmax_t;

/** 
 * designates a unsigned integer type with the property that any valid 
 * pointer to void can be converted to this type
 */
typedef uintptr_t       apl_uint_t;

/** 
 * volatile-qualified integer type of an object that can be accessed as an 
 * atomic entity
 */
typedef sig_atomic_t    apl_atom_t;

#define __EXPCON__(A,B) A##B

#if defined(INT8_C)
#   define APL_INT8_C(n)       INT8_C(n)
#else
#   define APL_INT8_C(n)        (n)
#endif

#if defined(INT16_C)
#   define APL_INT16_C(n)      INT16_C(n)
#else
#   define APL_INT16_C(n)      (n)
#endif

#if defined(INT32_C)
#   define APL_INT32_C(n)      INT32_C(n)
#else
#   define APL_INT32_C(n)      __EXPCON__(n,l)
#endif

#if defined(INT64_C)
#   define APL_INT64_C(n)      INT64_C(n)
#else
#   define APL_INT64_C(n)      __EXPCON__(n,ll)
#endif

#if defined(INTMAX_C)
#   define APL_INTMAX_C(n)     INTMAX_C(n)
#else
#   define APL_INTMAX_C(n)     __EXPCON__(n,ll)
#endif

#if defined(UINT8_C)
#   define APL_UINT8_C(n)      UINT8_C(n)
#else
#   define APL_UINT8_C(n)      __EXPCON__(n,u)
#endif

#if defined(UINT16_C)
#   define APL_UINT16_C(n)     UINT16_C(n)
#else
#   define APL_UINT16_C(n)     __EXPCON__(n,u)
#endif

#if defined(UINT32_C)
#   define APL_UINT32_C(n)     UINT32_C(n)
#else
#   define APL_UINT32_C(n)     __EXPCON__(n,ul)
#endif

#if defined(UINT64_C)
#   define APL_UINT64_C(n)     UINT64_C(n)
#else
#   define APL_UINT64_C(n)     __EXPCON__(n,ull)
#endif

#if defined(UINTMAX_C)
#   define APL_UINTMAX_C(n)    UINTMAX_C(n)
#else
#   define APL_UINTMAX_C(n)    __EXPCON__(n,ull)
#endif

/**
 * expand to an integer constant expression corresponding to the type 
 * apl_int8_t.
 *
 * @def APL_INT8_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_int8_t that equal to n.
 */

/**
 * expand to an integer constant expression corresponding to the type 
 * apl_int16_t.
 *
 * @def APL_INT16_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_int16_t that equal to n.
 */

/**
 * expand to an integer constant expression corresponding to the type 
 * apl_int32_t.
 *
 * @def APL_INT32_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_int32_t that equal to n.
 */

/**
 * expand to an integer constant expression corresponding to the type 
 * apl_int64_t.
 *
 * @def APL_INT64_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_int64_t that equal to n.
 */

/**
 * expand to an integer constant expression corresponding to the type 
 * apl_intmax_t.
 *
 * @def APL_INTMAX_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_intmax_t that equal to n.
 */

/**
 * expand to an integer constant expression corresponding to the type 
 * apl_int_t.
 *
 * @def APL_INT_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_int_t that equal to n.
 */

/**
 * expand to an unsigned integer constant expression corresponding to the 
 * type apl_uint8_t.
 *
 * @def APL_UINT8_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_uint8_t that equal to n.
 */

/**
 * expand to an unsigned integer constant expression corresponding to the 
 * type apl_uint16_t.
 *
 * @def APL_UINT16_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_uint16_t that equal to n.
 */

/**
 * expand to an unsigned integer constant expression corresponding to the type 
 * apl_uint32_t.
 *
 * @def APL_UINT32_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_uint32_t that equal to n.
 */

/**
 * expand to an unsigned integer constant expression corresponding to the type 
 * apl_uint64_t.
 *
 * @def APL_UINT64_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_uint64_t that equal to n.
 */

/**
 * expand to an unsigned integer constant expression corresponding to the type
 * apl_uintmax_t.
 *
 * @def APL_UINTMAX_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_uintmax_t that equal to n.
 */

/**
 * expand to an unsigned integer constant expression corresponding to the 
 * type apl_uint_t.
 *
 * @def APL_UINT_C
 * @param[in]    n    an interger that wanted to expand.
 * @return     an integer of type apl_uint_t that equal to n.
 */



#if defined(INT8_MIN)
#    define APL_INT8_MIN        (INT8_MIN)
#else
#    define APL_INT8_MIN        APL_INT8_C(0x80)
#endif

#if defined(INT16_MIN)
#    define APL_INT16_MIN       (INT16_MIN)
#else
#    define APL_INT16_MIN       APL_INT16_C(0x8000)
#endif

#if defined(INT32_MIN)
#    define APL_INT32_MIN       (INT32_MIN)
#else
#    define APL_INT32_MIN       APL_INT32_C(0x80000000)
#endif

#if defined(INT64_MIN)
#    define APL_INT64_MIN       (INT64_MIN)
#else
#    define APL_INT64_MIN       APL_INT64_C(0x8000000000000000)
#endif

#if defined(INTMAX_MIN)
#    define APL_INTMAX_MIN      (INTMAX_MIN)
#else
#    define APL_INTMAX_MIN      APL_INTMAX_C(0x8000000000000000)
#endif

#if defined(INTPTR_MIN)
#    define APL_INT_MIN         (INTPTR_MIN)
#else
#    define APL_INT_MIN         APL_INT32_C(0x80000000)
#endif

#if defined(SIG_ATOM_MIN)
#    define APL_ATOM_MIN        (SIG_ATOM_MIN)
#else
#    define APL_ATOM_MIN        APL_INT8_C(0)
#endif

/**
 * Minimum values of 8-width signed integer types.
 *
 * @def APL_INT8_MIN
 */

/**
 * Minimum values of 16-width signed integer types.
 *
 * @def APL_INT16_MIN
 */

/**
 * Minimum values of 32-width signed integer types.
 *
 * @def APL_INT32_MIN
 */

/**
 * Minimum values of 64-width signed integer types.
 *
 * @def APL_INT64_MIN
 */

/**
 * Minimum values of greatest-width signed integer types.
 *
 * @def APL_INTMAX_MIN
 */

/**
 * Minimum values of pointer-holding signed integer types.
 *
 * @def APL_INT_MIN
 */

/**
 * signed integer type that not greater than -127.
 *
 * @def APL_ATOM_MIN
 */


#if defined(INT8_MAX)
#    define APL_INT8_MAX        (INT8_MAX)
#else
#    define APL_INT8_MAX        APL_INT8_C(0x7f)
#endif

#if defined(INT16_MAX)
#    define APL_INT16_MAX       (INT16_MAX)
#else
#    define APL_INT16_MAX       APL_INT16_C(0x7fff)
#endif

#if defined(INT32_MAX)
#    define APL_INT32_MAX       (INT32_MAX)
#else
#    define APL_INT32_MAX       APL_INT32_C(0x7fffffff)
#endif

#if defined(INT64_MAX)
#    define APL_INT64_MAX       (INT64_MAX)
#else
#    define APL_INT64_MAX       APL_INT64_C(0x7fffffffffffffff)
#endif

#if defined(INTMAX_MAX)
#    define APL_INTMAX_MAX      (INTMAX_MAX)
#else
#    define APL_INTMAX_MAX      APL_INTMAX_C(0x7fffffffffffffff)
#endif

/**
 * Maximum values of 8-width signed integer types.
 *
 * @def APL_INT8_MAX
 */

/**
 * Maximum values of 16-width signed integer types.
 *
 * @def APL_INT16_MAX
 */

/**
 * Maximum values of 32-width signed integer types.
 *
 * @def APL_INT32_MAX
 */

/**
 * Maximum values of 64-width signed integer types.
 *
 * @def APL_INT64_MAX
 */

/**
 * Maximum values of greatest signed integer types.
 *
 * @def APL_INTMAX_MAX
 */

/**
 * Maximum values of pointer-holding  signed integer types.
 *
 * @def APL_INT_MAX
 */


#if defined(UINT8_MAX)
#    define APL_UINT8_MAX       (UINT8_MAX)
#else
#    define APL_UINT8_MAX       APL_UINT8_C(0xff)
#endif

#if defined(UINT16_MAX)
#    define APL_UINT16_MAX      (UINT16_MAX)
#else
#    define APL_UINT16_MAX      APL_UINT16_C(0xffff)
#endif

#if defined(UINT32_MAX)
#    define APL_UINT32_MAX      (UINT32_MAX)
#else
#    define APL_UINT32_MAX      APL_UINT32_C(0xffffffff)
#endif

#if defined(UINT64_MAX)
#    define APL_UINT64_MAX      (UINT64_MAX)
#else
#    define APL_UINT64_MAX      APL_UINT64_C(0xffffffffffffffff)
#endif

#if defined(UINTMAX_MAX)
#    define APL_UINTMAX_MAX     (UINTMAX_MAX)
#else
#    define APL_UINTMAX_MAX     APL_UINTMAX_C(0xffffffffffffffff)
#endif

#if defined(APL_ATOM_MAX)
#    define APL_ATOM_MAX        (SIG_ATOM_MAX)
#else
#    define APL_ATOM_MAX        APL_UINT8_C(0xff)
#endif

#if (SIZEOF_INTPTR_T == SIZEOF_INT32_T)
#   define APL_INT_C(n)     APL_INT32_C(n)
#   define APL_INT_MAX      APL_INT32_C(0x7FFFFFFF)
#elif (SIZEOF_INTPTR_T == SIZEOF_INT64_T)
#   define APL_INT_C(n)     APL_INT64_C(n)
#   define APL_INT_MAX      APL_INT64_C(0x7FFFFFFFFFFFFFFF)
#else
#   error unknown system
#endif

#if (SIZEOF_UINTPTR_T == SIZEOF_UINT32_T)
#   define APL_UINT_C(n)    APL_UINT32_C(n)
#   define APL_UINT_MAX     APL_UINT32_C(0xFFFFFFFF)
#elif (SIZEOF_UINTPTR_T == SIZEOF_UINT64_T)
#   define APL_UINT_C(n)    APL_UINT64_C(n)
#   define APL_UINT_MAX     APL_UINT64_C(0xFFFFFFFFFFFFFFFF)
#else
#   error unknown system
#endif

/**
 * Maximum values of 8-width unsigned integer types.
 *
 * @def APL_UINT8_MAX
 */

/**
 * Maximum values of 16-width unsigned integer types.
 *
 * @def APL_UINT16_MAX
 */

/**
 * Maximum values of 32-width unsigned integer types.
 *
 * @def APL_UINT32_MAX
 */

/**
 * Maximum values of 64-width unsigned integer types.
 *
 * @def APL_UINT64_MAX
 */

/**
 * Maximum values of greatest unsigned integer types.
 *
 * @def APL_UINTMAX_MAX
 */

/**
 * Maximum values of pointer-holding  unsigned integer types.
 *
 * @def APL_UINT_MAX
 */

/**
 * unsigned integer type that not less than 127.
 *
 * @def APL_ATOM_MAX
 */


#if defined(PRId8)
#   define APL_PRId8        PRId8
#else
#   define APL_PRId8        "hhd" 
#endif

#if defined(PRId16)
#   define APL_PRId16       PRId16
#else
#   define APL_PRId16       "hd" 
#endif

#if defined(PRId32)
#   define APL_PRId32       PRId32
#else
#   define APL_PRId32       "d" 
#endif

#if defined(PRId64)
#   define APL_PRId64       PRId64
#else
#   define APL_PRId64       "lld"
#endif

#if defined(PRIdMAX)
#   define APL_PRIdMAX      PRIdMAX
#else
#   define APL_PRIdMAX      "lld"
#endif

#if defined(PRIdPTR)
#   define APL_PRIdINT      PRIdPTR
#else
#   define APL_PRIdINT      "ld"
#endif

/**
 * used in a format string to print the value of an integer of type 
 * apl_int8_t.
 *
 * @def APL_PRId8
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_int16_t.
 *
 * @def APL_PRId16
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_int32_t.
 *
 * @def APL_PRId32
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_int64_t.
 *
 * @def APL_PRId64
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_intmax_t.
 *
 * @def APL_PRIdMAX
 */

/**
 * used in a format string to print the value of an integer of type apl_int_t.
 *
 * @def APL_PRIdINT
 */


#if defined(PRIi8)
#   define APL_PRIi8        PRIi8
#else
#   define APL_PRIi8        "hhi" 
#endif

#if defined(PRIi16)
#   define APL_PRIi16       PRIi16
#else
#   define APL_PRIi16       "hi" 
#endif

#if defined(PRIi32)
#   define APL_PRIi32       PRIi32
#else
#   define APL_PRIi32       "i" 
#endif

#if defined(PRIi64)
#   define APL_PRIi64       PRIi64
#else
#   define APL_PRIi64       "lli"
#endif

#if defined(PRIiMAX)
#   define APL_PRIiMAX      PRIiMAX
#else
#   define APL_PRIiMAX      "lli"
#endif

#if defined(PRIiPTR)
#   define APL_PRIiINT      PRIiPTR
#else
#   define APL_PRIiINT      "li"
#endif


/**
 * used in a format string to print the value of an integer of type 
 * apl_int8_t.
 *
 * @def APL_PRIi8
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_int16_t.
 *
 * @def APL_PRIi16
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_int32_t.
 *
 * @def APL_PRIi32
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_int64_t.
 *
 * @def APL_PRIi64
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_intmax_t.
 *
 * @def APL_PRIiMAX
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_int_t.
 *
 * @def APL_PRIiINT
 */


#if defined(PRIu8)
#   define APL_PRIu8        PRIu8
#else
#   define APL_PRIu8        "hhu" 
#endif

#if defined(PRIu16)
#   define APL_PRIu16       PRIu16
#else
#   define APL_PRIu16       "hu" 
#endif

#if defined(PRIu32)
#   define APL_PRIu32       PRIu32
#else
#   define APL_PRIu32       "u" 
#endif

#if defined(PRIu64)
#   define APL_PRIu64       PRIu64
#else
#   define APL_PRIu64       "llu"
#endif

#if defined(PRIuMAX)
#   define APL_PRIuMAX      PRIuMAX
#else
#   define APL_PRIuMAX      "llu"
#endif

#if defined(PRIuPTR)
#   define APL_PRIuINT      PRIuPTR
#else
#   define APL_PRIuINT      "lu"
#endif

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint8_t.
 *
 * @def APL_PRIu8
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint16_t.
 *
 * @def APL_PRIu16
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint32_t.
 *
 * @def APL_PRIu32
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint64_t.
 *
 * @def APL_PRIu64
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uintmax_t.
 *
 * @def APL_PRIuMAX
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint_t.
 *
 * @def APL_PRIuINT
 */



#if defined(PRIo8)
#   define APL_PRIo8        PRIo8
#else
#   define APL_PRIo8        "hho" 
#endif

#if defined(PRIo16)
#   define APL_PRIo16       PRIo16
#else
#   define APL_PRIo16       "ho" 
#endif

#if defined(PRIo32)
#   define APL_PRIo32       PRIo32
#else
#   define APL_PRIo32       "o" 
#endif

#if defined(PRIo64)
#   define APL_PRIo64       PRIo64
#else
#   define APL_PRIo64       "llo"
#endif

#if defined(PRIoMAX)
#   define APL_PRIoMAX      PRIoMAX
#else
#   define APL_PRIoMAX      "llo"
#endif

#if defined(PRIoPTR)
#   define APL_PRIoINT      PRIoPTR
#else
#   define APL_PRIoINT      "lo"
#endif

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint8_t as octal.
 *
 * @def APL_PRIo8
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint16_t as octal.
 *
 * @def APL_PRIo16
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint32_t as octal.
 *
 * @def APL_PRIo32
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint64_t as octal.
 *
 * @def APL_PRIo64
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uintmax_t as octal.
 *
 * @def APL_PRIoMAX
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint_t as octal.
 *
 * @def APL_PRIoINT
 */



#if defined(PRIx8)
#   define APL_PRIx8        PRIx8
#else
#   define APL_PRIx8        "hhx" 
#endif

#if defined(PRIx16)
#   define APL_PRIx16       PRIx16
#else
#   define APL_PRIx16       "hx" 
#endif

#if defined(PRIx32)
#   define APL_PRIx32       PRIx32
#else
#   define APL_PRIx32       "x" 
#endif

#if defined(PRIx64)
#   define APL_PRIx64       PRIx64
#else
#   define APL_PRIx64       "llx"
#endif

#if defined(PRIxMAX)
#   define APL_PRIxMAX      PRIxMAX
#else
#   define APL_PRIxMAX      "llx"
#endif

#if defined(PRIxPTR)
#   define APL_PRIxINT      PRIxPTR
#else
#   define APL_PRIxINT      "lx"
#endif

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint8_t as hexadecimal.
 *
 * @def APL_PRIx8
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint16_t as hexadecimal.
 *
 * @def APL_PRIx16
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint32_t as hexadecimal.
 *
 * @def APL_PRIx32
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint64_t as hexadecimal.
 *
 * @def APL_PRIx64
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uintmax_t as hexadecimal.
 *
 * @def APL_PRIxMAX
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint_t as hexadecimal.
 *
 * @def APL_PRIxINT
 */



#if defined(PRIX8)
#   define APL_PRIX8        PRIX8
#else
#   define APL_PRIX8        "hhX" 
#endif

#if defined(PRIX16)
#   define APL_PRIX16       PRIX16
#else
#   define APL_PRIX16       "hX" 
#endif

#if defined(PRIX32)
#   define APL_PRIX32       PRIX32
#else
#   define APL_PRIX32       "X" 
#endif

#if defined(PRIX64)
#   define APL_PRIX64       PRIX64
#else
#   define APL_PRIX64       "llX"
#endif

#if defined(PRIXMAX)
#   define APL_PRIXMAX      PRIXMAX
#else
#   define APL_PRIXMAX      "llX"
#endif

#if defined(PRIXPTR)
#   define APL_PRIXINT      PRIXPTR
#else
#   define APL_PRIXINT      "lX"
#endif

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint8_t as hexadecimal.
 *
 * @def APL_PRIX8
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint16_t as hexadecimal.
 *
 * @def APL_PRIX16
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint32_t as hexadecimal.
 *
 * @def APL_PRIX32
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint64_t as hexadecimal.
 *
 * @def APL_PRIX64
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uintmax_t as hexadecimal.
 *
 * @def APL_PRIXMAX
 */

/**
 * used in a format string to print the value of an integer of type 
 * apl_uint_t as hexadecimal.
 *
 * @def APL_PRIXINT
 */

#define APL_SCNd8           SCNd8
#define APL_SCNd16          SCNd16
#define APL_SCNd32          SCNd32
#define APL_SCNd64          SCNd64
#define APL_SCNdMAX         SCNdMAX
#define APL_SCNdINT         SCNdPTR

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_int8_t.
 *
 * @def APL_SCNd8
 */

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_int16_t.
 *
 * @def APL_SCNd16
 */

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_int32_t.
 *
 * @def APL_SCNd32
 */

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_int64_t.
 *
 * @def APL_SCNd64
 */

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_intmax_t.
 *
 * @def APL_SCNdMAX
 */

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_int_t.
 *
 * @def APL_SCNdINT
 */

#define APL_SCNi8           SCNi8
#define APL_SCNi16          SCNi16
#define APL_SCNi32          SCNi32
#define APL_SCNi64          SCNi64
#define APL_SCNiMAX         SCNiMAX
#define APL_SCNiINT         SCNiPTR

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_int8_t.
 *
 * @def APL_SCNi8
 */

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_int16_t.
 *
 * @def APL_SCNi16
 */

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_int32_t.
 *
 * @def APL_SCNi32
 */

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_int64_t.
 *
 * @def APL_SCNi64
 */

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_intmax_t.
 *
 * @def APL_SCNiMAX
 */

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_int_t.
 *
 * @def APL_SCNiINT
 */

#define APL_SCNu8           SCNu8
#define APL_SCNu16          SCNu16
#define APL_SCNu32          SCNu32
#define APL_SCNu64          SCNu64
#define APL_SCNuMAX         SCNuMAX
#define APL_SCNuINT         SCNuPTR

/**
 * specifies the input stream shall match an optionally unsigned decimal 
 * integer which stores type is apl_uint8_t.
 *
 * @def APL_SCNu8
 */

/**
 * specifies the input stream shall match an optionally unsigned decimal 
 * integer which stores type is apl_uint16_t.
 *
 * @def APL_SCNu16
 */

/**
 * specifies the input stream shall match an optionally unsigned decimal 
 * integer which stores type is apl_uint32_t.
 *
 * @def APL_SCNu32
 */

/**
 * specifies the input stream shall match an optionally unsigned decimal 
 * integer which stores type is apl_uint64_t.
 *
 * @def APL_SCNu64
 */

/**
 * specifies the input stream shall match an optionally unsigned decimal 
 * integer which stores type is apl_uintmax_t.
 *
 * @def APL_SCNuMAX
 */

/**
 * specifies the input stream shall match an optionally signed decimal 
 * integer which stores type is apl_uint_t.
 *
 * @def APL_SCNuINT
 */

#define APL_SCNo8           SCNo8
#define APL_SCNo16          SCNo16
#define APL_SCNo32          SCNo32
#define APL_SCNo64          SCNo64
#define APL_SCNoMAX         SCNoMAX
#define APL_SCNoINT         SCNoPTR

/**
 * specifies the input stream shall match an optionally unsigned octal 
 * integer which stores type is apl_uint8_t.
 *
 * @def APL_SCNo8
 */

/**
 * specifies the input stream shall match an optionally unsigned octal 
 * integer which stores type is apl_uint16_t.
 *
 * @def APL_SCNo16
 */

/**
 * specifies the input stream shall match an optionally unsigned octal 
 * integer which stores type is apl_uint32_t.
 *
 * @def APL_SCNo32
 */

/**
 * specifies the input stream shall match an optionally unsigned octal 
 * integer which stores type is apl_uint64_t.
 *
 * @def APL_SCNo64
 */

/**
 * specifies the input stream shall match an optionally unsigned octal 
 * integer which stores type is apl_uintmax_t.
 *
 * @def APL_SCNoMAX
 */

/**
 * specifies the input stream shall match an optionally unsigned octal 
 * integer which stores type is apl_uint_t.
 *
 * @def APL_SCNoINT
 */

#define APL_SCNx8           SCNx8
#define APL_SCNx16          SCNx16
#define APL_SCNx32          SCNx32
#define APL_SCNx64          SCNx64
#define APL_SCNxMAX         SCNxMAX
#define APL_SCNxINT         SCNxPTR

/**
 * specifies the input stream shall match an optionally unsigned hexadecimal 
 * integer which stores type is apl_uint8_t.
 *
 * @def APL_SCNx8
 */

/**
 * specifies the input stream shall match an optionally unsigned hexadecimal 
 * integer which stores type is apl_uint16_t.
 *
 * @def APL_SCNx16
 */

/**
 * specifies the input stream shall match an optionally unsigned hexadecimal 
 * integer which stores type is apl_uint32_t.
 *
 * @def APL_SCNx32
 */

/**
 * specifies the input stream shall match an optionally unsigned hexadecimal 
 * integer which stores type is apl_uint64_t.
 *
 * @def APL_SCNx64
 */

/**
 * specifies the input stream shall match an optionally unsigned hexadecimal 
 * integer which stores type is apl_uintmax_t.
 *
 * @def APL_SCNxMAX
 */

/**
 * specifies the input stream shall match an optionally unsigned hexadecimal 
 * integer which stores type is apl_uint_t.
 *
 * @def APL_SCNxINT
 */



/** 
 * Convert a string to a apl_int32_t.
 *
 * This function shall convert the initial portion of the string pointed to 
 * by apc_path to a type apl_int32_t.
 *
 * They decompose the input string into three parts:
 * - An initial, possibly empty, sequence of white-space characters.
 * - A subject sequence interpreted as an integer represented in some radix 
 *   determined by the value of ai_base.
 * - A final string of one or more unrecognized characters, including the 
 *   terminating null byte of the input string.
 *
 * If the subject sequence has the expected form and the value of ai_base is 
 * 0, the sequence of characters starting with the first digit shall be 
 * interpreted as an integer constant. If the subject sequence has the 
 * expected form and the value of ai_base is between 2 and 36, it shall be 
 * used as the ai_base for conversion.
 *
 * If the subject sequence begins with a minus sign, the value resulting from 
 * the conversion shall be negated.
 *
 * @param[in]     apc_path       the input string which wanted to conversion.
 * @param[out]    appc_endptr    store the pointer points to the final string.
 * @param[in]     ai_base        represent the radix.
 *
 * @retval 0    no conversion could be performed,and errno indicate the error.
 * @retval !=0  conversion completion, the converted value would be returned.
 */
apl_int32_t apl_strtoi32(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base); 

/** 
 * Convert a string to a apl_uint32_t.
 *
 * This function shall be equivalent to the apl_strtoi32() function, except 
 * that the initial portion of the string shall be converted to apl_uint32_t 
 * representation, respectively.
 *
 * @see apl_strtoi32
 */
apl_uint32_t apl_strtou32(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base); 

/** 
 * Convert a string to a apl_int64_t.
 *
 * This function shall be equivalent to the apl_strtoi32() function, except 
 * that the initial portion of the string shall be converted to apl_int64_t 
 * representation, respectively.
 *
 * @see apl_strtoi32
 */
apl_int64_t apl_strtoi64(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base); 

/** 
 * Convert a string to a apl_uint64_t.
 *
 * This function shall be equivalent to the apl_strtoi32() function, except 
 * that the initial portion of the string shall be converted to apl_uint64_t 
 * representation, respectively.
 *
 * @see apl_strtoi32
 */
apl_uint64_t apl_strtou64(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base); 

/** 
 * Convert a string to a apl_intmax_t.
 *
 * This function shall be equivalent to the apl_strtoi32() function, except 
 * that the initial portion of the string shall be converted to apl_intmax_t 
 * representation, respectively.
 *
 * @see apl_strtoi32
 */
apl_intmax_t apl_strtoimax(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base); 

/** 
 * Convert a string to a apl_uintmax_t.
 *
 * This function shall be equivalent to the apl_strtoi32() function, except 
 * that the initial portion of the string shall be converted to apl_uintmax_t 
 * representation, respectively.
 *
 * @see apl_strtoi32
 */
apl_uintmax_t apl_strtoumax(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base); 

/** 
 * Convert values from host to network byte order
 *
 * This function shall convert 16-bit quantities from host byte order to 
 * network byte order.
 *
 * @param[in]   au16_h  the value with host byte order.
 *
 * @return  the network byte order of au16_h shall be returned.
 */
apl_uint16_t    apl_hton16(apl_uint16_t au16_h);

/** 
 * Convert values from network to host byte order
 *
 * This function shall convert 16-bit quantities from network byte order to 
 * host byte order.
 *
 * @param[in]    au16_n    the value with network byte order.
 *
 * @return    the host byte order of au16_n shall be returned.
 */
apl_uint16_t    apl_ntoh16(apl_uint16_t au16_n);

/** 
 * Convert values from host to network byte order
 *
 * This function shall convert 32-bit quantities from host byte order to 
 * network byte order.
 *
 * @param[in]    au32_h    the value with host byte order.
 *
 * @return    the network byte order of au32_h shall be returned.
 */
apl_uint32_t    apl_hton32(apl_uint32_t au32_h);

/** 
 * Convert values from network to host byte order
 *
 * This function shall convert 32-bit quantities from network byte order to 
 * host byte order.
 *
 * @param[in]    au32_n    the value with network byte order.
 *
 * @return    the host byte order of au32_n shall be returned.
 */
apl_uint32_t    apl_ntoh32(apl_uint32_t au32_n);

/** 
 * Convert values from host to network byte order
 *
 * This function shall convert 64-bit quantities from host byte order to 
 * network byte order.
 *
 * @param[in]    au64_h    the value with host byte order.
 *
 * @return    the network byte order of au64_h shall be returned.
 */
apl_uint64_t    apl_hton64(apl_uint64_t au64_h);

/** 
 * Convert values from host to network byte order
 *
 * This function shall convert 64-bit quantities from network byte order to 
 * host byte order.
 *
 * @param[in]    au64_n    the value with network byte order.
 *
 * @return    the host byte order of au64_n shall be returned.
 */
apl_uint64_t    apl_ntoh64(apl_uint64_t au64_n);



/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */


#endif /* APL_INTTYPES_H */

