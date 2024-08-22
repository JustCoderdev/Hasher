/* Stuff module for JustCoderdev Core library v1
 * */

#include <core.h>

#include <assert.h>

char* shift(int* argc, char*** argv)
{
	char* t;

	(*argc)--;
	assert(*argc >= 0);

	t = (*argv)[*argc];
	(*argv)++;

	return t;
}

