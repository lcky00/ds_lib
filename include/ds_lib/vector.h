#ifndef DS_LIB_VECTOR_H
#define DS_LIB_VECTOR_H

#include <stddef.h>
#include <ds_lib/common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VECT_SUCCESS = 0,
    VECT_ERR_NULL_PTR = -1,
    VECT_ERR_REALLOC = -2,
    VECT_ERR_OVERFLOW = -3
} vector_status_t;

typedef struct vector_t vector_t;

/* Insert */
vector_t * vector_create();
vector_status_t vector_append(vector_t * v, void * entry);
vector_status_t vector_prepend(vector_t * v, void * entry);
vector_status_t vector_insert_at(vector_t * v, size_t i, void * entry);
vector_status_t vector_replace_at(vector_t * v, size_t i, void * entry);

/* Delete */
void * vector_pop_back(vector_t * v);
void * vector_pop_front(vector_t * v);
vector_status_t vector_remove_at(vector_t * v, size_t i);
vector_status_t vector_clear(vector_t * v);

/* Getter */
void * vector_at(vector_t * v, size_t i);
void * vector_head();
void * vector_tail();

int vector_is_empty(vector_t * v);
size_t vector_get_len(vector_t * v);
size_t vector_get_cap(vector_t * v);

/* Search */
int vector_find(vector_t * v, void * entry, ds_compare_fn compare);

#ifdef __cplusplus
}
#endif


#endif /* DS_LIB_VECTOR_H */