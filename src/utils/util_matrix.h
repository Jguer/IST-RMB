#include "utils.h"

typedef struct _matrix *matrix;

// Gets
bool   get_overflow(matrix this);
size_t get_capacity(matrix this);
size_t get_size(matrix this);
item   get_element(matrix this, uint_fast32_t index);

// Methods
matrix create_matrix(size_t capacity);
void   add_element(matrix this, uint_fast32_t index, item to_add, void (*free_item)(item));
void   print_matrix(matrix this, void (*print_item)(item));
void   free_matrix(matrix this, void (*free_item)(item));
