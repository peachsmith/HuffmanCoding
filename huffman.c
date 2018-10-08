#include "huffman.h"

#include <stdlib.h>

hc_node* hc_create_node()
{
	hc_node* node = (hc_node*)malloc(sizeof(hc_node));
	node->leaf_1 = NULL;
	node->leaf_2 = NULL;
	node->next = NULL;
	node->prev = NULL;
	node->sym.code = NULL;
	node->sym.b = 0;
	node->sym.f = 0;
	node->sym.w = 0;
	node->sym.n = 0;

	return node;
}

hc_node_list* hc_create_list()
{
	hc_node_list* list = (hc_node_list*)malloc(sizeof(hc_node_list));

	list->count = 0;
	list->nodes = NULL;

	return list;
}

void hc_destroy_node(hc_node* node)
{
	if (node == NULL)
		return;

	if (node->leaf_1 != NULL)
		hc_destroy_node(node->leaf_1);

	if (node->leaf_2 != NULL)
		hc_destroy_node(node->leaf_2);

	if (node->sym.code != NULL)
	{
		hc_destroy_bitstring(node->sym.code);
	}

	free(node);
}

void hc_destroy_list(hc_node_list* list)
{
	if (list == NULL)
		return;

	hc_node* node = list->nodes;
	hc_node* next = NULL;
	while (node != NULL)
	{
		next = node->next;
		hc_destroy_node(node);
		node = next;
	}
	free(list);
}

void hc_add_node(hc_node_list* list, hc_node* node)
{
	if (list->count == 0 && list->nodes == NULL)
	{
		list->nodes = node;
		list->nodes->next = NULL;
		list->nodes->prev = NULL;
		list->tail = node;
	}
	else if (list->nodes != NULL)
	{
		list->tail->next = node;
		list->tail->next->prev = list->tail;
		list->tail = node;
	}

	list->count++;
}

void hc_sort_leaves(hc_node_list* list)
{
	int sorted = 0;
	while (!sorted)
	{
		sorted = 1;
		hc_node** node = &(list->nodes);
		while (*node != NULL)
		{
			if ((*node)->next != NULL && (*node)->next->sym.f < (*node)->sym.f)
			{
				sorted = 0;
				hc_node* next = (*node)->next;

				(*node)->next = next->next;
				next->prev = (*node)->prev;
				if (next->next != NULL)
				{
					next->next->prev = *node;
				}
				(*node)->prev = next;
				next->next = *node;
				*node = next;
			}
			else
			{
				node = &((*node)->next);
			}
		}
	}
}

void hc_construct_tree(hc_node_list* list)
{
	int built = 0;
	while (!built)
	{
		built = 1;

		hc_node** node = &(list->nodes);

		if ((*node)->next != NULL)
		{
			built = 0;

			hc_node* sum = (hc_node*)malloc(sizeof(hc_node));
			sum->next = NULL;
			sum->prev = NULL;
			sum->sym.f = (*node)->sym.f + (*node)->next->sym.f;
			sum->sym.b = 0;
			sum->sym.w = 1;
			sum->sym.n = 1;
			sum->sym.code = NULL;
			sum->leaf_1 = *node;
			sum->leaf_2 = (*node)->next;

			hc_node* next = (*node)->next->next;
			hc_node* last_next = next;
			hc_node* new_head = next;

			while (next != NULL && next->sym.f < sum->sym.f)
			{
				last_next = next;
				next = next->next;
			}

			sum->next = next;

			if (next != NULL)
			{
				sum->prev = next->prev;
				if (next->prev != NULL)
				{
					next->prev->next = sum;
				}
				next->prev = sum;
			}
			else if (next == NULL && last_next != NULL)
			{
				last_next->next = sum;
				sum->prev = last_next;
			}

			if (new_head == NULL || new_head == next)
			{
				*node = sum;
			}
			else
			{
				new_head->prev = NULL;
				*node = new_head;
			}

			list->count--;
		}
	}
}

