#define CORE_LOG_MODULE "SHS"
#define SGR_ENABLE 1

#include <core.h>
#include <shs.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


/* Create and free Blocks
 * ----------------------------------------------------------- */

SHS_Block512_List SHS_block512_create_list_from_file(FILE* file)
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

	assert(BLOCK_SIZE > 0 && BLOCK_SIZE < 255);
	{
	/* Read blocks from file */
		n64 i, file_blocks = (n64)(file_size / BLOCK_SIZE);
		n8 extra_bytes = (n8)(file_size % BLOCK_SIZE);
		SHS_Block512_List blocks = {0};

#define LAST_BLOCK_MAX_SIZE 55 /* 55B = (512 - 64 - 8)/8 */
		bool need_extra_block = extra_bytes > LAST_BLOCK_MAX_SIZE;

		/* #error "Debug here, check for 'added bytes'" */
		core_log(CORE_DEBUG,
				"Creating blocks for file of %lu bytes (%lu blocks + %lu extra bytes)\n",
				file_size, file_blocks, extra_bytes);

		if(file_blocks == 0 && extra_bytes == 0) {
			core_log(CORE_ERROR, "File is empty during block creation\n");
			exit(failure);
		}

		/* +1 is padding block,  */
		blocks.count = file_blocks + (need_extra_block ? 1 : 0) + 1;
		blocks.items = dmalloc(blocks.count * BLOCK_SIZE);
		if(blocks.items == NULL) {
			core_log(CORE_ERROR,
					"Can't mallocate %lu bytes to store blocks: %s\n",
					blocks.count, strerror(errno));
			exit(failure);
		}
		memset(blocks.items, 0, blocks.count * BLOCK_SIZE);

		for(i = 0; i < blocks.count; ++i)
		{
			SHS_Block512* block = &blocks.items[i];
			n64 j;

			size_t read_bytes, request_bytes = BLOCK_SIZE;

			if(blocks.count > 1 && i == blocks.count - 2 && need_extra_block)
				request_bytes = extra_bytes;

			if(i == blocks.count - 1 && extra_bytes > 0)
			{
				if(need_extra_block) break;
				request_bytes = extra_bytes;
			}

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

			/* core_log(CORE_DEBUG, "[%02lu] Read %lu bytes\n", i + 1, read_bytes); */

			/* Fix endianess */
			for(j = 0; j < 16; ++j)
			{
				SHS_Word32* word = &block->words[j];
				SHS_Word32 t = *word;
				*word
					= ((t & 0x000000FF) << 24)
					| ((t & 0x0000FF00) << 8)
					| ((t & 0x00FF0000) >> 8)
					| ((t & 0xFF000000) >> 24);
			}
		}

		{
			SHS_Block512* last_block = &blocks.items[blocks.count - 1];
			assert(!last_block->as.wF && !last_block->as.wE);

			if(extra_bytes > LAST_BLOCK_MAX_SIZE) {
				SHS_Block512* last_file_block = &blocks.items[blocks.count - 2];
				assert(extra_bytes < sizeof(SHS_Word32) * 16);

				/* write B10000000 to next free byte in word */
				last_file_block->words[extra_bytes / sizeof(SHS_Word32)]
					|= 0x80 << ((sizeof(SHS_Word32) - extra_bytes - 1) * 8);
			}
			else
			{
				last_block->words[extra_bytes / sizeof(SHS_Word32)]
					|= 0x80 << ((sizeof(SHS_Word32) - extra_bytes - 1) * 8);
			}

			/* write size (64b) to last words (2*32b) */
			last_block->as.wE = (n32)(file_size * 8 >> 32);
			last_block->as.wF = (n32)(file_size * 8);
		}

		return blocks;

	/* END Read blocks from file END */
	}
}

/* extern Block1024_List SHS_block1024_create_list_from_file(FILE* file); */

void SHS_block512_List_free(SHS_Block512_List* blocks) {
	if(blocks->items != NULL) dfree(blocks->items);
	blocks->count = 0;
}

void SHS_block1024_List_free(SHS_Block1024_List* blocks) {
	if(blocks->items != NULL) dfree(blocks->items);
	blocks->count = 0;
}


/* Operate on Word (ww is Word size, 32 or 64)
 * ----------------------------------------------------------- */

SHS_Word32 SHS_Word32_ROTL(SHS_Word32 word, n8 n) {
	assert(n < 32); /* 0 <= n < w */
	return (word << n) | (word >> (32 - n));
}

SHS_Word32 SHS_Word32_ROTR(SHS_Word32 word, n8 n) {
	assert(n < 32); /* 0 <= n < w */
	return (word >> n) | (word << (32 - n));
}

/* extern Word64 SHS_Word64_ROTL(Word64 word); */
/* extern Word64 SHS_Word64_ROTR(Word64 word); */
/* extern Word64 SHS_Word64_SHR(Word64 word); */


/* Generate digest
 * ----------------------------------------------------------- */

static SHS_Word32 SHS_SHA1_f(n8 t, SHS_Word32 b, SHS_Word32 c, SHS_Word32 d) {
	assert(t < 80);
	if(t <= 19) return (b & c) | ((~b) & d);
	if(t <= 39) return b ^ c ^ d;
	if(t <= 59) return (b & c) | (b & d) | (c & d);
	return b ^ c ^ d;
}

static SHS_Word32 SHS_SHA1_K(n8 t) {
	assert(t < 80);
	if(t <= 19) return 0x5a827999;
	if(t <= 39) return 0x6ed9eba1;
	if(t <= 59) return 0x8f1bbcdc;
	return 0xca62c1d6;
}

SHS_digest160 SHS_SHA1_generate_digest(SHS_Block512_List blocks)
{
	SHS_Word32 hash[5] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 };
	SHS_digest160 digest = {0};

	n64 i;
	n8 t;

	core_log(CORE_DEBUG, "Generating SHA1 hash with %lu blocks\n", blocks.count);

	for(i = 0; i < blocks.count; ++i)
	{
		SHS_Block512 block = blocks.items[i]; /* M */

		struct { SHS_Word32 a, b, c, d, e; } var = {0};
		SHS_Word32 T, W[80] = {0};


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
			T = SHS_Word32_ROTL(var.a, 5) + SHS_SHA1_f(t, var.b, var.c, var.d)
			  + var.e + W[t] + SHS_SHA1_K(t);
			var.e = var.d;
			var.d = var.c;
			var.c = SHS_Word32_ROTL(var.b, 30);
			var.b = var.a;
			var.a = T;
		}

		/* core_log(CORE_DEBUG, "[%02lu] Computing...    a: %#010x, b: %#010x, c: %#010x, d: %#010x, e: %#010x\n", */
		/* 		i, var.a, var.b, var.c, var.d, var.e); */

		/* 4. Compute the intermediate hash value */
		hash[0] += var.a;
		hash[1] += var.b;
		hash[2] += var.c;
		hash[3] += var.d;
		hash[4] += var.e;

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
		n8 hcount, const SHS_Word32* hash)
{
	n8 i;

	assert(hcount * SHS_WORD32_SIZE == dsize);
	for(i = 0; i < hcount; ++i)
	{
		digest[i * SHS_WORD32_SIZE + 0] = (n8)(hash[i] >> 24);
		digest[i * SHS_WORD32_SIZE + 1] = (n8)(hash[i] >> 16);
		digest[i * SHS_WORD32_SIZE + 2] = (n8)(hash[i] >>  8);
		digest[i * SHS_WORD32_SIZE + 3] = (n8)(hash[i] >>  0);
	}
}

