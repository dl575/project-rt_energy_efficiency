
 /*
   -----------------------------------------------------------------------
   Copyright (c) 2001 Dr Brian Gladman <brg@gladman.uk.net>, Worcester, UK
   
   TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   This software is provided 'as is' with no guarantees of correctness or
   fitness for purpose.
   -----------------------------------------------------------------------

   1. FUNCTION
 
   The AES algorithm Rijndael implemented for block and key sizes of 128,
   192 and 256 bits (16, 24 and 32 bytes) by Brian Gladman.

   This is an implementation of the AES encryption algorithm (Rijndael)
   designed by Joan Daemen and Vincent Rijmen. This version is designed
   to provide both fixed and dynamic block and key lengths and can also 
   run with either big or little endian internal byte order (see aes.h). 
   It inputs block and key lengths in bytes with the legal values being 
   16, 24 and 32.
 
   2. THE CIPHER INTERFACE

   byte                    (an unsigned 8-bit type)
   word                    (an unsigned 32-bit type)
   aes_ret:                (a signed 16 bit type for function return values)
        aes_good            (value != 0, a good return)
        aes_bad             (value == 0, an error return)
   enum aes_key:           (encryption direction)
        enc                 (set key for encryption)
        dec                 (set key for decryption)
        both                (set key for both)
   class or struct aes     (structure for context)

   C subroutine calls:

   aes_ret set_blk(const word block_length, aes *cx)  (variable block size)
   aes_ret set_key(const byte key[], const word key_length,
                   const enum aes_key direction, aes *cx)
   aes_ret encrypt(const byte input_blk[], byte output_blk[], const aes *cx)
   aes_ret decrypt(const byte input_blk[], byte output_blk[], const aes *cx)

   IMPORTANT NOTE: If you are using this C interface and your compiler does 
   not set the memory used for objects to zero before use, you will need to 
   ensure that cx.mode is set to zero before using the C subroutine calls.

   C++ aes class subroutines:

   aes_ret set_blk(const word block_length)  (variable block size)
   aes_ret set_key(const byte key[], const word key_length,
                   const aes_key direction)
   aes_ret encrypt(const byte input_blk[], byte output_blk[]) const
   aes_ret decrypt(const byte input_blk[], byte output_blk[]) const

   The block length inputs to set_block and set_key are in numbers of
   BYTES, not bits.  The calls to subroutines must be made in the above 
   order but multiple calls can be made without repeating earlier calls
   if their parameters have not changed. If the cipher block length is
   variable but set_blk has not been called before cipher operations a
   value of 16 is assumed (that is, the AES block size). In contrast to 
   earlier versions the block and key length parameters are now checked
   for correctness and the encryption and decryption routines check to 
   ensure that an appropriate key has been set before they are called.
   
   3. BYTE ORDER WITHIN 32 BIT WORDS

   The fundamental data processing units in Rijndael are 8-bit bytes. The 
   input, the output and the key input are all enumerated arrays of bytes 
   in which bytes are numbered starting at zero and increasing to one less
   than the number of bytes in the array in question.  When these inputs 
   and outputs are considered as bit sequences, the n'th byte contains 
   bits 8n to 8n+7 of the sequence with the lower numbered bit mapped to 
   the most significant bit within the  byte (i.e. that having a numeric 
   value of 128).  However, Rijndael can be implemented more efficiently 
   using 32-bit words to process 4 bytes at a time provided that the order
   of bytes within words is known.  This order is called big-endian if the 
   lowest numbered bytes in words have the highest numeric significance 
   and little-endian if the opposite applies. This code can work in either 
   order irrespective of the native order of the machine on which it runs.
   The byte order used internally is set by defining INTERNAL_BYTE_ORDER
   whereas the order for all inputs and outputs is specified by defining 
   EXTERNAL_BYTE_ORDER, the only purpose of the latter being to determine
   if a byte order change is needed immediately after input and immediately
   before output to account for the use of a different internal byte order.  
   In almost all situations both of these defines will be set to the native 
   order of the processor on which the code is to run but other settings 
   may somtimes be useful in special circumstances.

#define INTERNAL_BYTE_ORDER AES_LITTLE_ENDIAN
#define EXTERNAL_BYTE_ORDER AES_LITTLE_ENDIAN

   4. COMPILATION 

   To compile AES (Rijndael) for use in C code
    a. Exclude the AES_DLL define in aes.h
    b. Exclude the AES_IN_CPP define in aes.h

   To compile AES (Rijndael) for use in in C++ code
    a. Exclude the AES_DLL define in aes.h
    b. Include the AES_IN_CPP define in aes.h

   To compile AES (Rijndael) in C as a Dynamic Link Library
    a. Include the AES_DLL define in aes.h
    b. Compile the DLL.  If using the test files, exclude aes.c from
       the test build project and compile it with the same defines 
       as used for the DLL (ensure that the DLL path is correct)

   6. CONFIGURATION OPTIONS (see also aes.c)

   a. define BLOCK_SIZE to set the cipher block size (16, 24 or 32) or
      leave this undefined for dynamically variable block size (this will
      result in much slower code).
   b. set AES_IN_CPP to use the code from C++ rather than C
   c. set AES_DLL if AES (Rijndael) is to be compiled to a DLL
   d. set INTERNAL_BYTE_ORDER to one of the above constants to set the
      internal byte order (the order used within the algorithm code)
   e. set EXTERNAL_BYTE_ORDER to one of the above constants to set the byte
      order used at the external interfaces for the input, output and key
      byte arrays.

   IMPORTANT NOTE: BLOCK_SIZE is in BYTES: 16, 24, 32 or undefined for aes.c
   and 16, 20, 24, 28, 32 or undefined for aespp.c.  If left undefined a 
   slower version providing variable block length is compiled   

#define BLOCK_SIZE  16

   Define AES_IN_CPP if you intend to use the AES C++ class rather than the
   C code directly.

#define AES_IN_CPP

   Define AES_DLL if you wish to compile the code to produce a Windows DLL

#define AES_DLL

*/

#ifndef _AES_H
#define _AES_H

#define BLOCK_SIZE  16

#define AES_BIG_ENDIAN      1   /* do not change */
#define AES_LITTLE_ENDIAN   2   /* do not change */

#define INTERNAL_BYTE_ORDER AES_LITTLE_ENDIAN
#define EXTERNAL_BYTE_ORDER AES_LITTLE_ENDIAN

#if defined(BLOCK_SIZE) && ((BLOCK_SIZE & 3) || BLOCK_SIZE < 16 || BLOCK_SIZE > 32)
#error an illegal block size has been specified
#endif  

#if defined(AES_DLL)
#undef AES_IN_CPP
#endif

/* Adjust the C subroutine names if using the C++ interface */

#if defined(AES_IN_CPP)
#define c_name(x)   c_##x
#else
#define c_name(x)   x
#endif

 /*
   The number of key schedule words for different block and key lengths
   (allowing for the method of computation which requires the length to 
   be a multiple of the key length):

   Key Schedule    key length (bytes)
   Length          16  20  24  28  32
                ---------------------
   block     16 |  44  60  54  56  64
   length    20 |  60  60  66  70  80
   (bytes)   24 |  80  80  78  84  96
             28 | 100 100 102  98 112
             32 | 120 120 120 126 120

   Rcon Table      key length (bytes)
   Length          16  20  24  28  32
                ---------------------
   block     16 |  10   9   8   7   7
   length    20 |  14  11  10   9   9
   (bytes)   24 |  19  15  12  11  11
             28 |  24  19  16  13  13
             32 |  29  23  19  17  14
   
   The following values assume that the key length will be variable and may
   be of maximum length (32 bytes). 

   Nk = number_of_key_bytes / 4
   Nc = number_of_columns_in_state / 4
   Nr = number of encryption/decryption rounds
   Rc = number of elements in rcon table
   Ks = number of 32-bit words in key schedule
 */

#define Nr(Nk,Nc)   ((Nk > Nc ? Nk : Nc) + 6)
#define Rc(Nk,Nc)   ((Nb * (Nr(Nk,Nc) + 1) - 1) / Nk)   
#define Ks(Nk,Nc)   (Nk * (Rc(Nk,Nc) + 1))

#if !defined(BLOCK_SIZE)
#define RC_LENGTH    29
#define KS_LENGTH   128
#else
#define RC_LENGTH   5 * BLOCK_SIZE / 4 - (BLOCK_SIZE == 16 ? 10 : 11)
#define KS_LENGTH   4 * BLOCK_SIZE
#endif

/* End of configuration options, but see also aes.c */

typedef unsigned char   byte;           /* must be an 8-bit storage unit */
typedef unsigned long   word;           /* must be a 32-bit storage unit */
typedef short           aes_ret;        /* function return value         */

#define aes_bad     0
#define aes_good    1

 /*
   upr(x,n): rotates bytes within words by n positions, moving bytes 
   to higher index positions with wrap around into low positions
   ups(x,n): moves bytes by n positions to higher index positions in 
   words but without wrap around
   bval(x,n): extracts a byte from a word
 */

#if(INTERNAL_BYTE_ORDER == AES_LITTLE_ENDIAN)

#define upr(x,n)        (((x) << 8 * (n)) | ((x) >> (32 - 8 * (n))))
#define ups(x,n)        ((x) << 8 * (n))
#define bval(x,n)       ((byte)((x) >> 8 * (n)))
#define byte_swap(x)    (upr(x,1) & 0x00ff00ff | upr(x,3) & 0xff00ff00)
#define bytes2word(b0, b1, b2, b3) ((word)(b3) << 24 | (word)(b2) << 16 | (word)(b1) << 8 | (b0))
#else

#define upr(x,n)        (((x) >> 8 * (n)) | ((x) << (32 - 8 * (n))))
#define ups(x,n)        ((x) >> 8 * (n)))
#define bval(x,n)       ((byte)((x) >> 24 - 8 * (n)))
#define byte_swap(x)    (upr(x,3) & 0x00ff00ff | upr(x,1) & 0xff00ff00)
#define bytes2word(b0, b1, b2, b3) ((word)(b0) << 24 | (word)(b1) << 16 | (word)(b2) << 8 | (b3))
#endif

#if(INTERNAL_BYTE_ORDER == EXTERNAL_BYTE_ORDER)

#define word_in(x)      *(word*)(x)
#define word_out(x,v)   *(word*)(x) = (v)

#else

#define word_in(x)      byte_swap(*(word*)(x))
#define word_out(x,v)   *(word*)(x) = byte_swap(v)

#endif

enum aes_const  {   Nrow =  4,  /* the number of rows in the cipher state       */
                    Mcol =  8,  /* maximum number of columns in the state       */
#if defined(BLOCK_SIZE)         /* set up a statically defined block size       */
                    Ncol =  BLOCK_SIZE / 4,  
                    Shr0 =  0,  /* the cyclic shift values for rows 0, 1, 2 & 3 */
                    Shr1 =  1,  
                    Shr2 =  BLOCK_SIZE == 32 ? 3 : 2,
                    Shr3 =  BLOCK_SIZE == 32 ? 4 : 3
#endif 
                };

enum aes_key    {   enc  =  1,  /* set if encryption is needed */
                    dec  =  2,  /* set if decryption is needed */
                    both =  3   /* set if both are needed      */
                };

