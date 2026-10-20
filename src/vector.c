#include <ds_lib/vector.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define VECTOR_INIT_CAP 10

struct vector_t {
    void **vector;
    size_t len;
    size_t dim;
};

vector_t * vector_create() {
    vector_t *v;

    v = malloc(sizeof(vector_t));
    if (!v) return NULL;

    v->vector = malloc(sizeof(void *) * VECTOR_INIT_CAP);
    if (!v->vector) {
        free(v);
        return NULL;
    }

    v->dim = VECTOR_INIT_CAP;
    v->len = 0;

    return v;
}

vector_t * vector_create(size_t dim) {
    vector_t *v;

    if (dim > SIZE_MAX / sizeof(void *))
        return NULL;

    v = malloc(sizeof(vector_t));
    if (!v) return NULL;

    v->vector = malloc(sizeof(void *) * dim);
    if (!v->vector) {
        free(v);
        return NULL;
    }

    v->dim = dim;
    v->len = 0;

    return v;
}

static vector_status_t vector_expand(vector_t * v) {
    // Check overflow
    if (v->dim > SIZE_MAX / 2 / sizeof(void *)) return VECT_ERR_OVERFLOW;

    void **tmp;
    tmp = realloc(v->vector, sizeof(void *) * v->dim * 2);
    if (!tmp) return VECT_ERR_REALLOC;

    v->vector = tmp;
    v->dim = v->dim * 2;

    return VECT_SUCCESS;
}

static vector_status_t vector_check_cap(vector_t * v) {
    vector_status_t res;
    // Check capacity
    if (v->len == v->dim) {
        if ((res = vector_expand(v)) < 0)
            return res;
    }

    return VECT_SUCCESS;
}

/**
 * Insert
 */

vector_status_t vector_append(vector_t * v, void * entry) {
    if (!v || !entry || !v->vector) return VECT_ERR_NULL_PTR;
    vector_status_t res;

    // Check capacity
    if ((res = vector_check_cap(v)) < 0)
        return res;

    v->vector[v->len] = entry;
    v->len = v->len + 1;

    return VECT_SUCCESS;
}

vector_status_t vector_prepend(vector_t * v, void * entry) {
    if (!v || !entry || !v->vector) return VECT_ERR_NULL_PTR;
    vector_status_t res;

    // Check capacity
    if ((res = vector_check_cap(v)) < 0)
        return res;

    memmove(v->vector + 1, v->vector, sizeof(void *) * v->len);
    v->vector[0] = entry;
    v->len = v->len + 1;

    return VECT_SUCCESS;
}

vector_status_t vector_insert_at(vector_t * v, size_t i, void * entry) {
    if (!v || !entry || !v->vector) return VECT_ERR_NULL_PTR;
    if (i > v->len) return VECT_ERR_OUT_OF_RANGE;

    vector_status_t res;

    // Check capacity
    if ((res = vector_check_cap(v)) < 0)
        return res;

    memmove(v->vector + i + 1, v->vector + i, sizeof(void *) * (v->len - i));
    v->vector[i] = entry;
    v->len = v->len + 1;
    
    return VECT_SUCCESS;
}

vector_status_t vector_replace_at(vector_t * v, size_t i, void * entry) {
    if (!v || !entry || !v->vector) return VECT_ERR_NULL_PTR;
    if (i >= v->len) return VECT_ERR_OUT_OF_RANGE;

    v->vector[i] = entry;

    return VECT_SUCCESS;
}

/**
 * Getter
 */

void * vector_at(const vector_t * v, size_t i) {
    if (!v || !v->vector || i >= v->len) return NULL;

    return v->vector[i];
}

void * vector_head(const vector_t * v) {
    if (!v || !v->vector || v->len == 0) return NULL;

    return v->vector[0];
}

void * vector_tail(const vector_t * v) {
    if (!v || !v->vector || v->len == 0) return NULL;

    return v->vector[v->len - 1];
}

int vector_is_empty(const vector_t * v) {
    return !v || v->len == 0;
}

size_t vector_get_len(const vector_t * v) {
    return v ? v->len : 0;
}

size_t vector_get_cap(const vector_t * v) {
    return v ? v->dim : 0;
}

/**
 * Delete
 */

void * vector_pop_back(vector_t * v) {
    if (!v || !v->vector || v->len == 0) return NULL;

    v->len = v->len - 1;
    void *ret = v->vector[v->len];
    v->vector[v->len] = NULL;

    return ret;
}

void * vector_pop_front(vector_t * v) {
    if (!v || !v->vector || v->len == 0) return NULL;

    void *ret = v->vector[0];

    memmove(v->vector, v->vector + 1, sizeof(void *) * (v->len - 1));
    v->len = v->len - 1;
    v->vector[v->len] = NULL;

    return ret;
}

void * vector_remove_at(vector_t * v, size_t i) {
    if (!v || !v->vector || i >= v->len) return NULL;

    void *ret = v->vector[i];

    memmove(v->vector + i, v->vector + i + 1, sizeof(void *) * (v->len - i - 1));
    v->len = v->len - 1;

    return ret;
}

vector_status_t vector_destroy(vector_t ** v, ds_free_fn free_v) {
    if (!v || !*v) return VECT_ERR_NULL_PTR;

    if (free_v && (*v)->vector) {
        for (size_t i = 0; i < (*v)->len; i++) {
            free_v((*v)->vector[i]);
        }
    }

    free((*v)->vector);
    free(*v);
    *v = NULL;

    return VECT_SUCCESS;
}

/* Search */
int vector_find(const vector_t * v, const void * entry, ds_compare_fn compare) {
    if (!v || !entry || !compare || !v->vector) return -1;

    for (size_t i = 0; i < v->len; i++) {
        if (compare(v->vector[i], entry) == 0) 
            return (int)i;
    }
    return -1;
}