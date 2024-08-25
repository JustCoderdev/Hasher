#include <core.h>
#include <shs.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void usage(CString program)
{
	printf("Usage: %s [OPTIONS] FILE\n"
			"\n"
			"Options:\n"
			"-s\tprint short version of hash (16bits)\n",
	program);
}

typedef struct Flags {
	unsigned int short_hash: 1;
} Flags;

int main(int argc, char** argv)
{
	FILE* ftoh_file;
	Flags flags = {0};
	char *arg, *ftoh_name,
		 *program = shift(&argc, &argv);
	if(argc < 1) {
		core_log(CORE_ERROR, "Missing input file\n");
		usage(program);
		exit(failure);
	}

	arg = shift(&argc, &argv);
	if(arg[0] == '-')
	{
		switch(arg[1]) {
			case '\0':
				core_log(CORE_ERROR, "Incomplete flag\n");
				usage(program);
				exit(failure);
				break;

			case 's':
				flags.short_hash = true;
				break;

			default:
				core_log(CORE_ERROR, "Unrecognised flag '-%c'\n", arg[1]);
				usage(program);
				exit(failure);
		}

		ftoh_name = shift(&argc, &argv);
	} else {
		ftoh_name = arg;
	}

	core_log(CORE_DEBUG, "Hashing file '%s'\n", ftoh_name);

	/* ftoh: file to hash */
	ftoh_file = fopen(ftoh_name, "rb");
	if(ftoh_file == NULL) {
		core_log(CORE_ERROR, "Can't open file '%s' to hash: %s\n",
				ftoh_name, strerror(errno));
		exit(failure);
	}

	{
		SHS_digest160 digest;
		SHS_Block512_List blocks;

		blocks = SHS_block512_create_list_from_file(ftoh_file);
		fclose(ftoh_file);

		digest = SHS_SHA1_generate_digest(blocks);
		SHS_block512_List_free(&blocks);

		if(flags.short_hash) {
			printf("%08x%08x\n",
				digest.byte[ 0] << 24 | digest.byte[ 1] << 16 | digest.byte[ 2] << 8 | digest.byte[ 3],
				digest.byte[ 4] << 24 | digest.byte[ 5] << 16 | digest.byte[ 6] << 8 | digest.byte[ 7]
			);

		} else {
			printf(SHS_D160_FMT "\n", SHS_D160(digest));
		}
	}

	return success;
}
