#include "huffman.h"

#include <stdlib.h>

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

void hc_build_tree(hc_node_list* list)
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
		printf("| %4d | %10d    |\n", n->sym.b, n->sym.f);
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
		printf("[%4d] n=%d, code=", node->sym.b, node->sym.n);
		hc_print_bitstring(node->sym.code);
		printf("\n");
	}
	else
		printf("(branch) n=%d\n", level);

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

hc_bitstring* hc_prepare_output(FILE* input, hc_sym* table, hc_ulong len)
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

	if (ferror(input))
	{
		printf("error while reading from file\n");
		fclose(input);
		hc_destroy_bitstring(bs);
		return NULL;
	}

	return bs;
}

/* metadata */
static hc_byte table_begin = 1;
static hc_byte table_byte = 2;
static hc_byte table_code = 3;
static hc_byte table_end = 4;

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

hc_sym* hc_read_table(FILE* input)
{
	size_t cap = 10;
	size_t count = 0;
	size_t flag = 1;

	hc_sym* table = (hc_sym*)malloc(sizeof(hc_sym) * cap);

	hc_ulong ul; /* long   */
	hc_byte b; /* byte   */

	fread(&b, sizeof(hc_byte), 1, input);

	if (b != table_begin)
	{
		free(table);
		return NULL;
	}

	hc_sym sym;

	while (flag == 1)
	{
		flag = fread(&b, sizeof(hc_byte), 1, input);

		if (b == table_byte)
		{
			flag = fread(&b, sizeof(hc_byte), 1, input);
			if (b == 101)
			{
				int x = 0;
			}
			sym.b = b;
		}
		else if (b == table_code)
		{
			sym.code = hc_create_bitstring();

			flag = fread(&ul, sizeof(hc_ulong), 1, input);
			sym.code->bit_count = ul;

			flag = fread(&ul, sizeof(hc_ulong), 1, input);
			sym.code->byte_count = ul;

			flag = fread(&b, sizeof(hc_byte), 1, input);
			sym.code->current_bits = b;

			sym.code->bytes = (hc_byte*)malloc(sizeof(hc_byte) * sym.code->byte_count);
			flag = fread(sym.code->bytes, sizeof(hc_byte), sym.code->byte_count, input);

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

	return table;
}
