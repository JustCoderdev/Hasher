/*
 * JustCoderdev's Secure Hash Standard library v1
 */

/* RFC3174: <https://www.rfc-editor.org/rfc/rfc3174> */

#ifndef SHS_H_
#define SHS_H_

#define CORE_STR_VER 1

#include <core.h>


/* Types
 * ----------------------------------------------------------- */

#define SHS_WORD32_SIZE 4
#define SHS_WORD64_SIZE 8

typedef n32 SHS_Word32;
typedef n64 SHS_Word64;


#define SHS_BLOCK_WORDS_COUNT 16

typedef union SHS_Block512 {
	SHS_Word32 words[SHS_BLOCK_WORDS_COUNT];
	struct {
/* 4444 16B */	SHS_Word32 w0, w1, w2, w3;
/* 4444 16B */	SHS_Word32 w4, w5, w6, w7;
/* 4444 16B */	SHS_Word32 w8, w9, wA, wB;
/* 4444 16B */	SHS_Word32 wC, wD, wE, wF;
	} as;
} SHS_Block512;

typedef union Block1024 {
	SHS_Word64 words[SHS_BLOCK_WORDS_COUNT];
	struct {
/* 8888 32B */	SHS_Word64 W0, W1, W2, W3;
/* 8888 32B */	SHS_Word64 W4, W5, W6, W7;
/* 8888 32B */	SHS_Word64 W8, W9, WA, WB;
/* 8888 32B */	SHS_Word64 WC, WD, WE, WF;
	} as;
} Block1024;


typedef struct {
/* -------8 */ SHS_Block512* items;
/* -------8 */ n64 count;
} SHS_Block512_List;

typedef struct {
/* -------8 */ Block1024* items;
/* -------8 */ n64 count;
} SHS_Block1024_List;


typedef enum SHA_Digest_Size {
	SHS_DS160 = 20,
	SHS_DS224 = 28,
	SHS_DS256 = 32,
	SHS_DS384 = 48,
	SHS_DS512 = 64
} SHS_Digest_Size;

typedef struct { n8 byte[SHS_DS160]; } SHS_digest160;
typedef struct { n8 byte[SHS_DS224]; } SHS_digest224;
typedef struct { n8 byte[SHS_DS256]; } SHS_digest256;
typedef struct { n8 byte[SHS_DS384]; } SHS_digest384;
typedef struct { n8 byte[SHS_DS512]; } SHS_digest512;


/* Create and free Blocks (m is Block size, 512 or 1024)
 * ----------------------------------------------------------- */

extern SHS_Block512_List  SHS_block512_create_list_from_file(FILE* file);
extern SHS_Block1024_List SHS_block1024_create_list_from_file(FILE* file);

extern void SHS_block512_List_free(SHS_Block512_List* blocks);
extern void SHS_block1024_List_free(SHS_Block1024_List* blocks);


/* Operate on Words (w is Word size, 32 or 64)
 * ----------------------------------------------------------- */

extern SHS_Word32 SHS_Word32_ROTL(SHS_Word32 word, n8 n);
extern SHS_Word32 SHS_Word32_ROTR(SHS_Word32 word, n8 n);

extern SHS_Word64 SHS_Word64_ROTL(SHS_Word64 word, n8 n);
extern SHS_Word64 SHS_Word64_ROTR(SHS_Word64 word, n8 n);


/* Generate digest
 * ----------------------------------------------------------- */

extern SHS_digest160 SHS_SHA1_generate_digest(SHS_Block512_List blocks);
extern SHS_digest224 SHS_SHA224_generate_digest(SHS_Block512_List blocks);
extern SHS_digest256 SHS_SHA256_generate_digest(SHS_Block512_List blocks);

extern SHS_digest384 SHS_SHA384_generate_digest(SHS_Block1024_List blocks);
extern SHS_digest512 SHS_SHA512_generate_digest(SHS_Block1024_List blocks);

extern SHS_digest224 SHS_SHA512_224_generate_digest(SHS_Block1024_List blocks);
extern SHS_digest256 SHS_SHA512_256_generate_digest(SHS_Block1024_List blocks);


/* Operate on Digests
 * ----------------------------------------------------------- */

#define SHS_D160_FMT "%#010x%08x%08x%08x%08x"
#define SHS_D160(D) \
	(D).byte[ 0] << 24 | (D).byte[ 1] << 16 | (D).byte[ 2] << 8 | (D).byte[ 3], \
	(D).byte[ 4] << 24 | (D).byte[ 5] << 16 | (D).byte[ 6] << 8 | (D).byte[ 7], \
	(D).byte[ 8] << 24 | (D).byte[ 9] << 16 | (D).byte[10] << 8 | (D).byte[11], \
	(D).byte[12] << 24 | (D).byte[13] << 16 | (D).byte[14] << 8 | (D).byte[15], \
	(D).byte[16] << 24 | (D).byte[17] << 16 | (D).byte[18] << 8 | (D).byte[19]

extern bool SHS_digest_compare(SHS_Digest_Size size, const n8* dA, const n8* dB);

extern void SHS_digest_from_Word32(SHS_Digest_Size dsize, n8* digest,
		n8 hcount, const SHS_Word32* hash);
extern void SHS_digest_from_Word64(SHS_Digest_Size dsize, n8* digest,
		n8 hcount, const SHS_Word64* hash);


#endif /* SHS_H_ */
