/**
 * @file vector.h
 * @brief Dynamic array of generic pointers.
 * 
 * @author lcky00
 *
 * The vector stores @b pointers, not copies of the pointed-to data. It grows
 * automatically, and elements are always kept
 * contiguous in positions @c [0, len).
 *
 * @par Ownership
 * The vector never allocates, copies or frees the items it holds, unless a
 * ds_free_fn is explicitly passed to vector_destroy(). Items removed with
 * vector_pop_back(), vector_pop_front() or vector_remove_at() are handed
 * back to the caller, who becomes responsible for them again. Every stored
 * pointer must stay valid for as long as it is in the vector.
 *
 * @par NULL items
 * @c NULL is not a valid item and is rejected by every insertion function.
 * For this reason, a @c NULL returned by a getter or by a removal function
 * always means "no item" and is never ambiguous.
 *
 * @par Thread safety
 * None. Concurrent access to the same vector must be synchronised by the
 * caller.
 */

#ifndef DS_LIB_VECTOR_H
#define DS_LIB_VECTOR_H

#include <stddef.h>
#include <ds_lib/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Status codes returned by the vector functions.
 *
 * Success is zero; every error is negative, so `status < 0` is a valid
 * error test.
 */
typedef enum {
    VECT_SUCCESS            =  0,
    VECT_ERR_NULL_PTR       = -1,
    VECT_ERR_REALLOC        = -2,
    VECT_ERR_OVERFLOW       = -3,
    VECT_ERR_INVLD_LEN      = -4,
    VECT_ERR_OUT_OF_RANGE   = -5
} vector_status_t;

/**
 * @brief Opaque vector handle.
 *
 * The layout is private to the implementation; use only the functions below.
 */
typedef struct vector_t vector_t;

/* Insert */

/**
 * @brief Creates an empty vector.
 *
 * The initial capacity is 10 items. The vector must be released with
 * vector_destroy().
 *
 * @return A new vector, or @c NULL if memory allocation fails.
 */
vector_t * vector_create();

/**
 * @brief Adds an item at the end of the vector.
 *
 * Amortised O(1). Grows the vector if it is full.
 *
 * @param[in,out] v      The vector.
 * @param[in]     entry  The item to store. Must not be @c NULL.
 *
 * @retval VECT_SUCCESS        Item appended.
 * @retval VECT_ERR_NULL_PTR   @p v or @p entry is @c NULL.
 * @retval VECT_ERR_REALLOC    Growing failed; the vector is unchanged.
 * @retval VECT_ERR_OVERFLOW   The capacity cannot grow further.
 */
vector_status_t vector_append(vector_t * v, void * entry);

/**
 * @brief Adds an item at the beginning of the vector.
 *
 * Every existing item is shifted by one position. If this is used
 * often, consider whether a different structure fits better.
 *
 * @param[in,out] v      The vector.
 * @param[in]     entry  The item to store. Must not be @c NULL.
 *
 * @retval VECT_SUCCESS        Item prepended.
 * @retval VECT_ERR_NULL_PTR   @p v or @p entry is @c NULL.
 * @retval VECT_ERR_REALLOC    Growing failed; the vector is unchanged.
 * @retval VECT_ERR_OVERFLOW   The capacity cannot grow further.
 */
vector_status_t vector_prepend(vector_t * v, void * entry);

/**
 * @brief Inserts an item at a given position.
 *
 * Items from position @p i onward are shifted one place towards the end.
 * Using @p i equal to the current length is allowed and behaves
 * like vector_append().
 *
 * @param[in,out] v      The vector.
 * @param[in]     i      Target position, in the range @c [0, len].
 * @param[in]     entry  The item to store. Must not be @c NULL.
 *
 * @retval VECT_SUCCESS            Item inserted.
 * @retval VECT_ERR_NULL_PTR       @p v or @p entry is @c NULL.
 * @retval VECT_ERR_OUT_OF_RANGE   @p i is greater than the current length.
 * @retval VECT_ERR_REALLOC        Growing failed; the vector is unchanged.
 * @retval VECT_ERR_OVERFLOW       The capacity cannot grow further.
 */
vector_status_t vector_insert_at(vector_t * v, size_t i, void * entry);

/**
 * @brief Overwrites the item at a given position.
 *
 * The previous item is @b not freed and is not returned: if it is
 * owned by the caller, keep a copy of the pointer (see vector_at()) before
 * replacing it, otherwise it will be leaked.
 *
 * @param[in,out] v      The vector.
 * @param[in]     i      Position to overwrite, in the range @c [0, len).
 * @param[in]     entry  The new item. Must not be @c NULL.
 *
 * @retval VECT_SUCCESS            Item replaced.
 * @retval VECT_ERR_NULL_PTR       @p v or @p entry is @c NULL.
 * @retval VECT_ERR_OUT_OF_RANGE   @p i is not smaller than the current length.
 */