#if defined(__cplusplus)
extern "C"
{
#endif

/* _stdcall is needed for Visual Basic DLLs but is not necessary for C/C++ */
 
#if defined(AES_DLL)
#define cf_dec  aes_ret __declspec(dllexport) _stdcall
#else
#define cf_dec  aes_ret
#endif

typedef struct
{
    word    Nkey;               /* the number of words in the key input block */
    word    Nrnd;               /* the number of cipher rounds                */
    word    e_key[KS_LENGTH];   /* the encryption key schedule                */
    word    d_key[KS_LENGTH];   /* the decryption key schedule                */
#if !defined(BLOCK_SIZE)
    word    Ncol;               /* the number of columns in the cipher state  */
#endif
    byte    mode;               /* encrypt, decrypt or both                   */
} c_name(aes);

cf_dec c_name(set_key)(const byte key[], const word n_bytes, const enum aes_key f, c_name(aes) *cx);
cf_dec c_name(encrypt)(const byte in_blk[], byte out_blk[], const c_name(aes) *cx);
cf_dec c_name(decrypt)(const byte in_blk[], byte out_blk[], const c_name(aes) *cx);
cf_dec c_name(set_blk)(const word n_bytes, c_name(aes) *cx);

#if defined(__cplusplus)
}

#if defined(AES_IN_CPP) 

class aes
{
    c_name(aes) cx;
public:            
#if defined(BLOCK_SIZE)
    aes()   { cx.mode = 0; }
#else     
    aes(word n_bytes = 16)
            { cx.mode = 0; c_name(set_blk)(n_bytes, &cx); }
    aes_ret set_blk(const word n_bytes)
            { return c_name(set_blk)(n_bytes, &cx); }
#endif
    aes_ret set_key(const byte key[], const word n_bytes, const aes_key f)
            { return c_name(set_key)(key, n_bytes, f, &cx); }
    aes_ret encrypt(const byte in_blk[], byte out_blk[]) const
            { return c_name(encrypt)(in_blk, out_blk, &cx); }
    aes_ret decrypt(const byte in_blk[], byte out_blk[]) const
            { return c_name(decrypt)(in_blk, out_blk, &cx); }
};

#endif
#endif
#endif
 /*
   -----------------------------------------------------------------------
   Copyright (c) 2001 Dr Brian Gladman <brg@gladman.uk.net>, Worcester, UK
   
   TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   This software is provided 'as is' with no guarantees of correctness or
   fitness for purpose.
   -----------------------------------------------------------------------

   1. FUNCTION
 
   The AES algorithm Rijndael implemented for block and key sizes of 128,
   192 and 256 bits (16, 24 and 32 bytes) by Brian Gladman.

   This is an implementation of the AES encryption algorithm (Rijndael)
   designed by Joan Daemen and Vincent Rijmen. This version is designed
   to provide both fixed and dynamic block and key lengths and can also 
   run with either big or little endian internal byte order (see aes.h). 
   It inputs block and key lengths in bytes with the legal values being 
   16, 24 and 32.
 
   2. CONFIGURATION OPTIONS (see also aes.h)
 
   a.  Define UNROLL for full loop unrolling in encryption and decryption.
   b.  Define PARTIAL_UNROLL to unroll two loops in encryption and decryption.
   c.  Define FIXED_TABLES for compiled rather than dynamic tables.
   d.  Define FF_TABLES to use tables for field multiplies and inverses.
   e.  Define ARRAYS to use arrays to hold the local state block. If this
       is not defined, individually declared 32-bit words are used.
   f.  Define FAST_VARIABLE if a high speed variable block implementation
       is needed (essentially three separate fixed block size code sequences)
   g.  Define either ONE_TABLE or FOUR_TABLES for a fast table driven 
       version using 1 table (2 kbytes of table space) or 4 tables (8
       kbytes of table space) for higher speed.
   h.  Define either ONE_LR_TABLE or FOUR_LR_TABLES for a further speed 
       increase by using tables for the last rounds but with more table
       space (2 or 8 kbytes extra).
   i.  If neither ONE_TABLE nor FOUR_TABLES is defined, a compact but 
       slower version is provided.
   j.  If fast decryption key scheduling is needed define ONE_IM_TABLE
       or FOUR_IM_TABLES for higher speed (2 or 8 kbytes extra).

   3. USE OF DEFINES
  
   NOTE: some combinations of the following defines are disabled below.

   UNROLL or PARTIAL_UNROLL control the extent to which loops are unrolled
   in the main encryption and decryption routines. UNROLL does a complete
   unroll while PARTIAL_UNROLL uses a loop with two rounds in it.
 
#define UNROLL
#define PARTIAL_UNROLL
 
   If FIXED_TABLES is defined, the tables are comipled statically into the 
   code, otherwise they are computed once when the code is first used.
 
#define FIXED_TABLES
 
   If FF_TABLES is defined faster finite field arithmetic is performed by 
   using tables.
 
#define FF_TABLES

   If ARRAYS is defined the state variables for encryption are defined as
   arrays, otherwise they are defined as individual variables. The latter
   is useful on machines where these variables can be mapped to registers. 
 
#define ARRAYS

   If FAST_VARIABLE is defined with variable block length, faster but larger
   code is used for encryption and decryption.

#define FAST_VARIABLE
 */

#define UNROLL
#define FIXED_TABLES
#define FF_TABLES
#define ARRAYS
#define FAST_VARIABLE

 /*
   This code uses three sets of tables, each of which can be a single table
   or four sub-tables to gain a further speed advantage.

   The defines ONE_TABLE and FOUR_TABLES control the use of tables in the 
   main encryption rounds and have the greatest impact on speed.  If neither
   is defined, tables are not used and the resulting code is then very slow.
   Defining ONE_TABLE gives a substantial speed increase using 2 kbytes of 
   table space; FOUR_TABLES gives a further speed increase but uses 8 kbytes
   of table space.
   
#define ONE_TABLE
#define FOUR_TABLES

   The defines ONE_LR_TABLE and FOUR_LR_TABLES apply to the last round only
   and their impact on speed is hence less. It is unlikely to be sensible to
   apply these options unless the correspnding option above is also used.    

#define ONE_LR_TABLE
#define FOUR_LR_TABLES

   The ONE_IM_TABLE and FOUR_IM_TABLES options use tables to speed up the 
   generation of the decryption key schedule. This will only be useful in
   limited situations where decryption speed with frequent re-keying is
   needed.

#define ONE_IM_TABLE
#define FOUR_IM_TABLES
 */

//#include "aes.h"

#define FOUR_TABLES
#define FOUR_LR_TABLES
#define FOUR_IM_TABLES

 /*
   In this implementation the columns of the state array are each held in
   32-bit words. The state array can be held in various ways: in an array
   of words, in a number of individual word variables or in a number of 
   processor registers. The following define maps a variable name x and
   a column number c to the way the state array variable is to be held.
 */

#if defined(ARRAYS)
#define s(x,c) x[c]
#else
#define s(x,c) x##c
#endif

#if defined(BLOCK_SIZE) && (BLOCK_SIZE == 20 || BLOCK_SIZE == 28)
#error an illegal block size has been specified
#endif  

#if defined(UNROLL) && defined (PARTIAL_UNROLL)
#error both UNROLL and PARTIAL_UNROLL are defined
#endif

#if defined(ONE_TABLE) && defined (FOUR_TABLES)
#error both ONE_TABLE and FOUR_TABLES are defined
#endif

#if defined(ONE_LR_TABLE) && defined (FOUR_LR_TABLES)
#error both ONE_LR_TABLE and FOUR_LR_TABLES are defined
#endif

#if defined(ONE_IM_TABLE) && defined (FOUR_IM_TABLES)
#error both ONE_IM_TABLE and FOUR_IM_TABLES are defined
#endif

/* End of configuration options */
/* Disable at least some poor combinations of options */

#if !defined(ONE_TABLE) && !defined(FOUR_TABLES)
#define FIXED_TABLES
#undef  UNROLL
#undef  ONE_LR_TABLE
#undef  FOUR_LR_TABLES
#undef  ONE_IM_TABLE
#undef  FOUR_IM_TABLES
#elif !defined(FOUR_TABLES)
#ifdef  FOUR_LR_TABLES
#undef  FOUR_LR_TABLES
#define ONE_LR_TABLE
#endif
#ifdef  FOUR_IM_TABLES
#undef  FOUR_IM_TABLES
#define ONE_IM_TABLE
#endif
#elif !defined(BLOCK_SIZE)
#if defined(UNROLL)
#define PARTIAL_UNROLL
#undef UNROLL
#endif
#endif

/* the finite field modular polynomial and elements */

#define ff_poly 0x011b
#define ff_hi   0x80

/* multiply four bytes in GF(2^8) by 'x' {02} in parallel */

#define m1  0x80808080
#define m2  0x7f7f7f7f
#define m3  0x0000001b
#define FFmulX(x)  ((((x) & m2) << 1) ^ ((((x) & m1) >> 7) * m3))

 /* 
   The following defines provide alternative definitions of FFmulX that might
   give improved performance if a fast 32-bit multiply is not available. Note
   that a temporary variable u needs to be defined where FFmulX is used.

#define FFmulX(x) (u = (x) & m1, u |= (u >> 1), ((x) & m2) << 1) ^ ((u >> 3) | (u >> 6)) 
#define m4  0x1b1b1b1b
#define FFmulX(x) (u = (x) & m1, ((x) & m2) << 1) ^ ((u - (u >> 7)) & m4) 

 */

/* perform column mix operation on four bytes in parallel */

#define fwd_mcol(x) (f2 = FFmulX(x), f2 ^ upr(x ^ f2,3) ^ upr(x,2) ^ upr(x,1))

#if defined(FIXED_TABLES)

//#include "aestab.h"

#else

static byte  s_box[256];
static byte  inv_s_box[256];
static word  rcon_tab[RC_LENGTH];

#if defined(ONE_TABLE)
static word  ft_tab[256];
static word  it_tab[256];
#elif defined(FOUR_TABLES)
static word  ft_tab[4][256];
static word  it_tab[4][256];
#endif

#if defined(ONE_LR_TABLE)
static word  fl_tab[256];
static word  il_tab[256];
#elif defined(FOUR_LR_TABLES)
static word  fl_tab[4][256];
static word  il_tab[4][256];
#endif

#if defined(ONE_IM_TABLE)
static word  im_tab[256];
#elif defined(FOUR_IM_TABLES)
static word  im_tab[4][256];
#endif

#if !defined(FF_TABLES)

/*
   Generate the tables for the dynamic table option

   It will generally be sensible to use tables to compute finite 
   field multiplies and inverses but where memory is scarse this 
   code might sometimes be better.

   return 2 ^ (n - 1) where n is the bit number of the highest bit
   set in x with x in the range 1 < x < 0x00000200.   This form is
   used so that locals within FFinv can be bytes rather than words
*/

static byte hibit(const word x)
{   byte r = (byte)((x >> 1) | (x >> 2));
    
    r |= (r >> 2);
    r |= (r >> 4);
    return (r + 1) >> 1;
}

/* return the inverse of the finite field element x */

static byte FFinv(const byte x)
{   byte    p1 = x, p2 = 0x1b, n1 = hibit(x), n2 = 0x80, v1 = 1, v2 = 0;

    if(x < 2) return x;

    for(;;)
    {
        if(!n1) return v1;

        while(n2 >= n1)
        {   
            n2 /= n1; p2 ^= p1 * n2; v2 ^= v1 * n2; n2 = hibit(p2);
        }
        
        if(!n2) return v2;

        while(n1 >= n2)
        {   
            n1 /= n2; p1 ^= p2 * n1; v1 ^= v2 * n1; n1 = hibit(p1);
        }
    }
}

/* define the finite field multiplies required for Rijndael */

#define FFmul02(x)  ((((x) & 0x7f) << 1) ^ ((x) & 0x80 ? 0x1b : 0))
#define FFmul03(x)  ((x) ^ FFmul02(x))
#define FFmul09(x)  ((x) ^ FFmul02(FFmul02(FFmul02(x))))
#define FFmul0b(x)  ((x) ^ FFmul02((x) ^ FFmul02(FFmul02(x))))
#define FFmul0d(x)  ((x) ^ FFmul02(FFmul02((x) ^ FFmul02(x))))
#define FFmul0e(x)  FFmul02((x) ^ FFmul02((x) ^ FFmul02(x)))

#else

#define FFinv(x)    ((x) ? pow[255 - log[x]]: 0)

