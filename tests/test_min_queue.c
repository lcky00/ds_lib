/**
 * @file test_min_queue.c
 * @brief Tests for min_queue.
 *
 * Build and run from the project root:
 *   make test    # normal build
 *   make asan    # with AddressSanitizer and UBSan
 */
#include <ds_lib/min_queue.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int checks_failed = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);       \
            checks_failed++;                                               \
        }                                                                  \
    } while (0)

#define RUN(test)                                                          \
    do {                                                                   \
        int before = checks_failed;                                        \
        tests_run++;                                                       \
        test();                                                            \
        printf("[%s] %s\n", checks_failed == before ? " OK " : "FAIL",     \
               #test);                                                     \
    } while (0)

/* ---------- comparators ---------- */

static int cmp_int_ptr(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

static int cmp_str_ptr(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

/* ---------- helpers ---------- */

/* Inserts values[0..n) into q. Returns 1 if all inserts succeeded. */
static int insert_all(min_queue_t *q, int *values, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (min_queue_insert(q, &values[i]) != MINQ_SUCCESS)
            return 0;
    }
    return 1;
}

/* Extracts everything and checks the order against a qsort'ed copy. */
static void check_drains_sorted(min_queue_t *q, const int *values, size_t n) {
    int *expected = malloc(n * sizeof *expected);
    CHECK(expected != NULL);
    if (!expected) return;

    memcpy(expected, values, n * sizeof *expected);
    qsort(expected, n, sizeof *expected, cmp_int_ptr);

    for (size_t i = 0; i < n; i++) {
        int *got = min_queue_extract_min(q);
        CHECK(got != NULL);
        if (!got) break;
        CHECK(*got == expected[i]);
    }
    CHECK(min_queue_is_empty(q));
    CHECK(min_queue_get_len(q) == 0);

    free(expected);
}

/* ---------- tests ---------- */

static void test_create_destroy(void) {
    min_queue_t *q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);
    CHECK(min_queue_get_len(q) == 0);
    CHECK(min_queue_get_cap(q) > 0);
    CHECK(min_queue_is_empty(q));

    min_queue_destroy(&q, NULL);
    CHECK(q == NULL);
}

static void test_create_null_compare(void) {
    CHECK(min_queue_create(NULL) == NULL);
}

static void test_destroy_null_safe(void) {
    min_queue_t *q = NULL;
    min_queue_destroy(&q, NULL);   /* *q == NULL */
    min_queue_destroy(NULL, NULL); /* q == NULL  */
    CHECK(q == NULL);

    /* double destroy: the second call sees NULL and does nothing */
    q = min_queue_create(cmp_int_ptr);
    min_queue_destroy(&q, NULL);
    min_queue_destroy(&q, NULL);
    CHECK(q == NULL);
}

static void test_null_arguments(void) {
    int x = 1;
    min_queue_t *q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);

    CHECK(min_queue_insert(NULL, &x) == MINQ_ERR_NULL_PTR);
    CHECK(min_queue_insert(q, NULL) == MINQ_ERR_NULL_PTR);
    CHECK(min_queue_get_len(q) == 0); /* rejected insert changes nothing */

    CHECK(min_queue_extract_min(NULL) == NULL);
    CHECK(min_queue_get_min(NULL) == NULL);
    CHECK(min_queue_get_len(NULL) == 0);
    CHECK(min_queue_get_cap(NULL) == 0);
    CHECK(min_queue_is_empty(NULL));

    min_queue_destroy(&q, NULL);
}

static void test_empty_queue(void) {
    min_queue_t *q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);

    CHECK(min_queue_extract_min(q) == NULL);
    CHECK(min_queue_get_min(q) == NULL);
    CHECK(min_queue_is_empty(q));

    min_queue_destroy(&q, NULL);
}

static void test_single_element(void) {
    int x = 42;
    min_queue_t *q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);

    CHECK(min_queue_insert(q, &x) == MINQ_SUCCESS);
    CHECK(min_queue_get_len(q) == 1);
    CHECK(!min_queue_is_empty(q));
    CHECK(min_queue_get_min(q) == &x);
    CHECK(min_queue_get_len(q) == 1); /* get_min does not remove */

    CHECK(min_queue_extract_min(q) == &x);
    CHECK(min_queue_is_empty(q));
    CHECK(min_queue_extract_min(q) == NULL);

    min_queue_destroy(&q, NULL);
}

