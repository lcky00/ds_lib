# ds_lib

A small collection of generic data structures in C11.

Items are stored as `void *`: the structures never copy your data, they only
keep its address. Ordering, comparison and release of items are customised with
function pointers (`ds_compare_fn`, `ds_free_fn`, see `include/ds_lib/common.h`).

## Structures

| Structure | Header | Notes |
|-----------|--------|-------|
| Dynamic array (vector) | `ds_lib/vector.h` | Contiguous, grows automatically (initial capacity 10, doubling); O(1) amortised append/pop_back, O(n) prepend/pop_front/insert/remove |
| Min priority queue (binary heap) | `ds_lib/min_queue.h` | Not stable; grows automatically |

## Build

```sh
make          # build/libds_lib.a
make test     # build and run all tests
make install  # PREFIX=/usr/local by default
```

## Examples

### Min queue

```c
#include <stdio.h>
#include <stdlib.h>
#include <ds_lib/min_queue.h>

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void) {
    min_queue_t *q = min_queue_create(cmp_int);

    for (int i = 0; i < 5; i++) {
        int *v = malloc(sizeof *v);
        *v = 50 - i * 7;
        min_queue_insert(q, v);
    }

    int *smallest = min_queue_extract_min(q);   /* caller owns it */
    printf("%d\n", *smallest);
    free(smallest);

    min_queue_destroy(&q, free);                /* frees the remaining items */
    return 0;
}
```

### Vector

```c
#include <stdio.h>
#include <ds_lib/vector.h>

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void) {
    int a = 10, b = 20, c = 30;
    vector_t *v = vector_create();

    vector_append(v, &a);                       /* [10]        */
    vector_append(v, &c);                       /* [10 30]     */
    vector_insert_at(v, 1, &b);                 /* [10 20 30]  */

    int target = 20;
    printf("index of 20: %d\n", vector_find(v, &target, cmp_int));

    int *last = vector_pop_back(v);             /* caller owns it again */
    printf("%d\n", *last);

    vector_destroy(&v, NULL);                   /* items are on the stack: no free_fn */
    return 0;
}
```

Link with `-Iinclude` and `build/libds_lib.a`.

## Ownership rules

- The structures never allocate, copy or free items on their own.
- Items returned by `extract_min`, `vector_pop_back`, `vector_pop_front` and
  `vector_remove_at` belong to the caller.
- `vector_replace_at` does not return the old item: keep a copy of the pointer
  (`vector_at`) first if you need to free it.
- `min_queue_destroy(&q, free_fn)` and `vector_destroy(&v, free_fn)` call
  `free_fn` on the items still inside; pass `NULL` if you manage them yourself.
  Both set your pointer to `NULL` afterwards.
- Every stored pointer must stay valid for as long as it is in the structure.

## Conventions

- `NULL` is not a valid item: insertion functions reject it, so a `NULL`
  returned by a getter or removal function always means "no item".
- Vector functions that return `vector_status_t` use `0` for success and
  negative values for errors, so `status < 0` is a valid error test.
- `vector_find` returns the zero-based index of the first match, or `-1` if
  nothing matches (or an argument is `NULL`).
- No thread safety: synchronise concurrent access to the same structure
  yourself.