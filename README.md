# ds_lib

A small collection of generic data structures in C11.

Items are stored as `void *`: the structures never copy your data, they only
keep its address. Ordering and release of items are customised with function
pointers (`ds_compare_fn`, `ds_free_fn`, see `include/ds_lib/common.h`).

## Structures

| Structure | Header | Notes |
|-----------|--------|-------|
| Min priority queue (binary heap) | `ds_lib/min_queue.h` | Not stable; grows automatically |

## Build

```sh
make          # build/libds_lib.a
make test     # build and run all tests
make asan     # clean rebuild with AddressSanitizer + UBSan, then run tests
make install  # PREFIX=/usr/local by default
```

## Example

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

Link with `-Iinclude` and `build/libds_lib.a`.

## Ownership rules

- The structure never frees items on its own.
- Items returned by `extract_min` belong to the caller.
- `min_queue_destroy(&q, free_fn)` calls `free_fn` on the items still inside;
  pass `NULL` if you manage them yourself.