static void test_sorted_order_small(void) {
    int v[] = {5, 3, 8, 1, 9, 2, 7};
    size_t n = sizeof v / sizeof v[0];
    min_queue_t *q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);

    CHECK(insert_all(q, v, n));
    CHECK(min_queue_get_len(q) == n);
    CHECK(*(int *)min_queue_get_min(q) == 1);
    check_drains_sorted(q, v, n);

    min_queue_destroy(&q, NULL);
}

static void test_already_sorted_and_reversed(void) {
    enum { N = 100 };
    int asc[N], desc[N];
    for (int i = 0; i < N; i++) {
        asc[i] = i;
        desc[i] = N - i;
    }

    min_queue_t *q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);

    CHECK(insert_all(q, asc, N));
    check_drains_sorted(q, asc, N);

    CHECK(insert_all(q, desc, N));
    check_drains_sorted(q, desc, N);

    min_queue_destroy(&q, NULL);
}

static void test_duplicates(void) {
    int v[] = {4, 4, 4, 1, 1, 9, 9, 4, 1, 9};
    size_t n = sizeof v / sizeof v[0];
    min_queue_t *q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);

    CHECK(insert_all(q, v, n));
    check_drains_sorted(q, v, n);

    min_queue_destroy(&q, NULL);
}

static void test_growth_beyond_initial_capacity(void) {
    enum { N = 5000 };
    int *v = malloc(N * sizeof *v);
    CHECK(v != NULL);
    if (!v) return;

    srand(12345);
    for (int i = 0; i < N; i++)
        v[i] = (int)(rand() % 1000);

    min_queue_t *q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);
    size_t initial_cap = min_queue_get_cap(q);

    CHECK(insert_all(q, v, N));
    CHECK(min_queue_get_len(q) == N);
    CHECK(min_queue_get_cap(q) > initial_cap);
    CHECK(min_queue_get_cap(q) >= min_queue_get_len(q));

    check_drains_sorted(q, v, N);

    min_queue_destroy(&q, NULL);
    free(v);
}

static void test_random_vs_qsort(void) {
    srand(2026);
    for (int round = 0; round < 50; round++) {
        size_t n = (size_t)(rand() % 300) + 1;
        int *v = malloc(n * sizeof *v);
        CHECK(v != NULL);
        if (!v) return;

        for (size_t i = 0; i < n; i++)
            v[i] = (int)(rand() % 200) - 100; /* includes negatives */

        min_queue_t *q = min_queue_create(cmp_int_ptr);
        CHECK(q != NULL);
        CHECK(insert_all(q, v, n));
        check_drains_sorted(q, v, n);

        min_queue_destroy(&q, NULL);
        free(v);
    }
}

/* Random mix of insert/extract, checked against a naive reference. */
static void test_interleaved_operations(void) {
    enum { OPS = 20000, MAXN = OPS };
    int *pool = malloc(MAXN * sizeof *pool); /* storage for inserted values */
    int *ref = malloc(MAXN * sizeof *ref);   /* reference multiset */
    CHECK(pool != NULL && ref != NULL);
    if (!pool || !ref) {
        free(pool);
        free(ref);
        return;
    }

    min_queue_t *q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);

    srand(777);
    size_t pool_n = 0, ref_n = 0;

    for (int op = 0; op < OPS; op++) {
        if (rand() % 3 != 0) { /* ~2/3 inserts */
            pool[pool_n] = (int)(rand() % 500);
            CHECK(min_queue_insert(q, &pool[pool_n]) == MINQ_SUCCESS);
            ref[ref_n++] = pool[pool_n];
            pool_n++;
        } else {
            int *got = min_queue_extract_min(q);
            if (ref_n == 0) {
                CHECK(got == NULL);
            } else {
                size_t min_i = 0;
                for (size_t i = 1; i < ref_n; i++)
                    if (ref[i] < ref[min_i]) min_i = i;

                CHECK(got != NULL);
                if (got) CHECK(*got == ref[min_i]);
                ref[min_i] = ref[--ref_n];
            }
        }
        CHECK(min_queue_get_len(q) == ref_n);
        if (checks_failed > 20) break; /* avoid flooding on a broken heap */
    }

    min_queue_destroy(&q, NULL);
    free(pool);
    free(ref);
}

