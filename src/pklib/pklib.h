/*****************************************************************************/
/* pklib.h                                Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* Header file for PKWARE Data Compression Library                           */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 31.03.03  1.00  Lad  Created                                              */
/*****************************************************************************/

#ifndef __PKLIB_H__
#define __PKLIB_H__

#pragma once

//-----------------------------------------------------------------------------
// Defines

#define CMP_BINARY             0            // Binary compression
#define CMP_ASCII              1            // Ascii compression

#define CMP_NO_ERROR           0
#define CMP_INVALID_DICTSIZE   1
#define CMP_INVALID_MODE       2
#define CMP_BAD_DATA           3
#define CMP_ABORT              4

#define CMP_IMPLODE_DICT_SIZE1   1024       // Dictionary size of 1024
#define CMP_IMPLODE_DICT_SIZE2   2048       // Dictionary size of 2048
#define CMP_IMPLODE_DICT_SIZE3   4096       // Dictionary size of 4096

enum CommonSizes {
    OUT_BUFF_SIZE = 0x802
};

enum LUTSizes {
    DIST_SIZES = 0x40,
    CH_BITS_ASC_SIZE = 0x100,
    LENS_SIZES = 0x10,
};

enum ImplodeSizes {
    OFFSS_SIZE2 = 0x204,
    LITERALS_COUNT = 0x306,
    HASHTABLE_SIZE = 0x900,
    BUFF_SIZE = 0x2204,
};

enum ExplodeSizes{
    IN_BUFF_SIZE = 0x800,
    CODES_SIZE = 0x100,
    OFFSS_SIZE = 0x100,
    OFFSS_SIZE1 = 0x80,
};

//-----------------------------------------------------------------------------
// Define calling convention

#ifndef PKEXPORT
#ifdef WIN32
#define PKEXPORT  __cdecl                   // Use for normal __cdecl calling
#else
#define PKEXPORT
#endif
#endif

//-----------------------------------------------------------------------------
// Internal structures

// Compression structure
typedef struct
{
    unsigned int   distance;                // 0000: Backward distance of the currently found repetition, decreased by 1
    unsigned int   out_bytes;               // 0004: # bytes available in out_buff
    unsigned int   out_bits;                // 0008: # of bits available in the last out byte
    unsigned int   dsize_bits;              // 000C: Number of bits needed for dictionary size. 4 = 0x400, 5 = 0x800, 6 = 0x1000
    unsigned int   dsize_mask;              // 0010: Bit mask for dictionary. 0x0F = 0x400, 0x1F = 0x800, 0x3F = 0x1000
    unsigned int   ctype;                   // 0014: Compression type (CMP_ASCII or CMP_BINARY)
    unsigned int   dsize_bytes;             // 0018: Dictionary size in bytes
    unsigned char  dist_bits[DIST_SIZES];   // 001C: Distance bits
    unsigned char  dist_codes[DIST_SIZES];  // 005C: Distance codes
    unsigned char  nChBits[LITERALS_COUNT]; // 009C: Table of literal bit lengths to be put to the output stream
    unsigned short nChCodes[LITERALS_COUNT];// 03A2: Table of literal codes to be put to the output stream
    unsigned short offs09AE;                // 09AE:

    void         * param;                   // 09B0: User parameter
    unsigned int (*read_buf)(char *buf, unsigned int *size, void *param);  // 9B4
    void         (*write_buf)(char *buf, unsigned int *size, void *param); // 9B8

    unsigned short offs09BC[OFFSS_SIZE2];   // 09BC:
    unsigned long  offs0DC4;                // 0DC4:
    unsigned short phash_to_index[HASHTABLE_SIZE];  // 0DC8: Array of indexes (one for each PAIR_HASH) to the "pair_hash_offsets" table
    unsigned short phash_to_index_end;      // 1FC8: End marker for "phash_to_index" table
    char           out_buff[OUT_BUFF_SIZE]; // 1FCA: Compressed data
    unsigned char  work_buff[BUFF_SIZE];     // 27CC: Work buffer
                                            //  + DICT_OFFSET  => Dictionary
                                            //  + UNCMP_OFFSET => Uncompressed data
    unsigned short phash_offs[BUFF_SIZE];    // 49D0: Table of offsets for each PAIR_HASH
} TCmpStruct;

#define CMP_BUFFER_SIZE  sizeof(TCmpStruct) // Size of compression structure.
                                            // Defined as 36312 in pkware header file


