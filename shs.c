#define LOG_MODULE "SHS"

#include <core.h>
#include <shs.h>

#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>
int fileno(FILE *stream);

#include <assert.h>


/* Create and free Blocks
 * ----------------------------------------------------------- */

Block512_List SHS_block512_create_list_from_file(FILE* file)
{
#define BLOCK_SIZE 64
	i64 file_size = -1;

/* Get file size */
	if(fseek(file, 0L, SEEK_END) == -1) {
		core_log(CORE_ERROR, "Can't get file size during block creation: %s\n",
				strerror(errno));
		exit(failure);
	}

	file_size = ftell(file);
	if(file_size == -1) {
		core_log(CORE_ERROR, "Can't get file size during block creation: %s\n",
				strerror(errno));
		exit(failure);
	}

	fseek(file, 0L, SEEK_SET);


	{
	/* Read blocks from file */
		n64 i, file_blocks = file_size / BLOCK_SIZE;
		n8 extra_bytes = file_size % BLOCK_SIZE;
		Block512_List blocks = {0};

		/* #error "Debug here, check for 'added bytes'" */
		core_log(CORE_DEBUG,
				"Creating blocks for file of %lu bytes (%lu blocks + %lu extra bytes)\n",
				file_size, file_blocks, extra_bytes);

		if(file_blocks == 0 && extra_bytes == 0) {
			core_log(CORE_ERROR, "File is empty during block creation\n");
			exit(failure);
		}

		blocks.count = file_blocks + (extra_bytes > 0 ? 1 : 0);
		blocks.items = dmalloc(blocks.count * BLOCK_SIZE);
		if(blocks.items == NULL) {
			core_log(CORE_ERROR,
					"Can't mallocate %lu bytes to store blocks: %s\n",
					blocks.count, strerror(errno));
			exit(failure);
		}

		for(i = 0; i < blocks.count; ++i)
		{
			Block512* block = &blocks.items[i];
			n64 j;

			ssize_t read_bytes, request_bytes = BLOCK_SIZE;
			if(i == blocks.count - 1 && extra_bytes > 0)
				request_bytes = extra_bytes;

			read_bytes = fread(block->words, 1, request_bytes, file);
			if(read_bytes < request_bytes) {
				if(ferror(file)) {
					core_log(CORE_ERROR,
							"Can't read from file for block creation: %s\n",
							 strerror(errno));
				} else {
					core_log(CORE_ERROR,
							"Read %lu bytes from file instead of expected %lu during block creation\n",
							 read_bytes, request_bytes);
				}
				exit(failure);
			}

			/* Fix endianess */
			for(j = 0; j < 16; ++j)
			{
				Word32* word = &block->words[j];
				Word32 t = *word;
				*word
					= ((t & 0x000000FF) << 24)
					| ((t & 0x0000FF00) << 8)
					| ((t & 0x00FF0000) >> 8)
					| ((t & 0xFF000000) >> 24);
			}
		}

		if(extra_bytes > 0) {
			n64 j;
			Block512* last_block = &blocks.items[blocks.count - 1];
			assert(extra_bytes < sizeof(Word32) * 16);

			/* write B10000000 to next free byte in word */
			last_block->words[extra_bytes / sizeof(Word32)]
				|= 0x80 << ((sizeof(Word32) - extra_bytes - 1) * 8);

			/* write size to last word */
			last_block->as.wF |= extra_bytes;
		}

		return blocks;

	/* END Read blocks from file END */
	}
}

/* extern Block1024_List SHS_block1024_create_list_from_file(FILE* file); */

void SHS_block512_List_free(Block512_List* blocks) {
	if(blocks->items != NULL) dfree(blocks->items);
	blocks->count = 0;
}

void SHS_block1024_List_free(Block1024_List* blocks) {
	if(blocks->items != NULL) dfree(blocks->items);
	blocks->count = 0;
}


/* Operate on Word (ww is Word size, 32 or 64)
 * ----------------------------------------------------------- */

Word32 SHS_Word32_ROTL(Word32 word, n8 n) {
	assert(n < 32); /* 0 <= n < w */
	return (word << n) | (word >> (32 - n));
}

Word32 SHS_Word32_ROTR(Word32 word, n8 n) {
	assert(n < 32); /* 0 <= n < w */
	return (word >> n) | (word << (32 - n));
}

Word32 SHS_Word32_Add(Word32 wA, Word32 wB) {
	n64 z = (n64)wA + (n64)wB;

	/* Overflow */
	assert(!(z < wA || z < wB || z - wA != wB || z - wB != wA));
	return z % (n32)-1;
}

/* extern Word64 SHS_Word64_ROTL(Word64 word); */
/* extern Word64 SHS_Word64_ROTR(Word64 word); */
/* extern Word64 SHS_Word64_SHR(Word64 word); */
/* extern Word64 SHS_Word64_AMOD(Word64 word); */


/* Generate digest
 * ----------------------------------------------------------- */

static Word32 SHS_SHA1_f(n8 t, Word32 x, Word32 y, Word32 z) {
	assert(t < 80);
	if(t <= 19) return (x & y) ^ (~x & z);
	if(t <= 39) return x ^ y ^ z;
	if(t <= 59) return (x & y) ^ (x & z) ^ (y & z);
	return x ^ y ^ z;
}

