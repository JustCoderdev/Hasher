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
		digest160 digest;
		Block512_List blocks = SHS_block512_create_list_from_file(ftoh_file);
		fclose(ftoh_file);

		digest = SHS_SHA1_generate_digest(blocks);
		printf(D160_FMT, D160(digest));

		SHS_block512_List_free(&blocks);
	}

	return success;
}
