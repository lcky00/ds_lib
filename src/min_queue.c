#include <ds_lib/min_queue.h>
#include <stdlib.h>
#include <stdint.h>

#define MINQ_INIT_CAP 10

struct min_queue_t{
    void **queue;
    size_t capacity;
    size_t len;
    ds_compare_fn compare;
};

static size_t min_queue_get_parent(size_t i) {
    return (i - 1) / 2;
}

static size_t min_queue_left_child(size_t i) {
    return 2 * i + 1;
}

static size_t min_queue_right_child(size_t i) {
    return 2 * i + 2;
}

static int valid_index(const min_queue_t *q, size_t i) {
    return i < q->len;
}

static void min_queue_heapify_up(min_queue_t *q) {   
    size_t i = q->len - 1;

    while (i > 0) {
        size_t parent = min_queue_get_parent(i);

        // Se non è più piccolo
        if (q->compare(q->queue[i], q->queue[parent]) >= 0) break;

        // Swap
        void * tmp = q->queue[i];
        q->queue[i] = q->queue[parent];
        q->queue[parent] = tmp;

        i = parent;
    }
}

static void min_queue_heapify_down(min_queue_t *q) {
    size_t i = 0;

    while (1) {
        size_t left  = min_queue_left_child(i);
        size_t right = min_queue_right_child(i);
        
        size_t min_idx = i;

        if (valid_index(q, left) && q->compare(q->queue[left], q->queue[min_idx]) < 0)
            min_idx = left;
        
        if (valid_index(q, right) && q->compare(q->queue[right], q->queue[min_idx]) < 0)
            min_idx = right;

        if (min_idx == i) break;

        // Swap
        void * tmp = q->queue[i];
        q->queue[i] = q->queue[min_idx];
        q->queue[min_idx] = tmp;

        i = min_idx;
    }
}

min_queue_t* min_queue_create(ds_compare_fn compare) {
    if (!compare) return NULL;

    min_queue_t *q;

    // Alloco min queue struct
    q = malloc(sizeof(min_queue_t));
    if (!q) return NULL;

    q->capacity = MINQ_INIT_CAP;
    q->len = 0;
    q->compare = compare;

    q->queue = malloc(sizeof(void *) * MINQ_INIT_CAP);
    if (!q->queue) {
        free(q);
        return NULL;
    }

    return q;
}

min_queue_status_t min_queue_insert(min_queue_t *q, void *entry) {
    if (!q || !entry) return MINQ_ERR_NULL_PTR;
    
    if (q->capacity == q->len) {
        if (q->capacity > SIZE_MAX / 2 / sizeof(void *)) return MINQ_ERR_OVERFLOW;
        void **new_q;
        new_q = realloc(q->queue, sizeof(void *) * q->capacity * 2);
        if (!new_q) {
            return MINQ_ERR_REALLOC_QUEUE;
        }
        q->queue = new_q;
        q->capacity = q->capacity * 2; 
    }

    q->queue[q->len++] = entry;
    min_queue_heapify_up(q);

    return MINQ_SUCCESS;
}

void * min_queue_extract_min(min_queue_t *q) {
    if (!q || q->len == 0) return NULL;
    
    void *out = q->queue[0];
    q->len--;
    if (q->len > 0) {
        q->queue[0] = q->queue[q->len];
        min_queue_heapify_down(q);
    }
    return out;
}

void * min_queue_get_min(const min_queue_t *q) {
    if (!q || q->len == 0) return NULL;

    return q->queue[0];
}

size_t min_queue_get_len(const min_queue_t *q) {
    if (!q) return 0;

    return q->len;
}

size_t min_queue_get_cap(const min_queue_t *q) {
    if (!q) return 0;

    return q->capacity;
}

int min_queue_is_empty(const min_queue_t *q) {
    return !q || q->len == 0;
}

void min_queue_destroy(min_queue_t **q, ds_free_fn free_item) {
    if (!q || !*q) return;

    if (free_item) {
        for (size_t i = 0; i < (*q)->len; i++)
            free_item((*q)->queue[i]);
    }

    free((*q)->queue);
    free(*q);
    *q = NULL;
}