static void hc_assign_node(hc_node* node, hc_ulong level, hc_bitstring* bs)
{
	if (node == NULL) return;

	if (node->sym.w < 1)
	{
		node->sym.n = level;
		hc_add_bits(node->sym.code, bs);
	}

	if (node->leaf_1 != NULL)
	{
		hc_add_bit(bs, 1);
		hc_assign_node(node->leaf_1, level + 1, bs);
		hc_remove_bit(bs);
	}

	if (node->leaf_2 != NULL)
	{
		hc_add_bit(bs, 0);
		hc_assign_node(node->leaf_2, level + 1, bs);
		hc_remove_bit(bs);
	}
}

void hc_assign_codes(hc_node_list* list)
{
	if (list == NULL)
		return;

	hc_node* node = list->nodes;

	node->sym.n = 0;

	hc_bitstring* bs = hc_create_bitstring();

	if (node->sym.w == 0)
	{
		hc_add_bit(bs, 0);
	}

	hc_assign_node(node, 0, bs);

	hc_destroy_bitstring(bs);
}

void hc_print_list(hc_node_list* list)
{
	if (list == NULL)
		return;

	printf("+----------------------+\n");
	printf("| byte | frequency     |\n");
	printf("+----------------------+\n");

	hc_node* n = list->nodes;
	while (n != NULL)
	{
		printf("| %4d | %10ld    |\n", n->sym.b, n->sym.f);
		n = n->next;
	}

	printf("+----------------------+\n");
}

static void hc_print_node(hc_node* node, hc_ulong level)
{
	if (node == NULL) return;

	hc_ulong i;
	for (i = 0; i < level; i++)
	{
		printf("-");
	}

	if (node->sym.w < 1)
	{
		printf("[%4d] n=%ld, code=", node->sym.b, level);
		hc_print_bitstring(node->sym.code);
		printf("\n");
	}
	else
		printf("(branch) n=%ld\n", level);

	if (node->leaf_1 != NULL)
		hc_print_node(node->leaf_1, level + 1);

	if (node->leaf_2 != NULL)
		hc_print_node(node->leaf_2, level + 1);
}

void hc_print_tree(hc_node_list* list)
{
	hc_node* node = list->nodes;

	hc_print_node(node, 0);
}

hc_bitstring* hc_create_bitstring()
{
	hc_bitstring* bs = (hc_bitstring*)malloc(sizeof(hc_bitstring));

	bs->bit_count = 0;
	bs->byte_count = 1;
	bs->bytes = malloc(sizeof(hc_byte));
	bs->bytes[0] = 0;
	bs->current_bits = 0;

	return bs;
}

void hc_destroy_bitstring(hc_bitstring* bs)
{
	if (bs == NULL)
		return;

	free(bs->bytes);
	free(bs);
}

void hc_add_bit(hc_bitstring* bs, unsigned int bit)
{
	if (bit != 0 && bit != 1)
		return;

	if (bs->current_bits == CHAR_BIT)
	{
		bs->bytes = (hc_byte*)realloc(bs->bytes, bs->byte_count + 1);
		bs->byte_count++;
		bs->current_bits = 0;
		bs->bytes[bs->byte_count - 1] = 0;
	}

	bs->bytes[bs->byte_count - 1] |= (bit << bs->current_bits++);

	bs->bit_count++;
}

void hc_add_bits(hc_bitstring* dest, hc_bitstring* src)
{
	hc_ulong byte = 0;
	hc_ulong bit = 0;
	hc_ulong i;
	for (i = 0; i < src->bit_count; i++)
	{
		if (bit == CHAR_BIT)
		{
			byte++;
			bit = 0;
		}
		if (src->bytes[byte] & (1 << bit++))
		{
			hc_add_bit(dest, 1);
		}
		else
		{
			hc_add_bit(dest, 0);
		}
	}
}

void hc_remove_bit(hc_bitstring* bs)
{
	if (bs == NULL || bs->bit_count < 1)
		return;

	bs->bytes[bs->byte_count - 1] &= ~(1 << (bs->current_bits - 1));

	bs->current_bits--;

	if (bs->current_bits == 0 && bs->byte_count > 1)
	{
		bs->bytes = (hc_byte*)realloc(bs->bytes, bs->byte_count - 1);
		bs->byte_count--;
		bs->current_bits = CHAR_BIT;
	}

	bs->bit_count--;
}

