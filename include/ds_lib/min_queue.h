/**
 * @file min_queue.h
 * @brief Generic min priority queue (binary heap) storing void pointers.
 *
 * The queue stores pointers to user data and orders them with a
 * user-supplied comparator. It never copies or modifies the items. The
 * caller owns them, unless a free function is passed to
 * min_queue_destroy().
 *
 * @author lcky00
 *
 * @note Not thread-safe. Synchronize externally if shared between threads.
 * @note The queue is NOT stable: items that compare equal are extracted in
 *       an unspecified order (not necessarily insertion order). If you need
 *       FIFO order among equals, add a sequence number to your items and
 *       use it as a tie-breaker in the comparator.
 */

#ifndef DS_LIB_MIN_QUEUE_H
#define DS_LIB_MIN_QUEUE_H

#include <stddef.h>
#include <ds_lib/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Status codes returned by functions that can fail. */
typedef enum {
    MINQ_SUCCESS = 0,              /**< Operation completed. */
    MINQ_ERR_NULL_PTR = -1,        /**< The queue or the item is NULL. */
    MINQ_ERR_REALLOC_QUEUE = -2,   /**< Out of memory while growing the queue. */
    MINQ_ERR_OVERFLOW = -3         /**< Maximum capacity reached. */
} min_queue_status_t;

/** @brief Opaque min queue type. */
typedef struct min_queue_t min_queue_t;

/**
 * @brief Creates an empty queue.
 *
 * @param compare Comparison function (must not be NULL). The item for which
 *                the comparator says "smaller" is extracted first.
 * @return A new queue, or NULL if @p compare is NULL or memory allocation
 *         fails. Release it with min_queue_destroy().
 *
 * @warning The comparator must define a consistent ordering, and the keys
 *          of items already in the queue must not change while they are
 *          inside it, otherwise the heap property is broken.
 */
min_queue_t * min_queue_create(ds_compare_fn compare);

/**
 * @brief Inserts an item.
 *
 * The queue grows automatically (capacity doubles when full).
 *
 * @param q     The queue.
 * @param entry Item to insert. Must not be NULL.
 * @retval MINQ_SUCCESS          Item inserted.
 * @retval MINQ_ERR_NULL_PTR     @p q or @p entry is NULL.
 * @retval MINQ_ERR_OVERFLOW     Maximum capacity reached.
 * @retval MINQ_ERR_REALLOC_QUEUE Out of memory. The queue is unchanged.
 */
min_queue_status_t min_queue_insert(min_queue_t *q, void *entry);

/**
 * @brief Removes and returns the smallest item.
 *
 * @param q The queue.
 * @return The smallest item, or NULL if the queue is empty or @p q is NULL.
 *         Since NULL items are not allowed, NULL always means "no item".
 *         The caller becomes responsible for the returned pointer.
 */
void * min_queue_extract_min(min_queue_t *q);

/**
 * @brief Returns the smallest item without removing it.
 *
 * @param q The queue.
 * @return The smallest item, or NULL if the queue is empty or @p q is NULL.
 */
void * min_queue_get_min(const min_queue_t *q);

/**
 * @brief Returns the number of items in the queue.
 * @param q The queue.
 * @return The number of items, or 0 if @p q is NULL.
 */
size_t min_queue_get_len(const min_queue_t *q);

/**
 * @brief Returns the current capacity (items that fit without reallocation).
 * @param q The queue.
 * @return The capacity, or 0 if @p q is NULL.
 */
size_t min_queue_get_cap(const min_queue_t *q);

/**
 * @brief Checks whether the queue is empty.
 * @param q The queue.
 * @return Non-zero if the queue is empty or @p q is NULL, 0 otherwise.
 */
int min_queue_is_empty(const min_queue_t *q);

/**
 * @brief Destroys the queue and releases its memory.
 *
 * By default the items are NOT freed: the queue does not own them. If
 * @p free_item is not NULL it is called once on every item still in the
 * queue (in unspecified order) before the queue itself is released.
 * Items already extracted are never touched.
 *
 * @param q         Address of the queue pointer. On return, *q is set to
 *                  NULL. Does nothing if @p q or *q is NULL.
 * @param free_item Function used to release the remaining items (e.g.
 *                  free), or NULL to leave them alone.
 */
void min_queue_destroy(min_queue_t **q, ds_free_fn free_item);

#ifdef __cplusplus
}
#endif

#endif /* DS_LIB_MIN_QUEUE_H */