vector_status_t vector_replace_at(vector_t * v, size_t i, void * entry);

/* Getter */

/**
 * @brief Returns the item at a given position without removing it.
 *
 * The returned pointer is the one stored in the vector, not a copy.
 *
 * @param[in] v  The vector.
 * @param[in] i  Position, in the range @c [0, len).
 *
 * @return The item, or @c NULL if @p v is @c NULL or @p i is out of range.
 */
void * vector_at(const vector_t * v, size_t i);

/**
 * @brief Returns the first item without removing it.
 *
 * @param[in] v  The vector.
 *
 * @return The first item, or @c NULL if @p v is @c NULL or empty.
 */
void * vector_head(const vector_t * v);

/**
 * @brief Returns the last item without removing it.
 *
 * @param[in] v  The vector.
 *
 * @return The last item, or @c NULL if @p v is @c NULL or empty.
 */
void * vector_tail(const vector_t * v);

/**
 * @brief Tells whether the vector contains no items.
 *
 * @param[in] v  The vector.
 *
 * @return Non-zero if the vector is empty @b or @p v is @c NULL, zero
 *         otherwise.
 */
int vector_is_empty(const vector_t * v);

/**
 * @brief Returns the number of items currently stored.
 *
 * @param[in] v  The vector.
 *
 * @return The number of items, or 0 if @p v is @c NULL.
 */
size_t vector_get_len(const vector_t * v);

/**
 * @brief Returns the current capacity, in items.
 *
 * The capacity is the number of items that fit before the next
 * reallocation. It is always greater than or equal to the length.
 *
 * @param[in] v  The vector.
 *
 * @return The capacity, or 0 if @p v is @c NULL.
 */
size_t vector_get_cap(const vector_t * v);

/* Delete */

/**
 * @brief Removes and returns the last item.
 *
 * O(1). The item is not freed: ownership passes to the caller.
 *
 * @param[in,out] v  The vector.
 *
 * @return The removed item, or @c NULL if @p v is @c NULL or empty.
 */
void * vector_pop_back(vector_t * v);

/**
 * @brief Removes and returns the first item.
 *
 * O(n): the remaining items are shifted by one position. The item is not
 * freed: ownership passes to the caller.
 *
 * @param[in,out] v  The vector.
 *
 * @return The removed item, or @c NULL if @p v is @c NULL or empty.
 */
void * vector_pop_front(vector_t * v);

/**
 * @brief Removes and returns the item at a given position.
 *
 * O(n - i): the following items are shifted one place towards the start.
 * The item is not freed: ownership passes to the caller.
 *
 * @param[in,out] v  The vector.
 * @param[in]     i  Position, in the range @c [0, len).
 *
 * @return The removed item, or @c NULL if @p v is @c NULL or @p i is out
 *         of range.
 */
void * vector_remove_at(vector_t * v, size_t i);

/**
 * @brief Destroys a vector and releases its memory.
 *
 * Frees the internal buffer and the vector itself, then sets @c *v to
 * @c NULL so the caller's pointer cannot be reused by mistake.
 *
 * If @p free_v is not @c NULL, it is called once for each item still stored
 * (in order, from the first to the last). If it is @c NULL, the items are
 * left untouched and remain the caller's responsibility. Items previously
 * removed with vector_pop_back(), vector_pop_front() or vector_remove_at()
 * are no longer in the vector and are never passed to @p free_v.
 *
 * @param[in,out] v       Address of the vector pointer. Set to @c NULL on
 *                        success.
 * @param[in]     free_v  Optional function that releases a single item
 *                        (e.g. @c free). May be @c NULL.
 *
 * @retval VECT_SUCCESS       Vector destroyed.
 * @retval VECT_ERR_NULL_PTR  @p v or @c *v is @c NULL; nothing is done.
 */
vector_status_t vector_destroy(vector_t ** v, ds_free_fn free_v);

/* Search */

/**
 * @brief Finds the first item equal to @p entry.
 *
 * Items are visited from the first to the last. For each item,
 * @c compare(item, entry) is called; the first one for which it returns 0
 * is a match (qsort-style contract: see ds_compare_fn).
 *
 * @param[in] v        The vector.
 * @param[in] entry    The value to look for. Must not be @c NULL.
 * @param[in] compare  Comparison function. Must not be @c NULL.
 *
 * @return The zero-based index of the first match, or -1 if no item
 *         matches @b or if any argument is @c NULL. The two cases cannot be
 *         told apart from the return value. Indexes above @c INT_MAX are
 *         not representable and must not be expected from this function.
 */
int vector_find(const vector_t * v, const void * entry, ds_compare_fn compare);

#ifdef __cplusplus
}
#endif


#endif /* DS_LIB_VECTOR_H */