void hc_print_bitstring(hc_bitstring* bs)
{
	hc_ulong byte = 0;
	hc_ulong bit = 0;
	hc_ulong i;
	for (i = 0; i < bs->bit_count; i++)
	{
		if (bit == CHAR_BIT)
		{
			byte++;
			bit = 0;
		}
		if (bs->bytes[byte] & (1 << bit++))
		{
			printf("1");
		}
		else
		{
			printf("0");
		}
	}
}

hc_bitstring* hc_encode_data(FILE* input, hc_sym* table, hc_ulong len)
{
	hc_bitstring* bs = hc_create_bitstring();

	hc_ulong i;
	hc_byte b;

	while (fread(&b, 1, 1, input) == 1)
	{
		for (i = 0; i < len; i++)
		{
			if (table[i].b == b)
				hc_add_bits(bs, table[i].code);
		}
	}

	return bs;
}

/* metadata */
static hc_byte table_begin = 1;
static hc_byte table_byte = 2;
static hc_byte table_code = 3;
static hc_byte table_end = 4;
static hc_byte data_begin = 5;
static hc_byte data_end = 6;

void hc_write_table(FILE* out_file, hc_sym* table, hc_ulong len)
{
	hc_ulong i;
	hc_ulong j;

	fwrite(&table_begin, 1, 1, out_file);

	for (i = 0; i < len; i++)
	{
		/* write the byte */
		fwrite(&table_byte, sizeof(hc_byte), 1, out_file);
		fwrite(&(table[i].b), sizeof(hc_byte), 1, out_file);

		/* write the bit code metadata */
		fwrite(&table_code, sizeof(hc_byte), 1, out_file);
		fwrite(&(table[i].code->bit_count), sizeof(hc_ulong), 1, out_file);
		fwrite(&(table[i].code->byte_count), sizeof(hc_ulong), 1, out_file);
		fwrite(&(table[i].code->current_bits), sizeof(hc_byte), 1, out_file);

		/* write the bit code data */
		for (j = 0; j < table[i].code->byte_count; j++)
		{
			fwrite(&(table[i].code->bytes[j]), sizeof(hc_byte), 1, out_file);
		}
	}

	fwrite(&table_end, 1, 1, out_file);
}

hc_sym* hc_read_table(FILE *in_stream, size_t *len)
{
	size_t cap = 10;
	size_t count = 0;
	size_t flag = 1;

	hc_sym* table = (hc_sym*)malloc(sizeof(hc_sym) * cap);

	hc_ulong ul; /* long */
	hc_byte b;   /* byte */

	fread(&b, sizeof(hc_byte), 1, in_stream);

	if (b != table_begin)
	{
		free(table);
		return NULL;
	}

	hc_sym sym;

	while (flag > 0)
	{
		flag = fread(&b, sizeof(hc_byte), 1, in_stream);

		if (b == table_byte)
		{
			flag = fread(&b, sizeof(hc_byte), 1, in_stream);
			sym.b = b;
		}
		else if (b == table_code)
		{
			sym.code = hc_create_bitstring();

			/* destroy the bytes since we'll recreate them later */
			free(sym.code->bytes);

			flag = fread(&ul, sizeof(hc_ulong), 1, in_stream);
			sym.code->bit_count = ul;

			flag = fread(&ul, sizeof(hc_ulong), 1, in_stream);
			sym.code->byte_count = ul;

			flag = fread(&b, sizeof(hc_byte), 1, in_stream);
			sym.code->current_bits = b;

			sym.code->bytes = (hc_byte*)
				malloc(sizeof(hc_byte) * sym.code->byte_count);

			flag = fread(sym.code->bytes, sizeof(hc_byte),
				sym.code->byte_count, in_stream);

			/* resize the dictionary if necessary */
			if (count >= cap)
			{
				size_t new_cap = cap + cap / 2;
				table = (hc_sym*)realloc(table, sizeof(hc_sym) * new_cap);
				cap = new_cap;
			}

			/* add the symbol to the dictionary */
			table[count++] = sym;
		}
		else if (b == table_end)
		{
			flag = 0;
		}
	}

	/* free excess memory */
	if (count < cap)
	{
		table = (hc_sym*)realloc(table, sizeof(hc_sym) * count);
	}

	*len = count;

	return table;
}