static Word32 SHS_SHA1_K(n8 t) {
	assert(t < 80);
	if(t <= 19) return 0x5a827999;
	if(t <= 39) return 0x6ed9eba1;
	if(t <= 59) return 0x8f1bbcdc;
	return 0xca62c1d6;
}

digest160 SHS_SHA1_generate_digest(Block512_List blocks)
{
	Word32 hash[5] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 };
	digest160 digest = {0};

	n64 i, t, j;

	core_log(CORE_DEBUG, "Generating SHA1 hash with %lu blocks\n", blocks.count);

	for(i = 0; i < blocks.count; ++i)
	{
		Block512 block = blocks.items[i]; /* M */

		struct { Word32 a, b, c, d, e; } var = {0};
		Word32 T, W[80] = {0};


		/* 1. Prepare the message schedule */
		for(t = 0; t <= 15; ++t) {
			W[t] = block.words[t];
		}

		for(t = 16; t <= 79; ++t) {
			W[t] = SHS_Word32_ROTL(W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);
		}

		/* core_log(CORE_DEBUG, "[%02lu] Scheduling... \n", i); */


		/* 2. Initialize the five working variables */
		var.a = hash[0];
		var.b = hash[1];
		var.c = hash[2];
		var.d = hash[3];
		var.e = hash[4];

		/* core_log(CORE_DEBUG, "[%02lu] Initialising... a: %#010x, b: %#010x, c: %#010x, d: %#010x, e: %#010x\n", */
		/* 		i, hash[0], hash[1], hash[2], hash[3], hash[4]); */


		/* 3. Compute... */
		for(t = 0; t <= 79; ++t) {
			T = SHS_Word32_ROTL(var.a, 5);
			T = SHS_Word32_Add(T, SHS_SHA1_f(t, var.b, var.c, var.d));
			T = SHS_Word32_Add(T, var.e);
			T = SHS_Word32_Add(T, SHS_SHA1_K(t));
			T = SHS_Word32_Add(T, W[t]);
			var.e = var.d;
			var.d = var.c;
			var.c = SHS_Word32_ROTL(var.b, 30);
			var.b = var.a;
			var.a = T;
		}

		/* core_log(CORE_DEBUG, "[%02lu] Computing...    a: %#010x, b: %#010x, c: %#010x, d: %#010x, e: %#010x\n", */
		/* 		i, var.a, var.b, var.c, var.d, var.e); */

		/* 4. Compute the intermediate hash value */
		hash[0] = SHS_Word32_Add(var.a, hash[0]);
		hash[1] = SHS_Word32_Add(var.b, hash[1]);
		hash[2] = SHS_Word32_Add(var.c, hash[2]);
		hash[3] = SHS_Word32_Add(var.d, hash[3]);
		hash[4] = SHS_Word32_Add(var.e, hash[4]);

		/* core_log(CORE_DEBUG, "[%02lu] Intermediate... a: %#010x, b: %#010x, c: %#010x, d: %#010x, e: %#010x\n\n", */
		/* 		i, hash[0], hash[1], hash[2], hash[3], hash[4]); */
	}

	/* Pack digest */
	SHS_digest_from_Word32(SHS_DS160, digest.byte, 5, hash);

	/* core_log( CORE_DEBUG, */
	/* 		"Generated SHA1, packing: \n" */
	/* 		"      -->   hash %#010x%08x%08x%08x%08x\n" */
	/* 		"      --> digest " D160_FMT "\n", */
	/* 		hash[0], hash[1], hash[2], hash[3], hash[4], */
	/* 		D160(digest) */
	/* ); */

	return digest;
}

/* extern digest224 SHS_SHA224_generate_digest(Block512_List blocks); */
/* extern digest256 SHS_SHA256_generate_digest(Block512_List blocks); */

/* extern digest384 SHS_SHA384_generate_digest(Block1024_List blocks); */
/* extern digest512 SHS_SHA512_generate_digest(Block1024_List blocks); */

/* extern digest224 SHS_SHA512_224_generate_digest(Block1024_List blocks); */
/* extern digest256 SHS_SHA512_256_generate_digest(Block1024_List blocks); */


/* Operate on Digests
 * ----------------------------------------------------------- */

bool SHS_digest_compare(SHS_Digest_Size size, const n8* dA, const n8* dB)
{
	n64 i;
	for(i = 0; i < size; ++i)
	{
		if(dA[i] != dB[i]) return false;
	}

	return true;
}


void SHS_digest_from_Word32(SHS_Digest_Size dsize, n8* digest,
		n8 hcount, const Word32* hash)
{
	n8 i;

#define WORD_SIZE 4

	assert(hcount * WORD_SIZE == dsize);
	for(i = 0; i < hcount; ++i)
	{
		digest[i * WORD_SIZE + 0] = hash[i] >> 24;
		digest[i * WORD_SIZE + 1] = hash[i] >> 16;
		digest[i * WORD_SIZE + 2] = hash[i] >>  8;
		digest[i * WORD_SIZE + 3] = hash[i] >>  0;
	}
}

