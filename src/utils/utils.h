#ifndef UTILSH
#define UTILSH

#include <stdio.h>
#include <stdlib.h>

#define KNRM  "\x1B[0m"		//Terminal color codes
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"
#define STRING_SIZE 128
#define RESPONSE_SIZE 512

typedef void *item;
typedef struct _stack list;
typedef struct _stack stack;
typedef struct _node node;

/* LIST */
list *create_list();
node *get_head(list *got_list);
void push_item_to_list(list *got_list, item new_item);
void push_node_to_list(list *got_list, node *got_node);
size_t get_list_size(list *got_list);
void print_list(list *got_list, void (*print_item)(item));
void free_list(list *got_list, void (*free_item)(item));
void merge_lists(list *list_a, list *list_b);

/* NODE */
node *create_node(item new_item, node *next_node);
node *get_next_node(node *got_node);
item get_node_item(node *got_node);
void free_node(node *got_node, void (*free_item)(item));
void free_connected_nodes(node *got_node, void (*free_item)(item));
void remove_next_node(node *cur_node, node * next_node, void (*free_item)(item));

/* UTILS */
void memory_error(char *msg);

#endif