#define FFmul02(x) (x ? pow[log[x] + 0x19] : 0)
#define FFmul03(x) (x ? pow[log[x] + 0x01] : 0)
#define FFmul09(x) (x ? pow[log[x] + 0xc7] : 0)
#define FFmul0b(x) (x ? pow[log[x] + 0x68] : 0)
#define FFmul0d(x) (x ? pow[log[x] + 0xee] : 0)
#define FFmul0e(x) (x ? pow[log[x] + 0xdf] : 0)

#endif

/* The forward and inverse affine transformations used in the S-box */

#define fwd_affine(x)     (w = (word)x, w ^= (w<<1)^(w<<2)^(w<<3)^(w<<4), 0x63^(byte)(w^(w>>8)))

#define inv_affine(x)     (w = (word)x, w = (w<<1)^(w<<3)^(w<<6), 0x05^(byte)(w^(w>>8)))

static void gen_tabs(void)
{   word  i, w;

#if defined(FF_TABLES)

    byte  pow[512], log[256];

    /*
       log and power tables for GF(2^8) finite field with
       0x011b as modular polynomial - the simplest primitive
       root is 0x03, used here to generate the tables
    */

    i = 0; w = 1; 
    do
    {   
        pow[i] = (byte)w;
        pow[i + 255] = (byte)w;
        log[w] = (byte)i++;
        w ^=  (w << 1) ^ (w & ff_hi ? ff_poly : 0);
    }
    while (w != 1);

#endif

    for(i = 0, w = 1; i < RC_LENGTH; ++i)
    {
        rcon_tab[i] = bytes2word(w, 0, 0, 0);
        w = (w << 1) ^ (w & ff_hi ? ff_poly : 0);
    }

    for(i = 0; i < 256; ++i)
    {   byte    b;

        s_box[i] = b = fwd_affine(FFinv((byte)i));

        w = bytes2word(b, 0, 0, 0);
#if defined(ONE_LR_TABLE)
        fl_tab[i] = w;
#elif defined(FOUR_LR_TABLES)
        fl_tab[0][i] = w;
        fl_tab[1][i] = upr(w,1);
        fl_tab[2][i] = upr(w,2);
        fl_tab[3][i] = upr(w,3);
#endif
        w = bytes2word(FFmul02(b), b, b, FFmul03(b));
#if defined(ONE_TABLE)
        ft_tab[i] = w;
#elif defined(FOUR_TABLES)
        ft_tab[0][i] = w;
        ft_tab[1][i] = upr(w,1);
        ft_tab[2][i] = upr(w,2);
        ft_tab[3][i] = upr(w,3);
#endif
        inv_s_box[i] = b = FFinv(inv_affine((byte)i));

        w = bytes2word(b, 0, 0, 0);
#if defined(ONE_LR_TABLE)
        il_tab[i] = w;
#elif defined(FOUR_LR_TABLES)
        il_tab[0][i] = w;
        il_tab[1][i] = upr(w,1);
        il_tab[2][i] = upr(w,2);
        il_tab[3][i] = upr(w,3);
#endif
        w = bytes2word(FFmul0e(b), FFmul09(b), FFmul0d(b), FFmul0b(b));
#if defined(ONE_TABLE)
        it_tab[i] = w;
#elif defined(FOUR_TABLES)
        it_tab[0][i] = w;
        it_tab[1][i] = upr(w,1);
        it_tab[2][i] = upr(w,2);
        it_tab[3][i] = upr(w,3);
#endif
#if defined(ONE_IM_TABLE)
        im_tab[b] = w;
#elif defined(FOUR_IM_TABLES)
        im_tab[0][b] = w;
        im_tab[1][b] = upr(w,1);
        im_tab[2][b] = upr(w,2);
        im_tab[3][b] = upr(w,3);
#endif

    }
}

#endif

#define no_table(x,box,vf,rf,c) bytes2word(     box[bval(vf(x,0,c),rf(0,c))],     box[bval(vf(x,1,c),rf(1,c))],     box[bval(vf(x,2,c),rf(2,c))],     box[bval(vf(x,3,c),rf(3,c))])

#define one_table(x,op,tab,vf,rf,c)  (     tab[bval(vf(x,0,c),rf(0,c))]   ^ op(tab[bval(vf(x,1,c),rf(1,c))],1)   ^ op(tab[bval(vf(x,2,c),rf(2,c))],2)   ^ op(tab[bval(vf(x,3,c),rf(3,c))],3))

#define four_tables(x,tab,vf,rf,c)  (  tab[0][bval(vf(x,0,c),rf(0,c))]   ^ tab[1][bval(vf(x,1,c),rf(1,c))]   ^ tab[2][bval(vf(x,2,c),rf(2,c))]   ^ tab[3][bval(vf(x,3,c),rf(3,c))])

#define vf1(x,r,c)  (x)
#define rf1(r,c)    (r)
#define rf2(r,c)    ((r-c)&3)

#if defined(FOUR_LR_TABLES)
#define ls_box(x,c)     four_tables(x,fl_tab,vf1,rf2,c)
#elif defined(ONE_LR_TABLE)
#define ls_box(x,c)     one_table(x,upr,fl_tab,vf1,rf2,c)
#else
#define ls_box(x,c)     no_table(x,s_box,vf1,rf2,c)
#endif

#if defined(FOUR_IM_TABLES)
#define inv_mcol(x)     four_tables(x,im_tab,vf1,rf1,0)
#elif defined(ONE_IM_TABLE)
#define inv_mcol(x)     one_table(x,upr,im_tab,vf1,rf1,0)
#else
#define inv_mcol(x)     (f9 = (x),f2 = FFmulX(f9), f4 = FFmulX(f2), f8 = FFmulX(f4), f9 ^= f8,     f2 ^= f4 ^ f8 ^ upr(f2 ^ f9,3) ^ upr(f4 ^ f9,2) ^ upr(f9,1))
#endif

 /* 
   Subroutine to set the block size (if variable) in bytes, legal
   values being 16, 24 and 32.
 */

#if defined(BLOCK_SIZE)
#define nc   (Ncol)
#else
#define nc   (cx->Ncol)

cf_dec c_name(set_blk)(const word n_bytes, c_name(aes) *cx)
{
#if !defined(FIXED_TABLES)
    if(!(cx->mode & 0x08)) { gen_tabs(); cx->mode = 0x08; }
#endif

    if((n_bytes & 7) || n_bytes < 16 || n_bytes > 32) 
    {     
        return (n_bytes ? cx->mode &= ~0x07, aes_bad : (aes_ret)(nc << 2));
    }

    cx->mode = cx->mode & ~0x07 | 0x04;
    nc = n_bytes >> 2;
    return aes_good;
}

#endif

 /*
   Initialise the key schedule from the user supplied key. The key
   length is now specified in bytes - 16, 24 or 32 as appropriate.
   This corresponds to bit lengths of 128, 192 and 256 bits, and
   to Nk values of 4, 6 and 8 respectively.
 */

#define mx(t,f) (*t++ = inv_mcol(*f),f++)
#define cp(t,f) *t++ = *f++

#if   BLOCK_SIZE == 16
#define cpy(d,s)    cp(d,s); cp(d,s); cp(d,s); cp(d,s)
#define mix(d,s)    mx(d,s); mx(d,s); mx(d,s); mx(d,s)
#elif BLOCK_SIZE == 24
#define cpy(d,s)    cp(d,s); cp(d,s); cp(d,s); cp(d,s);                     cp(d,s); cp(d,s)
#define mix(d,s)    mx(d,s); mx(d,s); mx(d,s); mx(d,s);                     mx(d,s); mx(d,s)
#elif BLOCK_SIZE == 32
#define cpy(d,s)    cp(d,s); cp(d,s); cp(d,s); cp(d,s);                     cp(d,s); cp(d,s); cp(d,s); cp(d,s)
#define mix(d,s)    mx(d,s); mx(d,s); mx(d,s); mx(d,s);                     mx(d,s); mx(d,s); mx(d,s); mx(d,s)
#else

#define cpy(d,s) switch(nc) {   case 8: cp(d,s); cp(d,s);     case 6: cp(d,s); cp(d,s);     case 4: cp(d,s); cp(d,s);             cp(d,s); cp(d,s); }

#define mix(d,s) switch(nc) {   case 8: mx(d,s); mx(d,s);     case 6: mx(d,s); mx(d,s);     case 4: mx(d,s); mx(d,s);             mx(d,s); mx(d,s); }

#endif

cf_dec c_name(set_key)(const byte in_key[], const word n_bytes, const enum aes_key f, c_name(aes) *cx)
{   word    *kf, *kt, rci;

#if !defined(FIXED_TABLES)
    if(!(cx->mode & 0x08)) { gen_tabs(); cx->mode = 0x08; }
#endif

#if !defined(BLOCK_SIZE)
    if(!(cx->mode & 0x04)) c_name(set_blk)(16, cx);
#endif

    if((n_bytes & 7) || n_bytes < 16 || n_bytes > 32 || !(f & 1) && !(f & 2)) 
    {     
        return (n_bytes ? cx->mode &= ~0x03, aes_bad : (aes_ret)(cx->Nkey << 2));
    }

    cx->mode = cx->mode & ~0x03 | (byte)f & 0x03;      
    cx->Nkey = n_bytes >> 2;
    cx->Nrnd = Nr(cx->Nkey, nc);

    cx->e_key[0] = word_in(in_key     );
    cx->e_key[1] = word_in(in_key +  4);
    cx->e_key[2] = word_in(in_key +  8);
    cx->e_key[3] = word_in(in_key + 12);

    kf = cx->e_key; 
    kt = kf + nc * (cx->Nrnd + 1) - cx->Nkey; 
    rci = 0;

    switch(cx->Nkey)
    {
    case 4: do
            {   kf[4] = kf[0] ^ ls_box(kf[3],3) ^ rcon_tab[rci++];
                kf[5] = kf[1] ^ kf[4];
                kf[6] = kf[2] ^ kf[5];
                kf[7] = kf[3] ^ kf[6];
                kf += 4;
            }
            while(kf < kt);
            break;

    case 6: cx->e_key[4] = word_in(in_key + 16);
            cx->e_key[5] = word_in(in_key + 20);
            do
            {   kf[ 6] = kf[0] ^ ls_box(kf[5],3) ^ rcon_tab[rci++];
                kf[ 7] = kf[1] ^ kf[ 6];
                kf[ 8] = kf[2] ^ kf[ 7];
                kf[ 9] = kf[3] ^ kf[ 8];
                kf[10] = kf[4] ^ kf[ 9];
                kf[11] = kf[5] ^ kf[10];
                kf += 6;
            }
            while(kf < kt);
            break;

    case 8: cx->e_key[4] = word_in(in_key + 16);
            cx->e_key[5] = word_in(in_key + 20);
            cx->e_key[6] = word_in(in_key + 24);
            cx->e_key[7] = word_in(in_key + 28);
            do
            {   kf[ 8] = kf[0] ^ ls_box(kf[7],3) ^ rcon_tab[rci++];
                kf[ 9] = kf[1] ^ kf[ 8];
                kf[10] = kf[2] ^ kf[ 9];
                kf[11] = kf[3] ^ kf[10];
                kf[12] = kf[4] ^ ls_box(kf[11],0);
                kf[13] = kf[5] ^ kf[12];
                kf[14] = kf[6] ^ kf[13];
                kf[15] = kf[7] ^ kf[14];
                kf += 8;
            }
            while (kf < kt);
            break;
    }

    if((cx->mode & 3) != enc)
    {   word    i;
        
        kt = cx->d_key + nc * cx->Nrnd;
        kf = cx->e_key;
        
        cpy(kt, kf); kt -= 2 * nc;

        for(i = 1; i < cx->Nrnd; ++i)
        { 
#if defined(ONE_TABLE) || defined(FOUR_TABLES)
#if !defined(ONE_IM_TABLE) && !defined(FOUR_IM_TABLES)
            word    f2, f4, f8, f9;
#endif
            mix(kt, kf);
#else
            cpy(kt, kf);
#endif
            kt -= 2 * nc;
        }
        
        cpy(kt, kf);
    }

    return aes_good;
}

 /*
   I am grateful to Frank Yellin for the following constructions
   which, given the column (c) of the output state variable, give
   the input state variables which are needed for each row (r) of 
   the state.

   For the fixed block size options, compilers should reduce these 
   two expressions to fixed variable references. But for variable 
   block size code conditional clauses will sometimes be returned.

   y = output word, x = input word, r = row, c = column for r = 0, 
   1, 2 and 3 = column accessed for row r.
 */