// Decompression structure
typedef struct
{
    unsigned long offs0000;                 // 0000
    unsigned long ctype;                    // 0004: Compression type (CMP_BINARY or CMP_ASCII)
    unsigned long outputPos;                // 0008: Position in output buffer
    unsigned long dsize_bits;               // 000C: Dict size (4, 5, 6 for 0x400, 0x800, 0x1000)
    unsigned long dsize_mask;               // 0010: Dict size bitmask (0x0F, 0x1F, 0x3F for 0x400, 0x800, 0x1000)
    unsigned long bit_buff;                 // 0014: 16-bit buffer for processing input data
    unsigned long extra_bits;               // 0018: Number of extra (above 8) bits in bit buffer
    unsigned int  in_pos;                   // 001C: Position in in_buff
    unsigned long in_bytes;                 // 0020: Number of bytes in input buffer
    void        * param;                    // 0024: Custom parameter
    unsigned int (*read_buf)(char *buf, unsigned int *size, void *param); // Pointer to function that reads data from the input stream
    void         (*write_buf)(char *buf, unsigned int *size, void *param);// Pointer to function that writes data to the output stream

    unsigned char out_buff[BUFF_SIZE];      // 0030: Output circle buffer.
                                            //       0x0000 - 0x0FFF: Previous uncompressed data, kept for repetitions
                                            //       0x1000 - 0x1FFF: Currently decompressed data
                                            //       0x2000 - 0x2203: Reserve space for the longest possible repetition
    unsigned char in_buff[IN_BUFF_SIZE];           // 2234: Buffer for data to be decompressed
    unsigned char DistPosCodes[CODES_SIZE];      // 2A34: Table of distance position codes
    unsigned char LengthCodes[CODES_SIZE];       // 2B34: Table of length codes
    unsigned char offs2C34[OFFSS_SIZE];          // 2C34: Buffer for
    unsigned char offs2D34[OFFSS_SIZE];          // 2D34: Buffer for
    unsigned char offs2E34[OFFSS_SIZE1];           // 2E34: Buffer for
    unsigned char offs2EB4[OFFSS_SIZE];          // 2EB4: Buffer for
    unsigned char ChBitsAsc[CH_BITS_ASC_SIZE];         // 2FB4: Buffer for
    unsigned char DistBits[DIST_SIZES];           // 30B4: Numbers of bytes to skip copied block length
    unsigned char LenBits[LENS_SIZES];            // 30F4: Numbers of bits for skip copied block length
    unsigned char ExLenBits[LENS_SIZES];          // 3104: Number of valid bits for copied block
    unsigned short LenBase[LENS_SIZES];           // 3114: Buffer for
} TDcmpStruct;

#define EXP_BUFFER_SIZE sizeof(TDcmpStruct) // Size of decompression structure
                                            // Defined as 12596 in pkware headers

//-----------------------------------------------------------------------------
// Tables (in explode.c)

extern const unsigned char DistBits[DIST_SIZES];
extern const unsigned char DistCode[DIST_SIZES];
extern const unsigned char ExLenBits[LENS_SIZES];
extern const unsigned short LenBase[LENS_SIZES];
extern const unsigned char LenBits[LENS_SIZES];
extern const unsigned char LenCode[LENS_SIZES];
extern const unsigned char ChBitsAsc[CH_BITS_ASC_SIZE];
extern const unsigned short ChCodeAsc[CH_BITS_ASC_SIZE];

//-----------------------------------------------------------------------------
// Public functions

#ifdef __cplusplus
   extern "C" {
#endif

unsigned int PKEXPORT implode(
   unsigned int (*read_buf)(char *buf, unsigned int *size, void *param),
   void         (*write_buf)(char *buf, unsigned int *size, void *param),
   char         *work_buf,
   void         *param,
   unsigned int *type,
   unsigned int *dsize);


unsigned int PKEXPORT explode(
   unsigned int (*read_buf)(char *buf, unsigned  int *size, void *param),
   void         (*write_buf)(char *buf, unsigned  int *size, void *param),
   char         *work_buf,
   void         *param);

// The original name "crc32" was changed to "crc32_pklib" due
// to compatibility with zlib
unsigned long PKEXPORT crc32_pklib(char *buffer, unsigned int *size, unsigned long *old_crc);

#ifdef __cplusplus
   }                         // End of 'extern "C"' declaration
#endif

#endif // __PKLIB_H__
