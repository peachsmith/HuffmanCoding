#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "huffman.h"

int main(int argc, char** argv)
{
	FILE* in_stream;
	FILE* out_stream;

	if (argc != 4)
	{
		printf("invalid number of arguments expected 4 found %d\n", argc);
		return 1;
	}

	in_stream = fopen(argv[2], "rb");

	if (in_stream == NULL)
	{
		printf("could not open input file\n");
		return 1;
	}

	out_stream = fopen(argv[3], "wb");

	if (out_stream == NULL)
	{
		printf("could not open output file\n");
		fclose(in_stream);
		return 1;
	}

	if (!strcmp(argv[1], "-e"))
	{
		hc_encode_file(in_stream, out_stream);
	}
	else if (!strcmp(argv[1], "-d"))
	{
		hc_decode_file(in_stream, out_stream);
	}

	fclose(in_stream);
	fclose(out_stream);

	return 0;
}