#define unused  77  /* Sunset Strip */

#define fwd_var(x,r,c)  ( r==0 ?               ( c==0 ? s(x,0)     : c==1 ? s(x,1)     : c==2 ? s(x,2)     : c==3 ? s(x,3)     : c==4 ? s(x,4)     : c==5 ? s(x,5)     : c==6 ? s(x,6)     : s(x,7))        : r==1 ?               ( c==0 ? s(x,1)     : c==1 ? s(x,2)     : c==2 ? s(x,3)     : c==3 ? nc==4 ? s(x,0) : s(x,4)     : c==4 ? s(x,5)     : c==5 ? nc==8 ? s(x,6) : s(x,0)     : c==6 ? s(x,7)     : s(x,0))        : r==2 ?               ( c==0 ? nc==8 ? s(x,3) : s(x,2)     : c==1 ? nc==8 ? s(x,4) : s(x,3)     : c==2 ? nc==4 ? s(x,0) : nc==8 ? s(x,5) : s(x,4)     : c==3 ? nc==4 ? s(x,1) : nc==8 ? s(x,6) : s(x,5)     : c==4 ? nc==8 ? s(x,7) : s(x,0)     : c==5 ? nc==8 ? s(x,0) : s(x,1)     : c==6 ? s(x,1)     : s(x,2))        :                      ( c==0 ? nc==8 ? s(x,4) : s(x,3)     : c==1 ? nc==4 ? s(x,0) : nc==8 ? s(x,5) : s(x,4)     : c==2 ? nc==4 ? s(x,1) : nc==8 ? s(x,6) : s(x,5)     : c==3 ? nc==4 ? s(x,2) : nc==8 ? s(x,7) : s(x,0)     : c==4 ? nc==8 ? s(x,0) : s(x,1)     : c==5 ? nc==8 ? s(x,1) : s(x,2)     : c==6 ? s(x,2)     : s(x,3)))

#define inv_var(x,r,c)  ( r==0 ?               ( c==0 ? s(x,0)     : c==1 ? s(x,1)     : c==2 ? s(x,2)     : c==3 ? s(x,3)     : c==4 ? s(x,4)     : c==5 ? s(x,5)     : c==6 ? s(x,6)     : s(x,7))        : r==1 ?               ( c==0 ? nc==4 ? s(x,3) : nc==8 ? s(x,7) : s(x,5)     : c==1 ? s(x,0)     : c==2 ? s(x,1)     : c==3 ? s(x,2)     : c==4 ? s(x,3)     : c==5 ? s(x,4)     : c==6 ? s(x,5)     : s(x,6))        : r==2 ?               ( c==0 ? nc==4 ? s(x,2) : nc==8 ? s(x,5) : s(x,4)     : c==1 ? nc==4 ? s(x,3) : nc==8 ? s(x,6) : s(x,5)     : c==2 ? nc==8 ? s(x,7) : s(x,0)     : c==3 ? nc==8 ? s(x,0) : s(x,1)     : c==4 ? nc==8 ? s(x,1) : s(x,2)     : c==5 ? nc==8 ? s(x,2) : s(x,3)     : c==6 ? s(x,3)     : s(x,4))        :                      ( c==0 ? nc==4 ? s(x,1) : nc==8 ? s(x,4) : s(x,3)     : c==1 ? nc==4 ? s(x,2) : nc==8 ? s(x,5) : s(x,4)     : c==2 ? nc==4 ? s(x,3) : nc==8 ? s(x,6) : s(x,5)     : c==3 ? nc==8 ? s(x,7) : s(x,0)     : c==4 ? nc==8 ? s(x,0) : s(x,1)     : c==5 ? nc==8 ? s(x,1) : s(x,2)     : c==6 ? s(x,2)     : s(x,3)))

#define si(y,x,k,c) s(y,c) = word_in(x + 4 * c) ^ k[c]
#define so(y,x,c)   word_out(y + 4 * c, s(x,c))

#if defined(FOUR_TABLES)
#define fwd_rnd(y,x,k,c)    s(y,c)= (k)[c] ^ four_tables(x,ft_tab,fwd_var,rf1,c)
#define inv_rnd(y,x,k,c)    s(y,c)= (k)[c] ^ four_tables(x,it_tab,inv_var,rf1,c)
#elif defined(ONE_TABLE)
#define fwd_rnd(y,x,k,c)    s(y,c)= (k)[c] ^ one_table(x,upr,ft_tab,fwd_var,rf1,c)
#define inv_rnd(y,x,k,c)    s(y,c)= (k)[c] ^ one_table(x,upr,it_tab,inv_var,rf1,c)
#else
#define fwd_rnd(y,x,k,c)    s(y,c) = fwd_mcol(no_table(x,s_box,fwd_var,rf1,c)) ^ (k)[c]
#define inv_rnd(y,x,k,c)    s(y,c) = inv_mcol(no_table(x,inv_s_box,inv_var,rf1,c) ^ (k)[c])
#endif

#if defined(FOUR_LR_TABLES)
#define fwd_lrnd(y,x,k,c)   s(y,c)= (k)[c] ^ four_tables(x,fl_tab,fwd_var,rf1,c)
#define inv_lrnd(y,x,k,c)   s(y,c)= (k)[c] ^ four_tables(x,il_tab,inv_var,rf1,c)
#elif defined(ONE_LR_TABLE)
#define fwd_lrnd(y,x,k,c)   s(y,c)= (k)[c] ^ one_table(x,ups,fl_tab,fwd_var,rf1,c)
#define inv_lrnd(y,x,k,c)   s(y,c)= (k)[c] ^ one_table(x,ups,il_tab,inv_var,rf1,c)
#else
#define fwd_lrnd(y,x,k,c)   s(y,c) = no_table(x,s_box,fwd_var,rf1,c) ^ (k)[c]
#define inv_lrnd(y,x,k,c)   s(y,c) = no_table(x,inv_s_box,inv_var,rf1,c) ^ (k)[c]
#endif

#if BLOCK_SIZE == 16

#if defined(ARRAYS)
#define locals(y,x)     x[4],y[4]
#else
#define locals(y,x)     x##0,x##1,x##2,x##3,y##0,y##1,y##2,y##3
 /* 
   the following defines prevent the compiler requiring the declaration
   of generated but unused variables in the fwd_var and inv_var macros
 */
#define b04 unused
#define b05 unused
#define b06 unused
#define b07 unused
#define b14 unused
#define b15 unused
#define b16 unused
#define b17 unused
#endif
#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1);                         s(y,2) = s(x,2); s(y,3) = s(x,3);
#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2); si(y,x,k,3)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); so(y,x,3)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); rm(y,x,k,3)

#elif BLOCK_SIZE == 24

#if defined(ARRAYS)
#define locals(y,x)     x[6],y[6]
#else
#define locals(y,x)     x##0,x##1,x##2,x##3,x##4,x##5,                         y##0,y##1,y##2,y##3,y##4,y##5
#define b06 unused
#define b07 unused
#define b16 unused
#define b17 unused
#endif
#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1);                         s(y,2) = s(x,2); s(y,3) = s(x,3);                         s(y,4) = s(x,4); s(y,5) = s(x,5);
#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2);                         si(y,x,k,3); si(y,x,k,4); si(y,x,k,5)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2);                         so(y,x,3); so(y,x,4); so(y,x,5)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2);                         rm(y,x,k,3); rm(y,x,k,4); rm(y,x,k,5)
#else

#if defined(ARRAYS)
#define locals(y,x)     x[8],y[8]
#else
#define locals(y,x)     x##0,x##1,x##2,x##3,x##4,x##5,x##6,x##7,                         y##0,y##1,y##2,y##3,y##4,y##5,y##6,y##7
#endif
#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1);                         s(y,2) = s(x,2); s(y,3) = s(x,3);                         s(y,4) = s(x,4); s(y,5) = s(x,5);                         s(y,6) = s(x,6); s(y,7) = s(x,7);

#if BLOCK_SIZE == 32

#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2); si(y,x,k,3);                         si(y,x,k,4); si(y,x,k,5); si(y,x,k,6); si(y,x,k,7)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); so(y,x,3);                         so(y,x,4); so(y,x,5); so(y,x,6); so(y,x,7)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); rm(y,x,k,3);                         rm(y,x,k,4); rm(y,x,k,5); rm(y,x,k,6); rm(y,x,k,7)
#else

#define state_in(y,x,k) switch(nc) {   case 8: si(y,x,k,7); si(y,x,k,6);     case 6: si(y,x,k,5); si(y,x,k,4);     case 4: si(y,x,k,3); si(y,x,k,2);             si(y,x,k,1); si(y,x,k,0); }

#define state_out(y,x) switch(nc) {   case 8: so(y,x,7); so(y,x,6);     case 6: so(y,x,5); so(y,x,4);     case 4: so(y,x,3); so(y,x,2);             so(y,x,1); so(y,x,0); }

#if defined(FAST_VARIABLE)

#define round(rm,y,x,k) switch(nc) {   case 8: rm(y,x,k,7); rm(y,x,k,6);             rm(y,x,k,5); rm(y,x,k,4);             rm(y,x,k,3); rm(y,x,k,2);             rm(y,x,k,1); rm(y,x,k,0);             break;     case 6: rm(y,x,k,5); rm(y,x,k,4);             rm(y,x,k,3); rm(y,x,k,2);             rm(y,x,k,1); rm(y,x,k,0);             break;     case 4: rm(y,x,k,3); rm(y,x,k,2);             rm(y,x,k,1); rm(y,x,k,0);             break; }
#else

#define round(rm,y,x,k) switch(nc) {   case 8: rm(y,x,k,7); rm(y,x,k,6);     case 6: rm(y,x,k,5); rm(y,x,k,4);     case 4: rm(y,x,k,3); rm(y,x,k,2);             rm(y,x,k,1); rm(y,x,k,0); }

#endif

#endif
#endif

cf_dec c_name(encrypt)(const byte in_blk[], byte out_blk[], const c_name(aes) *cx)
{   word        locals(b0, b1);
    const word  *kp = cx->e_key;

#if !defined(ONE_TABLE) && !defined(FOUR_TABLES)
    word        f2;
#endif

    if(!(cx->mode & 0x01)) return aes_bad;

    state_in(b0, in_blk, kp); kp += nc;

#if defined(UNROLL)

    switch(cx->Nrnd)
    {
    case 14:    round(fwd_rnd,  b1, b0, kp         ); 
                round(fwd_rnd,  b0, b1, kp + nc    ); kp += 2 * nc;
    case 12:    round(fwd_rnd,  b1, b0, kp         ); 
                round(fwd_rnd,  b0, b1, kp + nc    ); kp += 2 * nc;
    case 10:    round(fwd_rnd,  b1, b0, kp         );             
                round(fwd_rnd,  b0, b1, kp +     nc);
                round(fwd_rnd,  b1, b0, kp + 2 * nc); 
                round(fwd_rnd,  b0, b1, kp + 3 * nc);
                round(fwd_rnd,  b1, b0, kp + 4 * nc); 
                round(fwd_rnd,  b0, b1, kp + 5 * nc);
                round(fwd_rnd,  b1, b0, kp + 6 * nc); 
                round(fwd_rnd,  b0, b1, kp + 7 * nc);
                round(fwd_rnd,  b1, b0, kp + 8 * nc);
                round(fwd_lrnd, b0, b1, kp + 9 * nc);
    }
#elif defined(PARTIAL_UNROLL)
    {   word    rnd;

        for(rnd = 0; rnd < (cx->Nrnd >> 1) - 1; ++rnd)
        {
            round(fwd_rnd, b1, b0, kp); 
            round(fwd_rnd, b0, b1, kp + nc); kp += 2 * nc;
        }

        round(fwd_rnd,  b1, b0, kp);
        round(fwd_lrnd, b0, b1, kp + nc);
    }
#else
    {   word    rnd;

        for(rnd = 0; rnd < cx->Nrnd - 1; ++rnd)
        {
            round(fwd_rnd, b1, b0, kp); 
            l_copy(b0, b1); kp += nc;
        }

        round(fwd_lrnd, b0, b1, kp);
    }
#endif

    state_out(out_blk, b0);
    return aes_good;
}

