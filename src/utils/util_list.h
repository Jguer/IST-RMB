#pragma once
#include "utils.h"

typedef struct _stack *list;
typedef struct _stack *stack;
typedef struct _node *node;

/* LIST */
list create_list();
node get_head(list got_list);
void push_item_to_list(list got_list, item new_item);
void push_node_to_list(list got_list, node got_node);
size_t get_list_size(list got_list);
void print_list(list got_list, void (*print_item)(item));
void free_list(list got_list, void (*free_item)(item));
void merge_lists(list list_a, list list_b);
void dec_size_list(list got_list);

/* NODE */
node create_node(item new_item, node next_node);
node get_next_node(node got_node);
item get_node_item(node got_node);
void free_node(node got_node, void (*free_item)(item));
void free_connected_nodes(node got_node, void (*free_item)(item));
void remove_head(list got_list, void (*free_item)(item));
void remove_next_node(list got_list, node cur_node, void (*free_item)(item));


