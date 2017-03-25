#include "util_list.h"

struct _stack {
    size_t size;
    node head;
};

struct _node {
    item this;
    node next;
};

/* LIST */
list create_list() {
    list new_list = NULL;

    /* Create List */
    new_list = (list)malloc(sizeof(struct _stack));
    if(!new_list)
        memory_error("Unable to reserve list memory");

    /* Set head to null */
    new_list->head = NULL;

    /* Set size to 0 */
    new_list->size = 0;

    return new_list;
}

node get_head(list got_list) {
    return got_list->head;
}

void set_head(list got_list, node got_head) {
    got_list->head = got_head;
    return;
}

void push_item_to_list(list got_list, item new_item) {
    /* Node Creation */
    node new_node = NULL;
    new_node = create_node(new_item, NULL);

    /* Prepend node to list*/
    push_node_to_list(got_list, new_node);
    return;
}

void push_node_to_list(list got_list, node got_node) {
    got_node->next = got_list->head;
    got_list->head = got_node;

    /* Increase List size */
    got_list->size ++;

    return;
}

size_t get_list_size(list got_list) {
    if (!got_list) {
        return -1;
    }

    return got_list->size;
}

void print_list(list got_list, void (*print_item)(item)) {
    node aux_node;
    printf("Print list:\n");
    fprintf(stdout, "Size of list: %zu\n", get_list_size(got_list));

    for(aux_node = get_head(got_list);
            aux_node != NULL;
            aux_node = get_next_node(aux_node))
    {
        printf("[ ");
        print_item(get_node_item(aux_node));
        printf("] -> \n");
    }

    printf("-> [ " KBLU "NULL" RESET " ]\n");

}

void free_list(list got_list, void (*free_item)(item)) {
    if (!got_list) {
        return;
    }

    node got_node = get_head(got_list);

    /* Free every node of list*/
    free_connected_nodes(got_node, free_item);

    /* Bring freedom to stack */
    free(got_list);

    return;
}

void dec_size_list(list got_list) {
    got_list->size --;
    return;
}

/* NODE */
node create_node(item new_item, node next_node) {
    node new_node = NULL;

    /* Memory allocation */
    new_node = (node)malloc(sizeof(struct _node));

    if (!new_node)
        memory_error("Unable to reserve node memory");

    /* Add item to node*/
    new_node->this = new_item;
    new_node->next = next_node;

    return new_node;
}

item get_node_item(node got_node) {
    return got_node->this;
}

node get_next_node(node got_node) {
    return got_node->next;
}

void free_node(node got_node, void (*free_item)(item)) {
    if (!got_node) {
        return;
    }
    /* Free node item */
    free_item(got_node->this);

    /* Free the node to save on the load */
    free(got_node);

    return;
}

void free_connected_nodes(node got_node, void (*free_item)(item)) {
    node aux_node = NULL;

    /* Free every node connected*/
    while(got_node) {
        aux_node = get_next_node(got_node);
        free_node(got_node, free_item);
        got_node = aux_node;
    }
    return;
}

void remove_head(list got_list, void (*free_item)(item)) {
    if (!got_list->head) {
        return;
    }

    node aux_node = got_list->head;
    got_list->head = aux_node->next;
    free_node(aux_node, free_item);
    got_list->size--;
    return;
}


void remove_next_node(list got_list, node cur_node, void (*free_item)(item)){
    if (!cur_node) {
        return;
    }
    if (!cur_node->next) {
        return;
    }
    node aux_node = cur_node->next;

    cur_node->next = aux_node->next;
    free_node(aux_node, free_item);
    got_list->size--;
    return;
}

void merge_lists(list list_a, list list_b) {
    node aux_node;
    node final_head;
    size_t final_size;

    final_size = list_a->size + list_b->size;

    if(!list_a->head) {
        list_a->head = list_b->head;
        list_b->head = NULL;
        return;
    } else if(!list_b->head) {
        return;
    }

    if(list_a->size < list_b->size) {
        aux_node = list_a->head;
        final_head = list_b->head;
        list_a->size = final_size;
    } else {
        aux_node = list_b->head;
        final_head = list_a->head;
        list_b->size = final_size;
        list_a->head = list_b->head;
    }

    while(aux_node->next) {
        aux_node = get_next_node(aux_node);
    }

    list_b->head = NULL;
    aux_node->next = final_head;
    return;
}

void for_each_element(list got_list, void (*action)(item obj, void *cnt_array[]), void *cnt_array[]) {
    if (NULL != got_list) {
        node aux_node;
        for (aux_node = get_head(got_list); aux_node != NULL;
                aux_node = get_next_node(aux_node)) {
            action(get_node_item(aux_node),cnt_array);
        }
    }
}

void already_free(item got_item) {
    if(got_item)
        return;
    return;
}

