#include <core.h>
#include <shs.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>
int snprintf(char* str, size_t size, const char *format, ...);



char* shift(int* argc, char*** argv)
{
	char* t;

	(*argc)--;
	assert(*argc >= 0);
	t = (*argv)[*argc];
	(*argv)++;

	return t;
}

void test(CString message, bool pass, CString expected, CString got) {
	if(pass)
		printf("[*] \033[32mPASS\033[0m: %s\n",
				message);
	else
		printf("[x] \033[31mFAIL\033[0m: %s\n  expected: %s\n       got: %s\n",
				message, expected, got);
}

int main(int argc, char** argv)
{
	Block512_List blocks = {0};

	printf("==== SHS Validation Tests ====\n");

	printf("\n Word32 Operations:\n");
	test("W32 Left Rotation (SHS_Word32_ROTL)",
	     SHS_Word32_ROTL(10, 5) == SHS_Word32_ROTR(10, 32 - 5),
	     "true", "false"
	);

	{
	/* Block creation */
		n64 i;
		FILE* tfile = fopen("aaaa", "r");
		assert(tfile);

		printf("\n Block creations:\n");
		blocks = SHS_block512_create_list_from_file(tfile);
		fclose(tfile);
	}

	{
	/* Digest generation */
		#define BUFF_LEN 43
		char buffH[BUFF_LEN] = {0};
		char buffR[BUFF_LEN] = {0};
		digest160 hardig = {0};

		printf("\n Digest operations:\n");

		{
			Word32 result[5] = { 0x4cfe4d18, 0x6a363dac, 0xd896f462, 0x973eb533, 0x6864e3e4 };
			char* result_text = "0x4cfe4d186a363dacd896f462973eb5336864e3e4";

			SHS_digest_from_Word32(SHS_DS160, hardig.byte, 5, result);
			assert(snprintf(buffH, BUFF_LEN, D160_FMT, D160(hardig)) < BUFF_LEN);

			test("Word32 array to digest (SHS_digest_from_Word32)",
				 !strncmp(result_text, buffH, BUFF_LEN), result_text, buffH);
		}


		printf("\n SHA-1:\n");

		{
			digest160 digest = SHS_SHA1_generate_digest(blocks);

			assert(snprintf(buffR, BUFF_LEN, D160_FMT, D160(digest)) < BUFF_LEN);

			test("Test A", SHS_digest_compare(SHS_DS160, digest.byte, hardig.byte),
				 buffH, buffR);
		}
	}

#if 1
	{
	/* List blocks as bytes */
		n64 i, j;
		for(i = 0; i < blocks.count; ++i)
		{
			Block512 block = blocks.items[i];

			printf("\n%02lu|", i + 1);
			for(j = 0; j < 8; ++j)
			{
				printf(" %c %c %c %c_ %c %c %c %c|",
					block.words[j * 2] >> 24, block.words[j * 2] >> 16,
					block.words[j * 2] >> 8, block.words[j * 2] >> 0,
					block.words[j * 2 + 1] >> 24, block.words[j * 2 + 1] >> 16,
					block.words[j * 2 + 1] >> 8, block.words[j * 2 + 1] >> 0
					);
			}
			printf("\n");

			printf("%02lu|", i + 1);
			for(j = 0; j < 8; ++j)
			{
				printf("%08x_%08x|", block.words[j * 2], block.words[j * 2 + 1]);
			}
			printf("\n");
		}
	}
#endif

	SHS_block512_List_free(&blocks);

	/* from different seeds,
	 * generate blobs of random data, size
	 * and compare them with known results
	 * */

	return success;
}