cf_dec c_name(decrypt)(const byte in_blk[], byte out_blk[], const c_name(aes) *cx)
{   word        locals(b0, b1);
    const word  *kp = cx->d_key;

#if !defined(ONE_TABLE) && !defined(FOUR_TABLES)
    word        f2, f4, f8, f9; 
#endif

    if(!(cx->mode & 0x02)) return aes_bad;

    state_in(b0, in_blk, kp); kp += nc;

#if defined(UNROLL)

    switch(cx->Nrnd)
    {
    case 14:    round(inv_rnd,  b1, b0, kp         );
                round(inv_rnd,  b0, b1, kp + nc    ); kp += 2 * nc;
    case 12:    round(inv_rnd,  b1, b0, kp         );
                round(inv_rnd,  b0, b1, kp + nc    ); kp += 2 * nc;
    case 10:    round(inv_rnd,  b1, b0, kp         );             
                round(inv_rnd,  b0, b1, kp +     nc);
                round(inv_rnd,  b1, b0, kp + 2 * nc); 
                round(inv_rnd,  b0, b1, kp + 3 * nc);
                round(inv_rnd,  b1, b0, kp + 4 * nc); 
                round(inv_rnd,  b0, b1, kp + 5 * nc);
                round(inv_rnd,  b1, b0, kp + 6 * nc); 
                round(inv_rnd,  b0, b1, kp + 7 * nc);
                round(inv_rnd,  b1, b0, kp + 8 * nc);
                round(inv_lrnd, b0, b1, kp + 9 * nc);
    }
#elif defined(PARTIAL_UNROLL)
    {   word    rnd;

        for(rnd = 0; rnd < (cx->Nrnd >> 1) - 1; ++rnd)
        {
            round(inv_rnd, b1, b0, kp); 
            round(inv_rnd, b0, b1, kp + nc); kp += 2 * nc;
        }

        round(inv_rnd,  b1, b0, kp);
        round(inv_lrnd, b0, b1, kp + nc);
    }
#else
    {   word    rnd;

        for(rnd = 0; rnd < cx->Nrnd - 1; ++rnd)
        {
            round(inv_rnd, b1, b0, kp); 
            l_copy(b0, b1); kp += nc;
        }

        round(inv_lrnd, b0, b1, kp);
    }
#endif

    state_out(out_blk, b0);
    return aes_good;
}


 /*
   -----------------------------------------------------------------------
   Copyright (c) 2001 Dr Brian Gladman <brg@gladman.uk.net>, Worcester, UK
   
   TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   This software is provided 'as is' with no guarantees of correctness or
   fitness for purpose.
   -----------------------------------------------------------------------
 */

/* Example of the use of the AES (Rijndael) algorithm for file  */
/* encryption.  Note that this is an example application, it is */
/* not intended for real operational use.  The Command line is: */
/*                                                              */
/* aesxam input_file_name output_file_name [D|E] hexadecimalkey */
/*                                                              */
/* where E gives encryption and D decryption of the input file  */
/* into the output file using the given hexadecimal key string  */
/* The later is a hexadecimal sequence of 32, 48 or 64 digits   */
/* Examples to encrypt or decrypt aes.c into aes.enc are:       */
/*                                                              */
/* aesxam file.c file.enc E 0123456789abcdeffedcba9876543210    */
/*                                                              */
/* aesxam file.enc file2.c D 0123456789abcdeffedcba9876543210   */
/*                                                              */
/* which should return a file 'file2.c' identical to 'file.c'   */

//#include <stdio.h>
//#include <stdlib.h>
//#include <memory.h>
//#include <ctype.h>

//#include "aes.h"

/* A Pseudo Random Number Generator (PRNG) used for the     */
/* Initialisation Vector. The PRNG is George Marsaglia's    */
/* Multiply-With-Carry (MWC) PRNG that concatenates two     */
/* 16-bit MWC generators:                                   */
/*     x(n)=36969 * x(n-1) + carry mod 2^16                 */ 
/*     y(n)=18000 * y(n-1) + carry mod 2^16                 */
/* to produce a combined PRNG with a period of about 2^60.  */  
/* The Pentium cycle counter is used to initialise it. This */
/* is crude but the IV does not need to be secret.          */
 
/* void cycles(unsigned long *rtn)     */
/* {                           // read the Pentium Time Stamp Counter */
/*     __asm */
/*     { */
/*     _emit   0x0f            // complete pending operations */
/*     _emit   0xa2 */
/*     _emit   0x0f            // read time stamp counter */
/*     _emit   0x31 */
/*     mov     ebx,rtn */
/*     mov     [ebx],eax */
/*     mov     [ebx+4],edx */
/*     _emit   0x0f            // complete pending operations */
/*     _emit   0xa2 */
/*     } */
/* } */

#define RAND(a,b) (((a = 36969 * (a & 65535) + (a >> 16)) << 16) + (b = 18000 * (b & 65535) + (b >> 16))  )

void fillrand(char *buf, int len)
{   static unsigned long a[2], mt = 1, count = 4;
    static char          r[4];
    int                  i;

    if(mt) { 
	 mt = 0; 
	 /*cycles(a);*/
      a[0]=0xeaf3;
	 a[1]=0x35fe;
    }

    for(i = 0; i < len; ++i)
    {
        if(count == 4)
        {
            *(unsigned long*)r = RAND(a[0], a[1]);
            count = 0;
        }

        buf[i] = r[count++];
    }
}    

int encfile(FILE *fout, aes *ctx, char* fn, char *file_buffer, int flen)
{   char            inbuf[16], outbuf[16];
    //int          flen;
    unsigned long   i=0, l=0;

    fillrand(outbuf, 16);           /* set an IV for CBC mode           */
    //fseek(fin, 0, SEEK_END);        /* get the length of the file       */
    //fgetpos(fin, &flen);            /* and then reset to start          */
    //fseek(fin, 0, SEEK_SET);        
    fwrite(outbuf, 1, 16, fout);    /* write the IV to the output       */
    fillrand(inbuf, 1);             /* make top 4 bits of a byte random */
    l = 15;                         /* and store the length of the last */
                                    /* block in the lower 4 bits        */
    inbuf[0] = ((char)flen & 15) | (inbuf[0] & ~15);

    //while(!feof(fin))               /* loop to encrypt the input file   */
    int j;
    for (j = 0; j < flen; j += 16)
    {                               /* input 1st 16 bytes to buf[1..16] */
        //i = fread(inbuf + 16 - l, 1, l, fin);  /*  on 1st round byte[0] */
                                               /* is the length code    */
        //if(i < l) break;            /* if end of the input file reached */
        i = flen - j;
        if (i < l) {
            int k;
            memcpy(inbuf + 16 - l, file_buffer + j, i); 
            break;
        }
        memcpy(inbuf + 16 - l, file_buffer + j, l); 

        for(i = 0; i < 16; ++i)         /* xor in previous cipher text  */
            inbuf[i] ^= outbuf[i]; 

        encrypt(inbuf, outbuf, ctx);    /* and do the encryption        */

        if(fwrite(outbuf, 1, 16, fout) != 16)
        {
            printf("Error writing to output file: %s\n", fn);
            return -7;
        }
                                    /* in all but first round read 16   */
        l = 16;                     /* bytes into the buffer            */
    }

    /* except for files of length less than two blocks we now have one  */
    /* byte from the previous block and 'i' bytes from the current one  */
    /* to encrypt and 15 - i empty buffer positions. For files of less  */
    /* than two blocks (0 or 1) we have i + 1 bytes and 14 - i empty    */
    /* buffer position to set to zero since the 'count' byte is extra   */

    if(l == 15)                         /* adjust for extra byte in the */
        ++i;                            /* in the first block           */

    if(i)                               /* if bytes remain to be output */
    {
        while(i < 16)                   /* clear empty buffer positions */
            inbuf[i++] = 0;
     
        for(i = 0; i < 16; ++i)         /* xor in previous cipher text  */
            inbuf[i] ^= outbuf[i]; 

        encrypt(inbuf, outbuf, ctx);    /* encrypt and output it        */

        if(fwrite(outbuf, 1, 16, fout) != 16)
        {
            printf("Error writing to output file: %s\n", fn);
            return -8;
        }
    }
        
    return 0;
}

int decfile(FILE *fin, FILE *fout, aes *ctx, char* ifn, char* ofn)
{   char    inbuf1[16], inbuf2[16], outbuf[16], *bp1, *bp2, *tp;
    int     i, l, flen;

    if(fread(inbuf1, 1, 16, fin) != 16)  /* read Initialisation Vector   */
    {
        printf("Error reading from input file: %s\n", ifn);
        return 9;
    }

    i = fread(inbuf2, 1, 16, fin);  /* read 1st encrypted file block    */

    if(i && i != 16)
    {
        printf("\nThe input file is corrupt");
        return -10;
    }

    decrypt(inbuf2, outbuf, ctx);   /* decrypt it                       */

    for(i = 0; i < 16; ++i)         /* xor with previous input          */
        outbuf[i] ^= inbuf1[i];

    flen = outbuf[0] & 15;  /* recover length of the last block and set */
    l = 15;                 /* the count of valid bytes in block to 15  */                              
    bp1 = inbuf1;           /* set up pointers to two input buffers     */
    bp2 = inbuf2;

    while(1)
    {
        i = fread(bp1, 1, 16, fin);     /* read next encrypted block    */
                                        /* to first input buffer        */
        if(i != 16)         /* no more bytes in input - the decrypted   */
            break;          /* partial final buffer needs to be output  */

        /* if a block has been read the previous block must have been   */
        /* full lnegth so we can now write it out                       */
         
        if(fwrite(outbuf + 16 - l, 1, l, fout) != (unsigned long)l)
        {
            printf("Error writing to output file: %s\n", ofn);
            return -11;
        }

        decrypt(bp1, outbuf, ctx);  /* decrypt the new input block and  */

        for(i = 0; i < 16; ++i)     /* xor it with previous input block */
            outbuf[i] ^= bp2[i];
        
        /* set byte count to 16 and swap buffer pointers                */

        l = i; tp = bp1, bp1 = bp2, bp2 = tp;
    }

    /* we have now output 16 * n + 15 bytes of the file with any left   */
    /* in outbuf waiting to be output. If x bytes remain to be written, */
    /* we know that (16 * n + x + 15) % 16 = flen, giving x = flen + 1  */
    /* But we must also remember that the first block is offset by one  */
    /* in the buffer - we use the fact that l = 15 rather than 16 here  */  

    l = (l == 15 ? 1 : 0); flen += 1 - l;

    if(flen)
        if(fwrite(outbuf + l, 1, flen, fout) != (unsigned long)flen)
        {
            printf("Error writing to output file: %s\n", ofn);
            return -12;
        }

    return 0;
}

