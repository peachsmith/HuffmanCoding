#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>

typedef unsigned char hc_byte;
typedef unsigned long hc_ulong;
typedef struct hc_sym hc_sym;
typedef struct hc_node hc_node;
typedef struct hc_node_list hc_node_list;
typedef struct hc_bitstring hc_bitstring;

struct hc_sym {
	hc_byte b;          /* byte       */
	hc_ulong f;         /* frequency  */
	hc_ulong w;         /* weight     */
	hc_ulong n;         /* tree depth */
	hc_bitstring* code; /* bit code   */
};

struct hc_node {
	hc_sym sym;
	hc_node* leaf_1;
	hc_node* leaf_2;
	hc_node* next;
	hc_node* prev;
};

struct hc_node_list {
	unsigned long count;
	hc_node* nodes;
	hc_node* tail;
};

struct hc_bitstring {
	hc_ulong bit_count;
	hc_ulong byte_count;
	hc_byte* bytes;
	hc_byte current_bits;
};

/**
 * Creates a new Huffman leaf node
 */
hc_node* hc_create_node();

/**
 * Creates an empty list of hc_nodes.
 *
 * Returns:
 *   hc_node_list - a reference to a new, empty hc_node_list
 */
hc_node_list* hc_create_list();

/**
 * Frees the resources allocated for an hc_node
 *
 * Params:
 *   hc_node - a reference to the node to destroy
 */
void hc_destroy_node(hc_node*);

/**
 * Frees the resources allocated for a list of hc_nodes
 *
 * Params:
 *   hc_node_list - a reference to the list to destroy
 */
void hc_destroy_list(hc_node_list*);

/**
 * Inserts an hc_node into an hc_node_list.
 *
 * Params:
 *   hc_node_list - a reference to the list to receive the node
 *   hc_node - a reference to the node to be inserted
 */
void hc_add_node(hc_node_list*, hc_node*);

/**
 * Sorts a list of hc_nodes by frequency.
 *
 * Params:
 *   hc_node_list - a reference to a list of hc_nodes
 */
void hc_sort_leaves(hc_node_list*);

/**
 * Constructs a binary tree from a list of hc_nodes.
 *
 * Params:
 *   hc_node_list - a reference to a list of hc_nodes
 */
void hc_build_tree(hc_node_list*);

/**
 * Traverses a tree of hc_nodes and assigns codes;
 */
void hc_assign_codes(hc_node_list*);

/**
 * Prints the contents of an hc_node_list to standard output.
 *
 * Params:
 *   hc_node_list - a reference to the list to print
 */
void hc_print_list(hc_node_list*);

/**
 * Prints the contents of a tree of hc_nodes to standard output.
 *
 * Params:
 *   hc_node_list - a reference to the tree to print
 */
void hc_print_tree(hc_node_list*);

/**
 * Creates a new bit string
 *
 * Returns:
 *   hc_bitstring - a new bit string
 */
hc_bitstring* hc_create_bitstring();

/**
 * Frees the resources allocated for an bit string.
 *
 * Params:
 *   hc_bitstring - the bit string to destroy
 */
void hc_destroy_bitstring(hc_bitstring*);

/**
 * Adds a bit to an bit string
 *
 * Params:
 *   hc_bitstring - the bit string to receive the bit
 *   unsigned int - the bit to insert into the bit string
 */
void hc_add_bit(hc_bitstring*, unsigned int);

/**
 * Appends the contents of one bit string onto another
 *
 * Params:
 *   hc_bitstring - the bit string to receive the bits
 *   hc_bitstring - the source of the bits to insert
 */
void hc_add_bits(hc_bitstring*, hc_bitstring*);

/**
 * Removes the last bit from a bit string
 *
 * Params:
 *   hc_bitstring - the bit string whose last bit will be removed
 */
void hc_remove_bit(hc_bitstring*);

/**
 * Prints the contents of an hc_bistring to standard output.
 *
 * Params:
 *   hc_bitstring - the bit string to print
 */
void hc_print_bitstring(hc_bitstring*);

/**
 * Converts the contents of a file to a bit string
 * using a table of bit codes.
 *
 * Params:
 *   FILE - the input file
 *   hc_sym - the bit code table
 *   unsigned long - the number of items in the bit code table
 *
 * Returns:
 *   hc_bitstring - a bit string containing the encoded data
 */
hc_bitstring* hc_prepare_output(FILE*, hc_sym*, hc_ulong);

/**
 * Writes a bit code dictionary to a file
 *
 * Params:
 *   FILE - the output file
 *   hc_sym - the bit code dictionary
 *   unsigned long - the number of elements in the bit code dictionary
 */
void hc_write_table(FILE*, hc_sym*, hc_ulong);

/**
 * Reads a bit code dictionary from a file
 *
 * Params:
 *   FILE - the input file
 *   size_t - reference to the length of the returned dictionary
 *
 * Returns:
 *   hc_sym - the bit code dictionary
 */
hc_sym* hc_read_table(FILE*, size_t*);

/**
 * Writes a bit string containing encoded data to a file
 *
 * Params:
 *   FILE - the output file
 *   hc_bitstring - the bit string to write
 */
void hc_write_data(FILE*, hc_bitstring*);

/**
 * Reads a bit string containing encoded data frp, a file
 *
 * Params:
 *   FILE - the output file
 *
 * Returns:
 *   hc_bitstring - a new bit string containing encoded data
 */
hc_bitstring* hc_read_data(FILE*);

/**
 * Reconstructs a Huffman tree from a bit code dictionary
 *
 * Params:
 *   hc_sym - the bit code dictionary
 *
 * Returns:
 *   hc_node_list - a Huffman tree
 *   hc_ulong - the number of elements in the dictionary
 */
hc_node_list* hc_reconstruct_tree(hc_sym*, hc_ulong);

/**
 * Decodes a bit string and writes the output to a file
 *
 * Params:
 *   hc_bitstring - the data to decode
 *   hc_node_list - the Huffman tree
 *   FILE - the output stream
 */
void hc_decode_bitstring(hc_bitstring*, hc_node_list*, FILE*);

#endif
