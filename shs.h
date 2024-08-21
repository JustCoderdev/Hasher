/* Secure Hash Standard module for JustCoderdev Core library v1
 * */

/*
 * Standard followed RFC3174
 * source: <https://www.rfc-editor.org/rfc/rfc3174>
 * */

#ifndef SHS_H_
#define SHS_H_

#include <core.h>


#define WORD32_SIZE 4
#define WORD64_SIZE 8

typedef n32 Word32;
typedef n64 Word64;


#define BLOCK_WORDS_COUNT 16

typedef union Block512 {
	Word32 words[BLOCK_WORDS_COUNT];
	struct {
/* 4444 16B */	Word32 w0, w1, w2, w3;
/* 4444 16B */	Word32 w4, w5, w6, w7;
/* 4444 16B */	Word32 w8, w9, wA, wB;
/* 4444 16B */	Word32 wC, wD, wE, wF;
	} as;
} Block512;

typedef union Block1024 {
	Word64 words[BLOCK_WORDS_COUNT];
	struct {
/* 8888 32B */	Word64 W0, W1, W2, W3;
/* 8888 32B */	Word64 W4, W5, W6, W7;
/* 8888 32B */	Word64 W8, W9, WA, WB;
/* 8888 32B */	Word64 WC, WD, WE, WF;
	} as;
} Block1024;


typedef struct {
/* -------8 */ Block512* items;
/* -------8 */ n64 count;
} Block512_List;

typedef struct {
/* -------8 */ Block1024* items;
/* -------8 */ n64 count;
} Block1024_List;


typedef enum SHA_Digest_Size {
	SHS_DS160 = 20,
	SHS_DS224 = 28,
	SHS_DS256 = 32,
	SHS_DS384 = 48,
	SHS_DS512 = 64
} SHS_Digest_Size;

typedef struct { n8 byte[SHS_DS160]; } digest160;
typedef struct { n8 byte[SHS_DS224]; } digest224;
typedef struct { n8 byte[SHS_DS256]; } digest256;
typedef struct { n8 byte[SHS_DS384]; } digest384;
typedef struct { n8 byte[SHS_DS512]; } digest512;


/* Create and free Blocks (m is Block size, 512 or 1024)
 * ----------------------------------------------------------- */

extern Block512_List  SHS_block512_create_list_from_file(FILE* file);
extern Block1024_List SHS_block1024_create_list_from_file(FILE* file);

extern void SHS_block512_List_free(Block512_List* blocks);
extern void SHS_block1024_List_free(Block1024_List* blocks);


/* Operate on Words (w is Word size, 32 or 64)
 * ----------------------------------------------------------- */

extern Word32 SHS_Word32_ROTL(Word32 word, n8 n);
extern Word32 SHS_Word32_ROTR(Word32 word, n8 n);

extern Word64 SHS_Word64_ROTL(Word64 word, n8 n);
extern Word64 SHS_Word64_ROTR(Word64 word, n8 n);


/* Generate digest
 * ----------------------------------------------------------- */

extern digest160 SHS_SHA1_generate_digest(Block512_List blocks);
extern digest224 SHS_SHA224_generate_digest(Block512_List blocks);
extern digest256 SHS_SHA256_generate_digest(Block512_List blocks);

extern digest384 SHS_SHA384_generate_digest(Block1024_List blocks);
extern digest512 SHS_SHA512_generate_digest(Block1024_List blocks);

extern digest224 SHS_SHA512_224_generate_digest(Block1024_List blocks);
extern digest256 SHS_SHA512_256_generate_digest(Block1024_List blocks);


/* Operate on Digests
 * ----------------------------------------------------------- */

#define D160_FMT "%#010x%08x%08x%08x%08x"
#define D160(D) \
	(D).byte[ 0] << 24 | (D).byte[ 1] << 16 | (D).byte[ 2] << 8 | (D).byte[ 3], \
	(D).byte[ 4] << 24 | (D).byte[ 5] << 16 | (D).byte[ 6] << 8 | (D).byte[ 7], \
	(D).byte[ 8] << 24 | (D).byte[ 9] << 16 | (D).byte[10] << 8 | (D).byte[11], \
	(D).byte[12] << 24 | (D).byte[13] << 16 | (D).byte[14] << 8 | (D).byte[15], \
	(D).byte[16] << 24 | (D).byte[17] << 16 | (D).byte[18] << 8 | (D).byte[19]

extern bool SHS_digest_compare(SHS_Digest_Size size, const n8* dA, const n8* dB);

extern void SHS_digest_from_Word32(SHS_Digest_Size dsize, n8* digest,
		n8 hcount, const Word32* hash);
extern void SHS_digest_from_Word64(SHS_Digest_Size dsize, n8* digest,
		n8 hcount, const Word64* hash);


#endif /* SHS_H_ */