int main(int argc, char *argv[])
{   FILE    *fin = 0, *fout = 0;
    char    *cp, ch, key[32];
    int     i=0, by=0, key_len=0, err = 0;
    aes     ctx[1];

    if(argc != 5 || (toupper(*argv[3]) != 'D' && toupper(*argv[3]) != 'E'))
    {
        printf("usage: rijndael in_filename out_filename [d/e] key_in_hex\n"); 
        err = -1; goto exit;
    }

    cp = argv[4];   /* this is a pointer to the hexadecimal key digits  */
    i = 0;          /* this is a count for the input digits processed   */
    
    while(i < 64 && *cp)    /* the maximum key length is 32 bytes and   */
    {                       /* hence at most 64 hexadecimal digits      */
        ch = toupper(*cp++);            /* process a hexadecimal digit  */
        if(ch >= '0' && ch <= '9')
            by = (by << 4) + ch - '0';
        else if(ch >= 'A' && ch <= 'F')
            by = (by << 4) + ch - 'A' + 10;
        else                            /* error if not hexadecimal     */
        {
            printf("key must be in hexadecimal notation\n"); 
            err = -2; goto exit;
        }
        
        /* store a key byte for each pair of hexadecimal digits         */
        if(i++ & 1) 
            key[i / 2 - 1] = by & 0xff; 
    }

    if(*cp)
    {
        printf("The key value is too long\n"); 
        err = -3; goto exit;
    }
    else if(i < 32 || (i & 15))
    {
        printf("The key length must be 32, 48 or 64 hexadecimal digits\n");
        err = -4; goto exit;
    }

    key_len = i / 2;

    if(!(fin = fopen(argv[1], "rb")))   /* try to open the input file */
    {
        printf("The input file: %s could not be opened\n", argv[1]); 
        err = -5; goto exit;
    }

    if(!(fout = fopen(argv[2], "wb")))  /* try to open the output file */
    {
        printf("The output file: %s could not be opened\n", argv[1]); 
        err = -6; goto exit;
    }

    if(toupper(*argv[3]) == 'E')
    {                           /* encryption in Cipher Block Chaining mode */
        set_key(key, key_len, enc, ctx);

        // Get file length
        int flen;
        fseek(fin, 0, SEEK_END);        /* get the length of the file       */
        fgetpos(fin, &flen);            /* and then reset to start          */
        fseek(fin, 0, SEEK_SET);        
        // Allocate buffer
        char *file_buffer = malloc(sizeof(char) * (flen + 1));
        // Read file into buffer
        size_t newLen = fread(file_buffer, sizeof(char), flen, fin);
        if (newLen == 0) {
            printf("Error reading file\n");
            exit(1);
        } else {
            file_buffer[++newLen] = '\0';
        }

        err = encfile(fout, ctx, argv[1], file_buffer, flen);
    }
    else
    {                           /* decryption in Cipher Block Chaining mode */
        set_key(key, key_len, dec, ctx);
    
        err = decfile(fin, fout, ctx, argv[1], argv[2]);
    }
exit:   
    if(fout) 
        fclose(fout);
    if(fin)
        fclose(fin);

    return err;
}


 /*
   -----------------------------------------------------------------------
   Copyright (c) 2001 Dr Brian Gladman <brg@gladman.uk.net>, Worcester, UK
   
   TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   This software is provided 'as is' with no guarantees of correctness or
   fitness for purpose.
   -----------------------------------------------------------------------
 */

