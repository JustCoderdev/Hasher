#include <core.h>
#include <shs.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


int main(int argc, char** argv)
{
	FILE* ftoh_file;
	char *ftoh_name, *program = shift(&argc, &argv);
	if(argc < 1) {
		core_log(CORE_ERROR, "Missing input file. Usage: %s <ftoh>\n",
				program);
		exit(failure);
	}

	/* ftoh: file to hash */
	ftoh_name = shift(&argc, &argv);
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

		printf(SHS_D160_FMT "\n", SHS_D160(digest));
	}

	return success;
}
