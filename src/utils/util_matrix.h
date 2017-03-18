/*! \file util_matrix.h
 * \brief Matrix structure definition 
*/
#include "utils.h"

/*! \var typedef struct _matrix *matrix
    \brief Back linked matrix
    Describes a pointer to struct _matrix.
    It is backlinked so accessing 207 goes to 007.
*/
typedef struct _matrix *matrix;

// Gets
/*! \fn bool get_overflow(matrix this);
    \brief Returns if matrix has been overflowed.
    \param this Matrix selected.
*/
bool get_overflow(matrix this);


/*! \fn size_t get_capacity(matrix this);
    \brief Returns matrix capacity.
    \param this Matrix selected.
*/
size_t get_capacity(matrix this);

/*! \fn size_t get_size(matrix this);
    \brief Next vacant position.
    \param this Matrix selected.
*/
size_t get_size(matrix this);

/*! \fn item get_element(matrix this, uint_fast32_t index);
    \brief Returns matrix element in index.
    \param this Matrix selected.
    \param index Selected index.
*/
item get_element(matrix this, uint_fast32_t index);

// Methods
/*! \fn matrix create_matrix(size_t capacity)
    \brief Initializes matrix structure
    \param capacity max size
*/
matrix create_matrix(size_t capacity);

/*! \fn void add_element(matrix this, uint_fast32_t index, item to_add, void (*free_item)(item))
    \brief Adds element to array position desired.
    If needed removes existing element.
    \param this Matrix selected.
    \param index Index selected.
    \param to_add Item to add to matrix.
    \param free_item Free item function.
*/
void add_element(matrix this, uint_fast32_t index, item to_add, void (*free_item)(item));

/*! \fn void print_matrix(matrix this, void (*print_item)(item));
    \brief Print full matrix.
    \param this Matrix selected.
    \param print_item Print item function.
*/
void print_matrix(matrix this, void (*print_item)(item));

/*! \fn void free_matrix(matrix this, void (*free_item)(item));
    \brief Frees matrix and all it's elements.
    \param this Matrix selected.
    \param free_item Free item function.
*/
void free_matrix(matrix this, void (*free_item)(item));
