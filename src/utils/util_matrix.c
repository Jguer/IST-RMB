#include "util_matrix.h"

struct _matrix {
    item   *array;
    size_t size;
    size_t capacity;
    bool   overflow;
};

/* MATRIX */
bool get_overflow(matrix this) {
    return this->overflow;
}
size_t get_capacity(matrix this) {
    return this->capacity;
}

size_t get_size(matrix this) {
    return this->size;
}

item get_element(matrix this, uint_fast32_t index) {
    return this->array[index % this->capacity];
}

void add_element(matrix this, uint_fast32_t index, item to_add, void (*free_item)(item)) {
    index = index % this->capacity;

    if(this->array[index]) {
        free_item(this->array[index]);
        this->overflow = true;
    }

    this->array[index] = to_add;
    this->size++;
    return;
}

matrix create_matrix(size_t capacity) {
    matrix new_matrix = NULL;

    /* Create matrix */
    new_matrix = (matrix)malloc(sizeof(struct _matrix));
    if(!new_matrix)
        memory_error("Unable to reserve matrix memory");

    new_matrix->array = (item)malloc(sizeof(item) * capacity);
    if(!new_matrix->array)
        memory_error("Unable to reserve array memory");

    /* Set capacity */
    new_matrix->capacity = capacity;

    /* Set size to 0 */
    new_matrix->size = 0;

    return new_matrix;
}

void print_matrix(matrix this, void (*print_item)(item)) {
    for (uint_fast32_t i = 0; i < this->capacity; i++) {
        print_item(this->array[i]);
        fflush(stdout);
    }
}

void free_matrix(matrix this, void (*free_item)(item)) {
    if (!this) {
        return;
    }
    /* Free every item of matrix*/
    for (uint_fast32_t i = 0; i < this->capacity; i++) {
        free_item(this->array[i]);
    }

    /* Bring freedom to matrix */
    free(this);
    return;
}

