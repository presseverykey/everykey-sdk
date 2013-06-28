/** endianness conversion macros */

#ifndef _ENDIAN_
#define _ENDIAN_

/** this should go to a more global configuration place. ARM states that endianness
 is implementation-dependent. For actual endianness, the ENDIANNESS flag in the
 AIRCR register should be checked. However, we don't want to spend runtime for that.
 Seems like a good idea to put this into a test once target unit tests are working. */

#define NATIVE_LE 1
// #define NATIVE_BE 1

/** very generic swapping */
#define ENDIAN_SWAP16(x) ((((uint16_t)((x) & 0xff00)) >> 8) | (((uint16_t)((x) & 0x00ff)) << 8))
#define ENDIAN_SWAP32(x) ((((uint16_t)((x) & 0xff000000)) >> 24) |\
							(((uint16_t)((x) & 0x00ff0000)) >> 8) |\
							(((uint16_t)((x) & 0x0000ff00)) << 8) |\
							(((uint16_t)((x) & 0x000000ff)) << 24))

/** type-aware swapping */
#define U16_SWAP(x) ((uint16_t)ENDIAN_SWAP16(x))
#define S16_SWAP(x) ((int16_t)ENDIAN_SWAP16(x))
#define U32_SWAP(x) ((uint32_t)ENDIAN_SWAP32(x))
#define S32_SWAP(x) ((int32_t)ENDIAN_SWAP32(x))

/** type- and native-aware swapping */

#ifdef NATIVE_BE

#define U16_TO_BE(x) (x)
#define U16_FROM_BE(x) (x)
#define U16_TO_LE(x) U16_SWAP(x)
#define U16_FROM_LE(x) U16_SWAP(x)

#define S16_TO_BE(x) (x)
#define S16_FROM_BE(x) (x)
#define S16_TO_LE(x) S16_SWAP(x)
#define S16_FROM_LE(x) S16_SWAP(x)

#define U32_TO_BE(x) (x)
#define U32_FROM_BE(x) (x)
#define U32_TO_LE(x) U32_SWAP(x)
#define U32_FROM_LE(x) U32_SWAP(x)

#define S32_TO_BE(x) (x)
#define S32_FROM_BE(x) (x)
#define S32_TO_LE(x) S32_SWAP(x)
#define S32_FROM_LE(x) S32_SWAP(x)

#elif NATIVE_LE

#define U16_TO_LE(x) (x)
#define U16_FROM_LE(x) (x)
#define U16_TO_BE(x) U16_SWAP(x)
#define U16_FROM_BE(x) U16_SWAP(x)

#define S16_TO_LE(x) (x)
#define S16_FROM_LE(x) (x)
#define S16_TO_BE(x) S16_SWAP(x)
#define S16_FROM_BE(x) S16_SWAP(x)

#define U32_TO_LE(x) (x)
#define U32_FROM_LE(x) (x)
#define U32_TO_BE(x) U32_SWAP(x)
#define U32_FROM_BE(x) U32_SWAP(x)

#define S32_TO_LE(x) (x)
#define S32_FROM_LE(x) (x)
#define S32_TO_BE(x) S32_SWAP(x)
#define S32_FROM_BE(x) S32_SWAP(x)


#else
#error "Native endianness not specified"
#endif

/** number to byte array conversion. useful for preparing endian-dependent binary data */

#define I16_TO_BE_BA(x) (((x) & 0xff00) >> 8), ((x) & 0x00ff)
#define I24_TO_BE_BA(x) (((x) & 0xff0000) >> 16), (((x) & 0x00ff00) >> 8), ((x) & 0x0000ff)
#define I32_TO_BE_BA(x) (((x) & 0xff000000) >> 24), (((x) & 0x00ff0000) >> 16), (((x) & 0x0000ff00) >> 8), ((x) & 0x000000ff)
#define I16_TO_LE_BA(x) ((x) & 0x00ff), (((x) & 0xff00) >> 8) 
#define I24_TO_LE_BA(x) ((x) & 0x0000ff), (((x) & 0x00ff00) >> 8), (((x) & 0xff0000) >> 16)
#define I32_TO_LE_BA(x) ((x) & 0x000000ff), (((x) & 0x0000ff00) >> 8), (((x) & 0x00ff0000) >> 16), (((x) & 0xff000000) >> 24)

#endif