const byte s_box[256] =
{
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

const byte inv_s_box[256] =
{
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38,
    0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87,
    0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d,
    0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2,
    0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16,
    0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda,
    0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a,
    0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02,
    0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea,
    0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85,
    0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89,
    0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20,
    0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31,
    0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d,
    0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0,
    0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26,
    0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

 /*
   Used to ensure table is generated in the right format
   depending on the internal byte order required.
 */

#if (INTERNAL_BYTE_ORDER == AES_LITTLE_ENDIAN)
#define w0(p)          0x000000##p
#else
#define w0(p)        0x##p##000000
#endif

 /*
   Number of elements required in this table for different
   block and key lengths is:

   Rcon Table      key length (bytes)
   Length          16  20  24  28  32
                ---------------------
   block     16 |  10   9   8   7   7
   length    20 |  14  11  10   9   9
   (bytes)   24 |  19  15  12  11  11
             28 |  24  19  16  13  13
             32 |  29  23  19  17  14

   this table can be a table of bytes if the key schedule
   code is adjusted accordingly
 */

const word rcon_tab[29] =
{
    w0(01), w0(02), w0(04), w0(08),
    w0(10), w0(20), w0(40), w0(80),
    w0(1b), w0(36), w0(6c), w0(d8),
    w0(ab), w0(4d), w0(9a), w0(2f),
    w0(5e), w0(bc), w0(63), w0(c6),
    w0(97), w0(35), w0(6a), w0(d4),
    w0(b3), w0(7d), w0(fa), w0(ef),
    w0(c5)
};

#undef  w0

 /*
   used to ensure table is generated in the right format
   depending on the internal byte order required
 */

#if (INTERNAL_BYTE_ORDER == AES_LITTLE_ENDIAN)
#define r0(p,q,r,s) 0x##p##q##r##s
#define r1(p,q,r,s) 0x##q##r##s##p
#define r2(p,q,r,s) 0x##r##s##p##q
#define r3(p,q,r,s) 0x##s##p##q##r
#define w0(p)          0x000000##p
#define w1(p)        0x0000##p##00
#define w2(p)        0x00##p##0000
#define w3(p)        0x##p##000000
#else
#define r0(p,q,r,s) 0x##s##r##q##p
#define r1(p,q,r,s) 0x##p##s##r##q
#define r2(p,q,r,s) 0x##q##p##s##r
#define r3(p,q,r,s) 0x##r##q##p##s
#define w0(p)        0x##p##000000
#define w1(p)        0x00##p##0000
#define w2(p)        0x0000##p##00
#define w3(p)          0x000000##p
#endif

 /*
   used to ensure table is generated in the right format
   depending on the internal byte order required
 */

#if defined(FIXED_TABLES) && (defined(ONE_TABLE) || defined(FOUR_TABLES)) 

/*  data for forward tables (other than last round) */

#define f_table     r(a5,63,63,c6), r(84,7c,7c,f8), r(99,77,77,ee), r(8d,7b,7b,f6),    r(0d,f2,f2,ff), r(bd,6b,6b,d6), r(b1,6f,6f,de), r(54,c5,c5,91),    r(50,30,30,60), r(03,01,01,02), r(a9,67,67,ce), r(7d,2b,2b,56),    r(19,fe,fe,e7), r(62,d7,d7,b5), r(e6,ab,ab,4d), r(9a,76,76,ec),    r(45,ca,ca,8f), r(9d,82,82,1f), r(40,c9,c9,89), r(87,7d,7d,fa),    r(15,fa,fa,ef), r(eb,59,59,b2), r(c9,47,47,8e), r(0b,f0,f0,fb),    r(ec,ad,ad,41), r(67,d4,d4,b3), r(fd,a2,a2,5f), r(ea,af,af,45),    r(bf,9c,9c,23), r(f7,a4,a4,53), r(96,72,72,e4), r(5b,c0,c0,9b),    r(c2,b7,b7,75), r(1c,fd,fd,e1), r(ae,93,93,3d), r(6a,26,26,4c),    r(5a,36,36,6c), r(41,3f,3f,7e), r(02,f7,f7,f5), r(4f,cc,cc,83),    r(5c,34,34,68), r(f4,a5,a5,51), r(34,e5,e5,d1), r(08,f1,f1,f9),    r(93,71,71,e2), r(73,d8,d8,ab), r(53,31,31,62), r(3f,15,15,2a),    r(0c,04,04,08), r(52,c7,c7,95), r(65,23,23,46), r(5e,c3,c3,9d),    r(28,18,18,30), r(a1,96,96,37), r(0f,05,05,0a), r(b5,9a,9a,2f),    r(09,07,07,0e), r(36,12,12,24), r(9b,80,80,1b), r(3d,e2,e2,df),    r(26,eb,eb,cd), r(69,27,27,4e), r(cd,b2,b2,7f), r(9f,75,75,ea),    r(1b,09,09,12), r(9e,83,83,1d), r(74,2c,2c,58), r(2e,1a,1a,34),    r(2d,1b,1b,36), r(b2,6e,6e,dc), r(ee,5a,5a,b4), r(fb,a0,a0,5b),    r(f6,52,52,a4), r(4d,3b,3b,76), r(61,d6,d6,b7), r(ce,b3,b3,7d),    r(7b,29,29,52), r(3e,e3,e3,dd), r(71,2f,2f,5e), r(97,84,84,13),    r(f5,53,53,a6), r(68,d1,d1,b9), r(00,00,00,00), r(2c,ed,ed,c1),    r(60,20,20,40), r(1f,fc,fc,e3), r(c8,b1,b1,79), r(ed,5b,5b,b6),    r(be,6a,6a,d4), r(46,cb,cb,8d), r(d9,be,be,67), r(4b,39,39,72),    r(de,4a,4a,94), r(d4,4c,4c,98), r(e8,58,58,b0), r(4a,cf,cf,85),    r(6b,d0,d0,bb), r(2a,ef,ef,c5), r(e5,aa,aa,4f), r(16,fb,fb,ed),    r(c5,43,43,86), r(d7,4d,4d,9a), r(55,33,33,66), r(94,85,85,11),    r(cf,45,45,8a), r(10,f9,f9,e9), r(06,02,02,04), r(81,7f,7f,fe),    r(f0,50,50,a0), r(44,3c,3c,78), r(ba,9f,9f,25), r(e3,a8,a8,4b),    r(f3,51,51,a2), r(fe,a3,a3,5d), r(c0,40,40,80), r(8a,8f,8f,05),    r(ad,92,92,3f), r(bc,9d,9d,21), r(48,38,38,70), r(04,f5,f5,f1),    r(df,bc,bc,63), r(c1,b6,b6,77), r(75,da,da,af), r(63,21,21,42),    r(30,10,10,20), r(1a,ff,ff,e5), r(0e,f3,f3,fd), r(6d,d2,d2,bf),    r(4c,cd,cd,81), r(14,0c,0c,18), r(35,13,13,26), r(2f,ec,ec,c3),    r(e1,5f,5f,be), r(a2,97,97,35), r(cc,44,44,88), r(39,17,17,2e),    r(57,c4,c4,93), r(f2,a7,a7,55), r(82,7e,7e,fc), r(47,3d,3d,7a),    r(ac,64,64,c8), r(e7,5d,5d,ba), r(2b,19,19,32), r(95,73,73,e6),    r(a0,60,60,c0), r(98,81,81,19), r(d1,4f,4f,9e), r(7f,dc,dc,a3),    r(66,22,22,44), r(7e,2a,2a,54), r(ab,90,90,3b), r(83,88,88,0b),    r(ca,46,46,8c), r(29,ee,ee,c7), r(d3,b8,b8,6b), r(3c,14,14,28),    r(79,de,de,a7), r(e2,5e,5e,bc), r(1d,0b,0b,16), r(76,db,db,ad),    r(3b,e0,e0,db), r(56,32,32,64), r(4e,3a,3a,74), r(1e,0a,0a,14),    r(db,49,49,92), r(0a,06,06,0c), r(6c,24,24,48), r(e4,5c,5c,b8),    r(5d,c2,c2,9f), r(6e,d3,d3,bd), r(ef,ac,ac,43), r(a6,62,62,c4),    r(a8,91,91,39), r(a4,95,95,31), r(37,e4,e4,d3), r(8b,79,79,f2),    r(32,e7,e7,d5), r(43,c8,c8,8b), r(59,37,37,6e), r(b7,6d,6d,da),    r(8c,8d,8d,01), r(64,d5,d5,b1), r(d2,4e,4e,9c), r(e0,a9,a9,49),    r(b4,6c,6c,d8), r(fa,56,56,ac), r(07,f4,f4,f3), r(25,ea,ea,cf),    r(af,65,65,ca), r(8e,7a,7a,f4), r(e9,ae,ae,47), r(18,08,08,10),    r(d5,ba,ba,6f), r(88,78,78,f0), r(6f,25,25,4a), r(72,2e,2e,5c),    r(24,1c,1c,38), r(f1,a6,a6,57), r(c7,b4,b4,73), r(51,c6,c6,97),    r(23,e8,e8,cb), r(7c,dd,dd,a1), r(9c,74,74,e8), r(21,1f,1f,3e),    r(dd,4b,4b,96), r(dc,bd,bd,61), r(86,8b,8b,0d), r(85,8a,8a,0f),    r(90,70,70,e0), r(42,3e,3e,7c), r(c4,b5,b5,71), r(aa,66,66,cc),    r(d8,48,48,90), r(05,03,03,06), r(01,f6,f6,f7), r(12,0e,0e,1c),    r(a3,61,61,c2), r(5f,35,35,6a), r(f9,57,57,ae), r(d0,b9,b9,69),    r(91,86,86,17), r(58,c1,c1,99), r(27,1d,1d,3a), r(b9,9e,9e,27),    r(38,e1,e1,d9), r(13,f8,f8,eb), r(b3,98,98,2b), r(33,11,11,22),    r(bb,69,69,d2), r(70,d9,d9,a9), r(89,8e,8e,07), r(a7,94,94,33),    r(b6,9b,9b,2d), r(22,1e,1e,3c), r(92,87,87,15), r(20,e9,e9,c9),    r(49,ce,ce,87), r(ff,55,55,aa), r(78,28,28,50), r(7a,df,df,a5),    r(8f,8c,8c,03), r(f8,a1,a1,59), r(80,89,89,09), r(17,0d,0d,1a),    r(da,bf,bf,65), r(31,e6,e6,d7), r(c6,42,42,84), r(b8,68,68,d0),    r(c3,41,41,82), r(b0,99,99,29), r(77,2d,2d,5a), r(11,0f,0f,1e),    r(cb,b0,b0,7b), r(fc,54,54,a8), r(d6,bb,bb,6d), r(3a,16,16,2c)

/*  data for inverse tables (other than last round) */

#define i_table     r(50,a7,f4,51), r(53,65,41,7e), r(c3,a4,17,1a), r(96,5e,27,3a),    r(cb,6b,ab,3b), r(f1,45,9d,1f), r(ab,58,fa,ac), r(93,03,e3,4b),    r(55,fa,30,20), r(f6,6d,76,ad), r(91,76,cc,88), r(25,4c,02,f5),    r(fc,d7,e5,4f), r(d7,cb,2a,c5), r(80,44,35,26), r(8f,a3,62,b5),    r(49,5a,b1,de), r(67,1b,ba,25), r(98,0e,ea,45), r(e1,c0,fe,5d),    r(02,75,2f,c3), r(12,f0,4c,81), r(a3,97,46,8d), r(c6,f9,d3,6b),    r(e7,5f,8f,03), r(95,9c,92,15), r(eb,7a,6d,bf), r(da,59,52,95),    r(2d,83,be,d4), r(d3,21,74,58), r(29,69,e0,49), r(44,c8,c9,8e),    r(6a,89,c2,75), r(78,79,8e,f4), r(6b,3e,58,99), r(dd,71,b9,27),    r(b6,4f,e1,be), r(17,ad,88,f0), r(66,ac,20,c9), r(b4,3a,ce,7d),    r(18,4a,df,63), r(82,31,1a,e5), r(60,33,51,97), r(45,7f,53,62),    r(e0,77,64,b1), r(84,ae,6b,bb), r(1c,a0,81,fe), r(94,2b,08,f9),    r(58,68,48,70), r(19,fd,45,8f), r(87,6c,de,94), r(b7,f8,7b,52),    r(23,d3,73,ab), r(e2,02,4b,72), r(57,8f,1f,e3), r(2a,ab,55,66),    r(07,28,eb,b2), r(03,c2,b5,2f), r(9a,7b,c5,86), r(a5,08,37,d3),    r(f2,87,28,30), r(b2,a5,bf,23), r(ba,6a,03,02), r(5c,82,16,ed),    r(2b,1c,cf,8a), r(92,b4,79,a7), r(f0,f2,07,f3), r(a1,e2,69,4e),    r(cd,f4,da,65), r(d5,be,05,06), r(1f,62,34,d1), r(8a,fe,a6,c4),    r(9d,53,2e,34), r(a0,55,f3,a2), r(32,e1,8a,05), r(75,eb,f6,a4),    r(39,ec,83,0b), r(aa,ef,60,40), r(06,9f,71,5e), r(51,10,6e,bd),    r(f9,8a,21,3e), r(3d,06,dd,96), r(ae,05,3e,dd), r(46,bd,e6,4d),    r(b5,8d,54,91), r(05,5d,c4,71), r(6f,d4,06,04), r(ff,15,50,60),    r(24,fb,98,19), r(97,e9,bd,d6), r(cc,43,40,89), r(77,9e,d9,67),    r(bd,42,e8,b0), r(88,8b,89,07), r(38,5b,19,e7), r(db,ee,c8,79),    r(47,0a,7c,a1), r(e9,0f,42,7c), r(c9,1e,84,f8), r(00,00,00,00),    r(83,86,80,09), r(48,ed,2b,32), r(ac,70,11,1e), r(4e,72,5a,6c),    r(fb,ff,0e,fd), r(56,38,85,0f), r(1e,d5,ae,3d), r(27,39,2d,36),    r(64,d9,0f,0a), r(21,a6,5c,68), r(d1,54,5b,9b), r(3a,2e,36,24),    r(b1,67,0a,0c), r(0f,e7,57,93), r(d2,96,ee,b4), r(9e,91,9b,1b),    r(4f,c5,c0,80), r(a2,20,dc,61), r(69,4b,77,5a), r(16,1a,12,1c),    r(0a,ba,93,e2), r(e5,2a,a0,c0), r(43,e0,22,3c), r(1d,17,1b,12),    r(0b,0d,09,0e), r(ad,c7,8b,f2), r(b9,a8,b6,2d), r(c8,a9,1e,14),    r(85,19,f1,57), r(4c,07,75,af), r(bb,dd,99,ee), r(fd,60,7f,a3),    r(9f,26,01,f7), r(bc,f5,72,5c), r(c5,3b,66,44), r(34,7e,fb,5b),    r(76,29,43,8b), r(dc,c6,23,cb), r(68,fc,ed,b6), r(63,f1,e4,b8),    r(ca,dc,31,d7), r(10,85,63,42), r(40,22,97,13), r(20,11,c6,84),    r(7d,24,4a,85), r(f8,3d,bb,d2), r(11,32,f9,ae), r(6d,a1,29,c7),    r(4b,2f,9e,1d), r(f3,30,b2,dc), r(ec,52,86,0d), r(d0,e3,c1,77),    r(6c,16,b3,2b), r(99,b9,70,a9), r(fa,48,94,11), r(22,64,e9,47),    r(c4,8c,fc,a8), r(1a,3f,f0,a0), r(d8,2c,7d,56), r(ef,90,33,22),    r(c7,4e,49,87), r(c1,d1,38,d9), r(fe,a2,ca,8c), r(36,0b,d4,98),    r(cf,81,f5,a6), r(28,de,7a,a5), r(26,8e,b7,da), r(a4,bf,ad,3f),    r(e4,9d,3a,2c), r(0d,92,78,50), r(9b,cc,5f,6a), r(62,46,7e,54),    r(c2,13,8d,f6), r(e8,b8,d8,90), r(5e,f7,39,2e), r(f5,af,c3,82),    r(be,80,5d,9f), r(7c,93,d0,69), r(a9,2d,d5,6f), r(b3,12,25,cf),    r(3b,99,ac,c8), r(a7,7d,18,10), r(6e,63,9c,e8), r(7b,bb,3b,db),    r(09,78,26,cd), r(f4,18,59,6e), r(01,b7,9a,ec), r(a8,9a,4f,83),    r(65,6e,95,e6), r(7e,e6,ff,aa), r(08,cf,bc,21), r(e6,e8,15,ef),    r(d9,9b,e7,ba), r(ce,36,6f,4a), r(d4,09,9f,ea), r(d6,7c,b0,29),    r(af,b2,a4,31), r(31,23,3f,2a), r(30,94,a5,c6), r(c0,66,a2,35),    r(37,bc,4e,74), r(a6,ca,82,fc), r(b0,d0,90,e0), r(15,d8,a7,33),    r(4a,98,04,f1), r(f7,da,ec,41), r(0e,50,cd,7f), r(2f,f6,91,17),    r(8d,d6,4d,76), r(4d,b0,ef,43), r(54,4d,aa,cc), r(df,04,96,e4),    r(e3,b5,d1,9e), r(1b,88,6a,4c), r(b8,1f,2c,c1), r(7f,51,65,46),    r(04,ea,5e,9d), r(5d,35,8c,01), r(73,74,87,fa), r(2e,41,0b,fb),    r(5a,1d,67,b3), r(52,d2,db,92), r(33,56,10,e9), r(13,47,d6,6d),    r(8c,61,d7,9a), r(7a,0c,a1,37), r(8e,14,f8,59), r(89,3c,13,eb),    r(ee,27,a9,ce), r(35,c9,61,b7), r(ed,e5,1c,e1), r(3c,b1,47,7a),    r(59,df,d2,9c), r(3f,73,f2,55), r(79,ce,14,18), r(bf,37,c7,73),    r(ea,cd,f7,53), r(5b,aa,fd,5f), r(14,6f,3d,df), r(86,db,44,78),    r(81,f3,af,ca), r(3e,c4,68,b9), r(2c,34,24,38), r(5f,40,a3,c2),    r(72,c3,1d,16), r(0c,25,e2,bc), r(8b,49,3c,28), r(41,95,0d,ff),    r(71,01,a8,39), r(de,b3,0c,08), r(9c,e4,b4,d8), r(90,c1,56,64),    r(61,84,cb,7b), r(70,b6,32,d5), r(74,5c,6c,48), r(42,57,b8,d0)

/* generate the required tables in the desired endian format */

#undef  r
#define r   r0

#if defined(ONE_TABLE)
const word ft_tab[256] =
    {   f_table };
#elif defined(FOUR_TABLES)
const word ft_tab[4][256] =
{   {   f_table },
#undef  r
#define r   r1
    {   f_table },
#undef  r
#define r   r2
    {   f_table },
#undef  r
#define r   r3
    {   f_table }
};
#endif

#undef  r
#define r   r0
#if defined(ONE_TABLE)
const word it_tab[256] =
    {   i_table };
#elif defined(FOUR_TABLES)
const word it_tab[4][256] =
{   {   i_table },
#undef  r
#define r   r1
    {   i_table },
#undef  r
#define r   r2
    {   i_table },
#undef  r
#define r   r3
    {   i_table }
};
#endif

#endif

#if defined(FIXED_TABLES) && (defined(ONE_LR_TABLE) || defined(FOUR_LR_TABLES)) 

/*  data for inverse tables (last round) */

#define li_table        w(52), w(09), w(6a), w(d5), w(30), w(36), w(a5), w(38),    w(bf), w(40), w(a3), w(9e), w(81), w(f3), w(d7), w(fb),    w(7c), w(e3), w(39), w(82), w(9b), w(2f), w(ff), w(87),    w(34), w(8e), w(43), w(44), w(c4), w(de), w(e9), w(cb),    w(54), w(7b), w(94), w(32), w(a6), w(c2), w(23), w(3d),    w(ee), w(4c), w(95), w(0b), w(42), w(fa), w(c3), w(4e),    w(08), w(2e), w(a1), w(66), w(28), w(d9), w(24), w(b2),    w(76), w(5b), w(a2), w(49), w(6d), w(8b), w(d1), w(25),    w(72), w(f8), w(f6), w(64), w(86), w(68), w(98), w(16),    w(d4), w(a4), w(5c), w(cc), w(5d), w(65), w(b6), w(92),    w(6c), w(70), w(48), w(50), w(fd), w(ed), w(b9), w(da),    w(5e), w(15), w(46), w(57), w(a7), w(8d), w(9d), w(84),    w(90), w(d8), w(ab), w(00), w(8c), w(bc), w(d3), w(0a),    w(f7), w(e4), w(58), w(05), w(b8), w(b3), w(45), w(06),    w(d0), w(2c), w(1e), w(8f), w(ca), w(3f), w(0f), w(02),    w(c1), w(af), w(bd), w(03), w(01), w(13), w(8a), w(6b),    w(3a), w(91), w(11), w(41), w(4f), w(67), w(dc), w(ea),    w(97), w(f2), w(cf), w(ce), w(f0), w(b4), w(e6), w(73),    w(96), w(ac), w(74), w(22), w(e7), w(ad), w(35), w(85),    w(e2), w(f9), w(37), w(e8), w(1c), w(75), w(df), w(6e),    w(47), w(f1), w(1a), w(71), w(1d), w(29), w(c5), w(89),    w(6f), w(b7), w(62), w(0e), w(aa), w(18), w(be), w(1b),    w(fc), w(56), w(3e), w(4b), w(c6), w(d2), w(79), w(20),    w(9a), w(db), w(c0), w(fe), w(78), w(cd), w(5a), w(f4),    w(1f), w(dd), w(a8), w(33), w(88), w(07), w(c7), w(31),    w(b1), w(12), w(10), w(59), w(27), w(80), w(ec), w(5f),    w(60), w(51), w(7f), w(a9), w(19), w(b5), w(4a), w(0d),    w(2d), w(e5), w(7a), w(9f), w(93), w(c9), w(9c), w(ef),    w(a0), w(e0), w(3b), w(4d), w(ae), w(2a), w(f5), w(b0),    w(c8), w(eb), w(bb), w(3c), w(83), w(53), w(99), w(61),    w(17), w(2b), w(04), w(7e), w(ba), w(77), w(d6), w(26),    w(e1), w(69), w(14), w(63), w(55), w(21), w(0c), w(7d),

/* generate the required tables in the desired endian format */

#undef  r
#define r(p,q,r,s)  w0(q)
#if defined(ONE_LR_TABLE)
const word fl_tab[256] =
    {   f_table     };
#elif defined(FOUR_LR_TABLES)
const word fl_tab[4][256] =
{   {   f_table    },
#undef  r
#define r(p,q,r,s)   w1(q)
    {   f_table    },
#undef  r
#define r(p,q,r,s)   w2(q)
    {   f_table    },
#undef  r
#define r(p,q,r,s)   w3(q)
    {   f_table    }
};
#endif

#undef  w
#define w   w0
#if defined(ONE_LR_TABLE)
const word il_tab[256] =
    {   li_table    };
#elif defined(FOUR_LR_TABLES)
const word il_tab[4][256] =
{   {   li_table    },
#undef  w
#define w   w1
    {   li_table    },
#undef  w
#define w   w2
    {   li_table    },
#undef  w
#define w   w3
    {   li_table    }
};
#endif

#endif

#if defined(FIXED_TABLES) && (defined(ONE_IM_TABLE) || defined(FOUR_IM_TABLES)) 

#define m_table     r(00,00,00,00), r(0b,0d,09,0e), r(16,1a,12,1c), r(1d,17,1b,12),    r(2c,34,24,38), r(27,39,2d,36), r(3a,2e,36,24), r(31,23,3f,2a),    r(58,68,48,70), r(53,65,41,7e), r(4e,72,5a,6c), r(45,7f,53,62),    r(74,5c,6c,48), r(7f,51,65,46), r(62,46,7e,54), r(69,4b,77,5a),    r(b0,d0,90,e0), r(bb,dd,99,ee), r(a6,ca,82,fc), r(ad,c7,8b,f2),    r(9c,e4,b4,d8), r(97,e9,bd,d6), r(8a,fe,a6,c4), r(81,f3,af,ca),    r(e8,b8,d8,90), r(e3,b5,d1,9e), r(fe,a2,ca,8c), r(f5,af,c3,82),    r(c4,8c,fc,a8), r(cf,81,f5,a6), r(d2,96,ee,b4), r(d9,9b,e7,ba),    r(7b,bb,3b,db), r(70,b6,32,d5), r(6d,a1,29,c7), r(66,ac,20,c9),    r(57,8f,1f,e3), r(5c,82,16,ed), r(41,95,0d,ff), r(4a,98,04,f1),    r(23,d3,73,ab), r(28,de,7a,a5), r(35,c9,61,b7), r(3e,c4,68,b9),    r(0f,e7,57,93), r(04,ea,5e,9d), r(19,fd,45,8f), r(12,f0,4c,81),    r(cb,6b,ab,3b), r(c0,66,a2,35), r(dd,71,b9,27), r(d6,7c,b0,29),    r(e7,5f,8f,03), r(ec,52,86,0d), r(f1,45,9d,1f), r(fa,48,94,11),    r(93,03,e3,4b), r(98,0e,ea,45), r(85,19,f1,57), r(8e,14,f8,59),    r(bf,37,c7,73), r(b4,3a,ce,7d), r(a9,2d,d5,6f), r(a2,20,dc,61),    r(f6,6d,76,ad), r(fd,60,7f,a3), r(e0,77,64,b1), r(eb,7a,6d,bf),    r(da,59,52,95), r(d1,54,5b,9b), r(cc,43,40,89), r(c7,4e,49,87),    r(ae,05,3e,dd), r(a5,08,37,d3), r(b8,1f,2c,c1), r(b3,12,25,cf),    r(82,31,1a,e5), r(89,3c,13,eb), r(94,2b,08,f9), r(9f,26,01,f7),    r(46,bd,e6,4d), r(4d,b0,ef,43), r(50,a7,f4,51), r(5b,aa,fd,5f),    r(6a,89,c2,75), r(61,84,cb,7b), r(7c,93,d0,69), r(77,9e,d9,67),    r(1e,d5,ae,3d), r(15,d8,a7,33), r(08,cf,bc,21), r(03,c2,b5,2f),    r(32,e1,8a,05), r(39,ec,83,0b), r(24,fb,98,19), r(2f,f6,91,17),    r(8d,d6,4d,76), r(86,db,44,78), r(9b,cc,5f,6a), r(90,c1,56,64),    r(a1,e2,69,4e), r(aa,ef,60,40), r(b7,f8,7b,52), r(bc,f5,72,5c),    r(d5,be,05,06), r(de,b3,0c,08), r(c3,a4,17,1a), r(c8,a9,1e,14),    r(f9,8a,21,3e), r(f2,87,28,30), r(ef,90,33,22), r(e4,9d,3a,2c),    r(3d,06,dd,96), r(36,0b,d4,98), r(2b,1c,cf,8a), r(20,11,c6,84),    r(11,32,f9,ae), r(1a,3f,f0,a0), r(07,28,eb,b2), r(0c,25,e2,bc),    r(65,6e,95,e6), r(6e,63,9c,e8), r(73,74,87,fa), r(78,79,8e,f4),    r(49,5a,b1,de), r(42,57,b8,d0), r(5f,40,a3,c2), r(54,4d,aa,cc),    r(f7,da,ec,41), r(fc,d7,e5,4f), r(e1,c0,fe,5d), r(ea,cd,f7,53),    r(db,ee,c8,79), r(d0,e3,c1,77), r(cd,f4,da,65), r(c6,f9,d3,6b),    r(af,b2,a4,31), r(a4,bf,ad,3f), r(b9,a8,b6,2d), r(b2,a5,bf,23),    r(83,86,80,09), r(88,8b,89,07), r(95,9c,92,15), r(9e,91,9b,1b),    r(47,0a,7c,a1), r(4c,07,75,af), r(51,10,6e,bd), r(5a,1d,67,b3),    r(6b,3e,58,99), r(60,33,51,97), r(7d,24,4a,85), r(76,29,43,8b),    r(1f,62,34,d1), r(14,6f,3d,df), r(09,78,26,cd), r(02,75,2f,c3),    r(33,56,10,e9), r(38,5b,19,e7), r(25,4c,02,f5), r(2e,41,0b,fb),    r(8c,61,d7,9a), r(87,6c,de,94), r(9a,7b,c5,86), r(91,76,cc,88),    r(a0,55,f3,a2), r(ab,58,fa,ac), r(b6,4f,e1,be), r(bd,42,e8,b0),    r(d4,09,9f,ea), r(df,04,96,e4), r(c2,13,8d,f6), r(c9,1e,84,f8),    r(f8,3d,bb,d2), r(f3,30,b2,dc), r(ee,27,a9,ce), r(e5,2a,a0,c0),    r(3c,b1,47,7a), r(37,bc,4e,74), r(2a,ab,55,66), r(21,a6,5c,68),    r(10,85,63,42), r(1b,88,6a,4c), r(06,9f,71,5e), r(0d,92,78,50),    r(64,d9,0f,0a), r(6f,d4,06,04), r(72,c3,1d,16), r(79,ce,14,18),    r(48,ed,2b,32), r(43,e0,22,3c), r(5e,f7,39,2e), r(55,fa,30,20),    r(01,b7,9a,ec), r(0a,ba,93,e2), r(17,ad,88,f0), r(1c,a0,81,fe),    r(2d,83,be,d4), r(26,8e,b7,da), r(3b,99,ac,c8), r(30,94,a5,c6),    r(59,df,d2,9c), r(52,d2,db,92), r(4f,c5,c0,80), r(44,c8,c9,8e),    r(75,eb,f6,a4), r(7e,e6,ff,aa), r(63,f1,e4,b8), r(68,fc,ed,b6),    r(b1,67,0a,0c), r(ba,6a,03,02), r(a7,7d,18,10), r(ac,70,11,1e),    r(9d,53,2e,34), r(96,5e,27,3a), r(8b,49,3c,28), r(80,44,35,26),    r(e9,0f,42,7c), r(e2,02,4b,72), r(ff,15,50,60), r(f4,18,59,6e),    r(c5,3b,66,44), r(ce,36,6f,4a), r(d3,21,74,58), r(d8,2c,7d,56),    r(7a,0c,a1,37), r(71,01,a8,39), r(6c,16,b3,2b), r(67,1b,ba,25),    r(56,38,85,0f), r(5d,35,8c,01), r(40,22,97,13), r(4b,2f,9e,1d),    r(22,64,e9,47), r(29,69,e0,49), r(34,7e,fb,5b), r(3f,73,f2,55),    r(0e,50,cd,7f), r(05,5d,c4,71), r(18,4a,df,63), r(13,47,d6,6d),    r(ca,dc,31,d7), r(c1,d1,38,d9), r(dc,c6,23,cb), r(d7,cb,2a,c5),    r(e6,e8,15,ef), r(ed,e5,1c,e1), r(f0,f2,07,f3), r(fb,ff,0e,fd),    r(92,b4,79,a7), r(99,b9,70,a9), r(84,ae,6b,bb), r(8f,a3,62,b5),    r(be,80,5d,9f), r(b5,8d,54,91), r(a8,9a,4f,83), r(a3,97,46,8d)

#undef r
#define r   r0

#if defined(ONE_IM_TABLE)
const word im_tab[256] =
    {   m_table };
#elif defined(FOUR_IM_TABLES)
const word im_tab[4][256] =
{   {   m_table },
#undef  r
#define r   r1
    {   m_table },
#undef  r
#define r   r2
    {   m_table },
#undef  r
#define r   r3
    {   m_table }
};
#endif

#endif