void hc_write_data(FILE *out_stream, hc_bitstring *bs)
{
	hc_ulong i;

	fwrite(&data_begin, sizeof(hc_byte), 1, out_stream);

	/* write the bit string metadata */
	fwrite(&(bs->bit_count), sizeof(hc_ulong), 1, out_stream);
	fwrite(&(bs->byte_count), sizeof(hc_ulong), 1, out_stream);
	fwrite(&(bs->current_bits), sizeof(hc_byte), 1, out_stream);

	/* write the bit code data */
	for (i = 0; i < bs->byte_count; i++)
	{
		fwrite(&(bs->bytes[i]), sizeof(hc_byte), 1, out_stream);
	}

	fwrite(&data_end, sizeof(hc_byte), 1, out_stream);
}

hc_bitstring* hc_read_data(FILE* in_stream)
{
	hc_bitstring* bs = hc_create_bitstring();

	/* dispose of this since we create it later */
	free(bs->bytes);

	hc_ulong ul;
	hc_byte b;
	size_t flag = 1;

	flag = fread(&b, sizeof(hc_byte), 1, in_stream);

	if (b == data_begin)
	{
		fread(&ul, sizeof(hc_ulong), 1, in_stream);
		bs->bit_count = ul;

		fread(&ul, sizeof(hc_ulong), 1, in_stream);
		bs->byte_count = ul;

		fread(&b, sizeof(hc_byte), 1, in_stream);
		bs->current_bits = b;

		bs->bytes = (hc_byte*)malloc(sizeof(hc_byte) * bs->byte_count);

		flag = fread(bs->bytes, sizeof(hc_byte), bs->byte_count, in_stream);
	}

	return bs;
}

static void hc_build_branch(hc_node** node, hc_byte* bytes, hc_sym data,
	hc_byte byte, hc_ulong bit, hc_ulong bit_count, hc_ulong bit_cap)
{
	if (bit_count == bit_cap)
	{
		if (*node == NULL)
		{
			*node = hc_create_node();
		}

		(*node)->sym = data;
		(*node)->sym.w = 0;

		return;
	}

	if (bit == CHAR_BIT)
	{
		byte++;
		bit = 0;
	}

	hc_node** leaf;

	if (bytes[byte] & (1 << bit++))
		leaf = &((*node)->leaf_1);
	else
		leaf = &((*node)->leaf_2);

	if (*leaf == NULL)
	{
		*leaf = hc_create_node();
		(*leaf)->sym.w = 1;
	}

	hc_build_branch(leaf, bytes, data, byte, bit, ++bit_count, bit_cap);
}

hc_node_list* hc_reconstruct_tree(hc_sym* table, hc_ulong len)
{
	hc_ulong i;
	hc_node_list* tree = hc_create_list();

	hc_node* root = hc_create_node();
	root->sym.w = 1;

	hc_add_node(tree, root);

	/*
	 * leaf_1 => 1
	 * leaf_2 => 0
	 */

	for (i = 0; i < len; i++)
	{
		hc_bitstring* bs = table[i].code;

		hc_build_branch(&root, bs->bytes, table[i],
			0,            /* current byte              */
			0,            /* position in current byte  */
			0,            /* current bit count         */
			bs->bit_count /* bit capactity             */
		);
	}

	return tree;
}

