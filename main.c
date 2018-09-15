#include <stdlib.h>
#include <stdio.h>

#include "huffman.h"

int main(int argc, char** argv)
{
	hc_ulong unique;
	hc_ulong i;
	hc_byte b;

	hc_sym data[UCHAR_MAX + 1];
	hc_sym* table;
	hc_node_list* tree;
	FILE* input;
	hc_bitstring* output;

	for (i = 0; i < UCHAR_MAX + 1; i++)
	{
		data[i].b = (hc_byte)i;
		data[i].f = 0;
		data[i].w = 0;
		data[i].n = 1;
	}

	const char* default_file = "test.txt";
	const char* file_name = default_file;

	if (argc == 2)
	{
		file_name = argv[1];
	}

	input = fopen(file_name, "rb");

	if (input == NULL)
	{
		printf("could not open input file\n");
		return 1;
	}

	unique = 0;

	while (fread(&b, 1, 1, input) == 1)
	{
		if (data[b].f == 0)
		{
			unique++;
			data[b].code = hc_create_bitstring();
		}

		data[b].f++;
	}

	if (ferror(input))
	{
		printf("error while reading from file\n");
		fclose(input);
		return 0;
	}

	fclose(input);

	tree = hc_create_list();

	hc_ulong j = 0;
	for (i = 0; i < UCHAR_MAX + 1; i++)
	{
		if (data[i].f > 0)
		{
			hc_node *node = malloc(sizeof(hc_node));
			node->next = NULL;
			node->leaf_1 = NULL;
			node->leaf_2 = NULL;
			node->sym = data[i];
			hc_add_node(tree, node);
			j++;
		}
	}

	/* sort nodes by frequency */
	hc_sort_leaves(tree);

	/* build the binary tree */
	hc_build_tree(tree);

	/* assing bit codes to the leaves of the tree */
	hc_assign_codes(tree);
	hc_print_tree(tree);

	/* populate the bit code dictionary */
	table = (hc_sym*)malloc(sizeof(hc_sym) * unique);
	for (i = 0, j = 0; i < UCHAR_MAX + 1; i++)
	{
		if (data[i].f > 0)
		{
			table[j].b = data[i].b;
			table[j].f = data[i].f;
			table[j].n = data[i].n;
			table[j++].code = data[i].code;
		}
	}

	/* print the bit code table */
	for (i = 0; i < unique; i++)
	{
		printf("[%4d][%10d] code: ", table[i].b, table[i].f);
		hc_print_bitstring(table[i].code);
		printf("\n");
	}

	/* open the file again */
	input = fopen(file_name, "rb");

	if (input == NULL)
	{
		printf("could not open input file\n");
		return 1;
	}

	/* build the bit string */
	output = hc_prepare_output(input, table, unique);

	fclose(input);

	if (output != NULL)
	{
		hc_print_bitstring(output);
		printf("\n");
	}

	FILE* out_file = fopen("out.huff", "wb");

	if (out_file == NULL)
	{
		printf("could not open input file\n");
		free(table);
		hc_destroy_list(tree);
		return 1;
	}

	/* write the bit code dictionary to a file */
	hc_write_table(out_file, table, unique);

	/* write the data to a file */
	hc_write_data(out_file, output);

	hc_destroy_bitstring(output);

	fclose(out_file);

	input = fopen("out.huff", "rb");

	if (input == NULL)
	{
		printf("could not open input file\n");
		free(table);
		hc_destroy_list(tree);
		return 1;
	}

	/* read the bit code dictionary from a file */
	size_t len;
	hc_sym* new_table = hc_read_table(input, &len);

	/* print the bit code table */
	for (i = 0; i < (hc_ulong)len; i++)
	{
		printf("[%4d] code: ", new_table[i].b);
		hc_print_bitstring(new_table[i].code);
		printf("\n");

		/* destroy the bit code here until we reconstruct the tree */
		/* hc_destroy_bitstring(new_table[i].code); */
	}

	hc_node_list* new_tree = hc_reconstruct_tree(new_table, unique);

	if (new_tree != NULL)
	{
		hc_print_tree(new_tree);
		hc_destroy_list(new_tree);
	}

	free(table);
	hc_destroy_list(tree);

	return 0;
}
