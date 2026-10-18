/**
 * @file common.h
 * @brief Types shared by all the ds_lib data structures.
 *
 * ds_lib stores user data as `void *` pointers: the structures never copy
 * the items, they only keep their address. See each structure's header for
 * who owns what.
 */

#ifndef DS_LIB_COMMON_H
#define DS_LIB_COMMON_H

/**
 * @brief Comparison function, same contract as the one used by qsort().
 *
 * @return A negative value if @p a < @p b, zero if equal, a positive value
 *         if @p a > @p b.
 */
typedef int (*ds_compare_fn)(const void *a, const void *b);

/**
 * @brief Function used to release one item (e.g. free()).
 *
 * Passed to destroy functions when the structure should also release the
 * items it still holds. Pass NULL to leave the items untouched.
 */
typedef void (*ds_free_fn)(void *item);

#endif /* DS_LIB_COMMON_H */