static void test_strings(void) {
    char *words[] = {"pear", "apple", "zebra", "mango", "banana", "cherry"};
    const char *sorted[] = {"apple", "banana", "cherry", "mango", "pear", "zebra"};
    size_t n = sizeof words / sizeof words[0];

    min_queue_t *q = min_queue_create(cmp_str_ptr);
    CHECK(q != NULL);

    for (size_t i = 0; i < n; i++)
        CHECK(min_queue_insert(q, words[i]) == MINQ_SUCCESS);

    for (size_t i = 0; i < n; i++) {
        char *got = min_queue_extract_min(q);
        CHECK(got != NULL);
        if (got) CHECK(strcmp(got, sorted[i]) == 0);
    }
    CHECK(min_queue_is_empty(q));

    min_queue_destroy(&q, NULL);
}

static void test_reuse_after_drain(void) {
    int a[] = {3, 1, 2};
    int b[] = {30, 10, 20};
    min_queue_t *q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);

    CHECK(insert_all(q, a, 3));
    check_drains_sorted(q, a, 3);

    CHECK(insert_all(q, b, 3));
    check_drains_sorted(q, b, 3);

    min_queue_destroy(&q, NULL);
}

static void test_independent_queues(void) {
    int a[] = {5, 1, 3};
    int b[] = {50, 10, 30};
    min_queue_t *q1 = min_queue_create(cmp_int_ptr);
    min_queue_t *q2 = min_queue_create(cmp_int_ptr);
    CHECK(q1 != NULL && q2 != NULL);

    CHECK(insert_all(q1, a, 3));
    CHECK(insert_all(q2, b, 3));

    CHECK(*(int *)min_queue_get_min(q1) == 1);
    CHECK(*(int *)min_queue_get_min(q2) == 10);

    check_drains_sorted(q1, a, 3);
    CHECK(min_queue_get_len(q2) == 3); /* q2 untouched */
    check_drains_sorted(q2, b, 3);

    min_queue_destroy(&q1, NULL);
    min_queue_destroy(&q2, NULL);
}

static int free_calls = 0;

static void counting_free(void *p) {
    free_calls++;
    free(p);
}

static void test_destroy_with_free_fn(void) {
    enum { N = 100, EXTRACTED = 30 };
    min_queue_t *q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);

    for (int i = 0; i < N; i++) {
        int *p = malloc(sizeof *p);
        CHECK(p != NULL);
        if (!p) break;
        *p = (N - i) * 7 % 101;
        CHECK(min_queue_insert(q, p) == MINQ_SUCCESS);
    }

    /* extracted items belong to the caller: free them ourselves */
    for (int i = 0; i < EXTRACTED; i++)
        free(min_queue_extract_min(q));

    free_calls = 0;
    size_t left = min_queue_get_len(q);
    min_queue_destroy(&q, counting_free);
    CHECK(q == NULL);
    CHECK(free_calls == (int)left);
    CHECK(left == N - EXTRACTED);

    /* empty queue: free_fn must not be called */
    q = min_queue_create(cmp_int_ptr);
    CHECK(q != NULL);
    free_calls = 0;
    min_queue_destroy(&q, counting_free);
    CHECK(free_calls == 0);

    /* NULL queue with a free_fn: no crash */
    min_queue_destroy(&q, counting_free);
    min_queue_destroy(NULL, counting_free);
    CHECK(free_calls == 0);
}

int main(void) {
    RUN(test_create_destroy);
    RUN(test_create_null_compare);
    RUN(test_destroy_null_safe);
    RUN(test_null_arguments);
    RUN(test_empty_queue);
    RUN(test_single_element);
    RUN(test_sorted_order_small);
    RUN(test_already_sorted_and_reversed);
    RUN(test_duplicates);
    RUN(test_growth_beyond_initial_capacity);
    RUN(test_random_vs_qsort);
    RUN(test_interleaved_operations);
    RUN(test_strings);
    RUN(test_reuse_after_drain);
    RUN(test_independent_queues);
    RUN(test_destroy_with_free_fn);

    printf("\n%d tests, %d failed checks\n", tests_run, checks_failed);
    return checks_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
