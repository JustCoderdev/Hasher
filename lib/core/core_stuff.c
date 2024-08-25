/* Stuff module for JustCoderdev Core library v1
 * */

#include <core.h>

#include <assert.h>

char* shift(int* argc, char*** argv)
{
	char* t;
	assert(*argc > 0);

	(*argc)--;
	t = (*argv)[0];
	(*argv)++;

	return t;
}