void hc_decode_data(hc_bitstring *bs, hc_node_list *tree, FILE *out_stream)
{
	hc_node* root = tree->nodes;
	hc_node* leaf = root;

	hc_ulong byte = 0;
	hc_ulong bit = 0;
	hc_ulong i;

	for (i = 0; i <= bs->bit_count; i++)
	{
		if (bit == CHAR_BIT)
		{
			byte++;
			bit = 0;
		}

		if (leaf->sym.w == 0)
		{
			fwrite(&(leaf->sym.b), sizeof(hc_byte), 1, out_stream);
			leaf = root;
			i--;
		}
		else
		{
			if (bs->bytes[byte] & (1 << bit++))
			{
				leaf = leaf->leaf_1;
			}
			else
			{
				leaf = leaf->leaf_2;
			}
		}
	}
}

int hc_encode_file(FILE *in_stream, FILE *out_stream)
{
	hc_ulong unique;
	hc_ulong i;
	hc_ulong j;
	hc_byte b;

	hc_sym data[UCHAR_MAX + 1];
	hc_sym* dict;
	hc_node_list* tree;
	hc_bitstring* enc;

	for (i = 0; i < UCHAR_MAX + 1; i++)
	{
		data[i].b = (hc_byte)i;
		data[i].f = 0;
		data[i].w = 0;
		data[i].n = 1;
	}

	/* read the data from the input stream */
	unique = 0;
	while (fread(&b, 1, 1, in_stream) == 1)
	{
		if (data[b].f == 0)
		{
			unique++;
			data[b].code = hc_create_bitstring();
		}
		data[b].f++;
	}

	fseek(in_stream, 0, SEEK_SET);

	tree = hc_create_list();

	/* create a leaf node for each unique byte */
	for (i = 0, j = 0; i < UCHAR_MAX + 1; i++)
	{
		if (data[i].f > 0)
		{
			/*
			 * nodes created here will be destroyed when
			 * the tree is destroyed.
			 */
			hc_node *node = malloc(sizeof(hc_node));
			node->next = NULL;
			node->leaf_1 = NULL;
			node->leaf_2 = NULL;
			node->sym = data[i];
			hc_add_node(tree, node);
			j++;
		}
	}

	/* sort the leave by frequency */
	hc_sort_leaves(tree);

	/* construct the tree */
	hc_construct_tree(tree);

	/* assign bit codes to the leaves */
	hc_assign_codes(tree);

	/* populate the bit code dictionary with bit codes */
	dict = (hc_sym*)malloc(sizeof(hc_sym) * unique);
	for (i = 0, j = 0; i < UCHAR_MAX + 1; i++)
	{
		if (data[i].f > 0)
		{
			dict[j].b = data[i].b;
			dict[j].f = data[i].f;
			dict[j].n = data[i].n;
			dict[j++].code = data[i].code;
		}
	}

	/* write the bit code dictionary to the output stream */
	hc_write_table(out_stream, dict, unique);

	if (ferror(out_stream))
	{
		hc_destroy_list(tree);
		return 0;
	}

	/* encode the data from the input stream */
	enc = hc_encode_data(in_stream, dict, unique);

	if (ferror(in_stream))
	{
		hc_destroy_list(tree);
		hc_destroy_bitstring(enc);
		return 0;
	}

	/* write the encoded data to the output stream */
	hc_write_data(out_stream, enc);

	hc_destroy_list(tree);
	hc_destroy_bitstring(enc);

	return 1;
}

int hc_decode_file(FILE *in_stream, FILE *out_stream)
{
	size_t len;
	hc_sym* dict;
	hc_node_list* tree;
	hc_bitstring* enc;

	/* read the bit code dictionary from the input stream */
	dict = hc_read_table(in_stream, &len);

	/* reconstruct the tree from the bit code dictionary */
	tree = hc_reconstruct_tree(dict, len);

	/* read the encoded data from the input stream */
	enc = hc_read_data(in_stream);

	if (ferror(in_stream))
	{
		hc_destroy_list(tree);
		return 0;
	}

	/* decode the bit string and write to the output stream */
	hc_decode_data(enc, tree, out_stream);

	hc_destroy_bitstring(enc);
	hc_destroy_list(tree);

	if (ferror(out_stream))
		return 0;

	return 1;
}
