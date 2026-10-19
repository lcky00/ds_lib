/**
 * @file test_vector.c
 * @brief Tests for vector.
 *
 * Build and run from the project root:
 *   make test    # normal build
 *   make asan    # with AddressSanitizer and UBSan
 */
#include <ds_lib/vector.h>

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

/* Appends &values[0..n) to v. Returns 1 if all appends succeeded. */
static int append_all(vector_t *v, int *values, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (vector_append(v, &values[i]) != VECT_SUCCESS)
            return 0;
    }
    return 1;
}

/* Checks that v holds exactly &values[0..n), in this order (by pointer). */
static void check_content(const vector_t *v, int *values, size_t n) {
    CHECK(vector_get_len(v) == n);
    for (size_t i = 0; i < n; i++) {
        CHECK(vector_at(v, i) == &values[i]);
    }
}

/* ---------- tests ---------- */

static void test_create_destroy(void) {
    vector_t *v = vector_create();
    CHECK(v != NULL);
    CHECK(vector_get_len(v) == 0);
    CHECK(vector_get_cap(v) == 10);
    CHECK(vector_is_empty(v));

    CHECK(vector_destroy(&v, NULL) == VECT_SUCCESS);
    CHECK(v == NULL);
}

static void test_destroy_null_safe(void) {
    vector_t *v = NULL;
    CHECK(vector_destroy(&v, NULL) == VECT_ERR_NULL_PTR);  /* *v == NULL */
    CHECK(vector_destroy(NULL, NULL) == VECT_ERR_NULL_PTR); /* v == NULL  */

    /* double destroy: the second call sees NULL and does nothing */
    v = vector_create();
    CHECK(v != NULL);
    CHECK(vector_destroy(&v, NULL) == VECT_SUCCESS);
    CHECK(vector_destroy(&v, NULL) == VECT_ERR_NULL_PTR);
    CHECK(v == NULL);
}

static void test_null_arguments(void) {
    int x = 1;
    vector_t *v = vector_create();
    CHECK(v != NULL);

    CHECK(vector_append(NULL, &x) == VECT_ERR_NULL_PTR);
    CHECK(vector_append(v, NULL) == VECT_ERR_NULL_PTR);
    CHECK(vector_prepend(NULL, &x) == VECT_ERR_NULL_PTR);
    CHECK(vector_prepend(v, NULL) == VECT_ERR_NULL_PTR);
    CHECK(vector_insert_at(NULL, 0, &x) == VECT_ERR_NULL_PTR);
    CHECK(vector_insert_at(v, 0, NULL) == VECT_ERR_NULL_PTR);
    CHECK(vector_get_len(v) == 0); /* rejected inserts change nothing */

    CHECK(vector_append(v, &x) == VECT_SUCCESS);
    CHECK(vector_replace_at(NULL, 0, &x) == VECT_ERR_NULL_PTR);
    CHECK(vector_replace_at(v, 0, NULL) == VECT_ERR_NULL_PTR);
    CHECK(vector_at(v, 0) == &x); /* untouched */

    CHECK(vector_at(NULL, 0) == NULL);
    CHECK(vector_head(NULL) == NULL);
    CHECK(vector_tail(NULL) == NULL);
    CHECK(vector_is_empty(NULL));
    CHECK(vector_get_len(NULL) == 0);
    CHECK(vector_get_cap(NULL) == 0);

    CHECK(vector_pop_back(NULL) == NULL);
    CHECK(vector_pop_front(NULL) == NULL);
    CHECK(vector_remove_at(NULL, 0) == NULL);

    CHECK(vector_find(NULL, &x, cmp_int_ptr) == -1);
    CHECK(vector_find(v, NULL, cmp_int_ptr) == -1);
    CHECK(vector_find(v, &x, NULL) == -1);

    vector_destroy(&v, NULL);
}

static void test_empty_vector(void) {
    int x = 1;
    vector_t *v = vector_create();
    CHECK(v != NULL);

    CHECK(vector_is_empty(v));
    CHECK(vector_at(v, 0) == NULL);
    CHECK(vector_head(v) == NULL);
    CHECK(vector_tail(v) == NULL);
    CHECK(vector_pop_back(v) == NULL);
    CHECK(vector_pop_front(v) == NULL);
    CHECK(vector_remove_at(v, 0) == NULL);
    CHECK(vector_find(v, &x, cmp_int_ptr) == -1);
    CHECK(vector_get_len(v) == 0);

    vector_destroy(&v, NULL);
}

static void test_single_element(void) {
    int x = 42;
    vector_t *v = vector_create();
    CHECK(v != NULL);

    CHECK(vector_append(v, &x) == VECT_SUCCESS);
    CHECK(vector_get_len(v) == 1);
    CHECK(!vector_is_empty(v));
    CHECK(vector_at(v, 0) == &x);
    CHECK(vector_head(v) == &x);
    CHECK(vector_tail(v) == &x);
    CHECK(vector_get_len(v) == 1); /* getters do not remove */

    CHECK(vector_pop_back(v) == &x);
    CHECK(vector_is_empty(v));
    CHECK(vector_pop_back(v) == NULL);

    CHECK(vector_prepend(v, &x) == VECT_SUCCESS);
    CHECK(vector_pop_front(v) == &x);
    CHECK(vector_is_empty(v));

    CHECK(vector_insert_at(v, 0, &x) == VECT_SUCCESS);
    CHECK(vector_remove_at(v, 0) == &x);
    CHECK(vector_is_empty(v));

    vector_destroy(&v, NULL);
}

static void test_append_order(void) {
    int a[] = {1, 2, 3, 4, 5};
    size_t n = sizeof a / sizeof a[0];
    vector_t *v = vector_create();
    CHECK(v != NULL);

    CHECK(append_all(v, a, n));
    check_content(v, a, n);
    CHECK(vector_head(v) == &a[0]);
    CHECK(vector_tail(v) == &a[n - 1]);

    vector_destroy(&v, NULL);
}

static void test_prepend_order(void) {
    int a[] = {1, 2, 3, 4, 5};
    size_t n = sizeof a / sizeof a[0];
    vector_t *v = vector_create();
    CHECK(v != NULL);

    for (size_t i = 0; i < n; i++)
        CHECK(vector_prepend(v, &a[i]) == VECT_SUCCESS);

    /* prepending 1..5 yields 5,4,3,2,1 */
    CHECK(vector_get_len(v) == n);
    for (size_t i = 0; i < n; i++)
        CHECK(vector_at(v, i) == &a[n - 1 - i]);

    vector_destroy(&v, NULL);
}

static void test_insert_at(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    vector_t *v = vector_create();
    CHECK(v != NULL);

    CHECK(vector_insert_at(v, 0, &a) == VECT_SUCCESS); /* [a]        empty, i == len */
    CHECK(vector_insert_at(v, 1, &c) == VECT_SUCCESS); /* [a c]      i == len        */
    CHECK(vector_insert_at(v, 1, &b) == VECT_SUCCESS); /* [a b c]    middle          */
    CHECK(vector_insert_at(v, 0, &d) == VECT_SUCCESS); /* [d a b c]  front           */
    CHECK(vector_insert_at(v, 2, &e) == VECT_SUCCESS); /* [d a e b c]                */

    CHECK(vector_get_len(v) == 5);
    CHECK(vector_at(v, 0) == &d);
    CHECK(vector_at(v, 1) == &a);
    CHECK(vector_at(v, 2) == &e);
    CHECK(vector_at(v, 3) == &b);
    CHECK(vector_at(v, 4) == &c);

    vector_destroy(&v, NULL);
}

static void test_insert_at_out_of_range(void) {
    int x = 1, y = 2;
    vector_t *v = vector_create();
    CHECK(v != NULL);

    CHECK(vector_insert_at(v, 1, &x) == VECT_ERR_OUT_OF_RANGE); /* len == 0 */
    CHECK(vector_get_len(v) == 0);

    CHECK(vector_append(v, &x) == VECT_SUCCESS);
    CHECK(vector_insert_at(v, 2, &y) == VECT_ERR_OUT_OF_RANGE);
    CHECK(vector_insert_at(v, (size_t)-1, &y) == VECT_ERR_OUT_OF_RANGE);
    CHECK(vector_get_len(v) == 1);
    CHECK(vector_at(v, 0) == &x);

    vector_destroy(&v, NULL);
}

static void test_replace_at(void) {
    int a = 1, b = 2, c = 3, z = 99;
    vector_t *v = vector_create();
    CHECK(v != NULL);

    CHECK(vector_append(v, &a) == VECT_SUCCESS);
    CHECK(vector_append(v, &b) == VECT_SUCCESS);
    CHECK(vector_append(v, &c) == VECT_SUCCESS);

    CHECK(vector_replace_at(v, 1, &z) == VECT_SUCCESS);
    CHECK(vector_get_len(v) == 3);
    CHECK(vector_at(v, 0) == &a);
    CHECK(vector_at(v, 1) == &z);
    CHECK(vector_at(v, 2) == &c);

    /* i == len and beyond are out of range (unlike insert_at) */
    CHECK(vector_replace_at(v, 3, &z) == VECT_ERR_OUT_OF_RANGE);
    CHECK(vector_replace_at(v, 100, &z) == VECT_ERR_OUT_OF_RANGE);
    CHECK(vector_get_len(v) == 3);

    /* on an empty vector everything is out of range */
    vector_t *e = vector_create();
    CHECK(e != NULL);
    CHECK(vector_replace_at(e, 0, &z) == VECT_ERR_OUT_OF_RANGE);

    vector_destroy(&e, NULL);
    vector_destroy(&v, NULL);
}

static void test_getters_out_of_range(void) {
    int a[] = {1, 2, 3};
    vector_t *v = vector_create();
    CHECK(v != NULL);
    CHECK(append_all(v, a, 3));

    CHECK(vector_at(v, 2) == &a[2]);
    CHECK(vector_at(v, 3) == NULL);
    CHECK(vector_at(v, 1000) == NULL);
    CHECK(vector_at(v, (size_t)-1) == NULL);

    vector_destroy(&v, NULL);
}

static void test_pop_back_front(void) {
    int a[] = {1, 2, 3, 4, 5};
    size_t n = sizeof a / sizeof a[0];
    vector_t *v = vector_create();
    CHECK(v != NULL);
    CHECK(append_all(v, a, n));

    CHECK(vector_pop_back(v) == &a[4]);
    CHECK(vector_pop_front(v) == &a[0]);
    CHECK(vector_get_len(v) == 3);
    CHECK(vector_head(v) == &a[1]);
    CHECK(vector_tail(v) == &a[3]);

    CHECK(vector_pop_front(v) == &a[1]);
    CHECK(vector_pop_back(v) == &a[3]);
    CHECK(vector_pop_back(v) == &a[2]);
    CHECK(vector_is_empty(v));
    CHECK(vector_pop_back(v) == NULL);
    CHECK(vector_pop_front(v) == NULL);

    vector_destroy(&v, NULL);
}

static void test_remove_at(void) {
    int a[] = {10, 20, 30, 40, 50};
    vector_t *v = vector_create();
    CHECK(v != NULL);
    CHECK(append_all(v, a, 5));

    CHECK(vector_remove_at(v, 2) == &a[2]); /* middle: [10 20 40 50] */
    CHECK(vector_get_len(v) == 4);
    CHECK(vector_at(v, 2) == &a[3]);

    CHECK(vector_remove_at(v, 0) == &a[0]); /* first:  [20 40 50]    */
    CHECK(vector_head(v) == &a[1]);

    CHECK(vector_remove_at(v, 2) == &a[4]); /* last:   [20 40]       */
    CHECK(vector_tail(v) == &a[3]);
    CHECK(vector_get_len(v) == 2);

    /* out of range: nothing changes */
    CHECK(vector_remove_at(v, 2) == NULL);
    CHECK(vector_remove_at(v, 100) == NULL);
    CHECK(vector_remove_at(v, (size_t)-1) == NULL);
    CHECK(vector_get_len(v) == 2);

    vector_destroy(&v, NULL);
}

static void test_find(void) {
    int a[] = {5, 3, 8, 3, 9};
    int target = 3, missing = 77, first = 5, last = 9;
    vector_t *v = vector_create();
    CHECK(v != NULL);
    CHECK(append_all(v, a, 5));

    CHECK(vector_find(v, &target, cmp_int_ptr) == 1); /* first match wins */
    CHECK(vector_find(v, &first, cmp_int_ptr) == 0);
    CHECK(vector_find(v, &last, cmp_int_ptr) == 4);
    CHECK(vector_find(v, &missing, cmp_int_ptr) == -1);

    /* find matches by comparator, not by pointer identity */
    int copy = 8;
    CHECK(vector_find(v, &copy, cmp_int_ptr) == 2);

    vector_destroy(&v, NULL);
}

static void test_find_strings(void) {
    char *words[] = {"pear", "apple", "zebra", "mango"};
    vector_t *v = vector_create();
    CHECK(v != NULL);

    for (size_t i = 0; i < 4; i++)
        CHECK(vector_append(v, words[i]) == VECT_SUCCESS);

    CHECK(vector_find(v, "zebra", cmp_str_ptr) == 2);
    CHECK(vector_find(v, "pear", cmp_str_ptr) == 0);
    CHECK(vector_find(v, "kiwi", cmp_str_ptr) == -1);

    vector_destroy(&v, NULL);
}

static void test_growth_append(void) {
    enum { N = 5000 };
    int *v_data = malloc(N * sizeof *v_data);
    CHECK(v_data != NULL);
    if (!v_data) return;
    for (int i = 0; i < N; i++) v_data[i] = i;

    vector_t *v = vector_create();
    CHECK(v != NULL);
    size_t initial_cap = vector_get_cap(v);

    for (int i = 0; i < N; i++) {
        CHECK(vector_append(v, &v_data[i]) == VECT_SUCCESS);
        CHECK(vector_get_cap(v) >= vector_get_len(v));
    }
    CHECK(vector_get_len(v) == N);
    CHECK(vector_get_cap(v) > initial_cap);
    check_content(v, v_data, N);

    vector_destroy(&v, NULL);
    free(v_data);
}

static void test_growth_exact_boundary(void) {
    /* Fill to exactly the capacity, then one more: must trigger growth. */
    int pool[64];
    for (int i = 0; i < 64; i++) pool[i] = i;

    vector_t *v = vector_create();
    CHECK(v != NULL);
    size_t cap = vector_get_cap(v);
    CHECK(cap < 64);

    for (size_t i = 0; i < cap; i++)
        CHECK(vector_append(v, &pool[i]) == VECT_SUCCESS);
    CHECK(vector_get_len(v) == cap);
    CHECK(vector_get_cap(v) == cap); /* full, not yet grown */

    CHECK(vector_append(v, &pool[cap]) == VECT_SUCCESS);
    CHECK(vector_get_cap(v) > cap);
    CHECK(vector_get_len(v) == cap + 1);
    check_content(v, pool, cap + 1);

    vector_destroy(&v, NULL);
}

static void test_growth_prepend_and_insert(void) {
    enum { N = 1000 };
    int *pool = malloc(N * sizeof *pool);
    CHECK(pool != NULL);
    if (!pool) return;
    for (int i = 0; i < N; i++) pool[i] = i;

    /* prepend: final order is N-1 ... 0 */
    vector_t *v = vector_create();
    CHECK(v != NULL);
    for (int i = 0; i < N; i++)
        CHECK(vector_prepend(v, &pool[i]) == VECT_SUCCESS);
    CHECK(vector_get_len(v) == N);
    for (int i = 0; i < N; i++)
        CHECK(vector_at(v, (size_t)i) == &pool[N - 1 - i]);
    vector_destroy(&v, NULL);

    /* insert always in the middle across several growths */
    v = vector_create();
    CHECK(v != NULL);
    for (int i = 0; i < N; i++)
        CHECK(vector_insert_at(v, vector_get_len(v) / 2, &pool[i]) == VECT_SUCCESS);
    CHECK(vector_get_len(v) == N);
    CHECK(vector_get_cap(v) >= N);
    /* every item must be present exactly once */
    for (int i = 0; i < N; i++)
        CHECK(vector_find(v, &pool[i], cmp_int_ptr) >= 0);
    vector_destroy(&v, NULL);

    free(pool);
}

static void test_capacity_does_not_shrink(void) {
    enum { N = 100 };
    int pool[N];
    for (int i = 0; i < N; i++) pool[i] = i;

    vector_t *v = vector_create();
    CHECK(v != NULL);
    CHECK(append_all(v, pool, N));
    size_t cap = vector_get_cap(v);

    for (int i = 0; i < N; i++)
        CHECK(vector_pop_back(v) != NULL);
    CHECK(vector_is_empty(v));
    CHECK(vector_get_cap(v) == cap);

    /* and it is fully reusable afterwards */
    CHECK(append_all(v, pool, N));
    check_content(v, pool, N);
    CHECK(vector_get_cap(v) == cap);

    vector_destroy(&v, NULL);
}

static void test_reuse_after_drain(void) {
    int a[] = {3, 1, 2};
    int b[] = {30, 10, 20};
    vector_t *v = vector_create();
    CHECK(v != NULL);

    CHECK(append_all(v, a, 3));
    check_content(v, a, 3);
    while (!vector_is_empty(v)) vector_pop_front(v);

    CHECK(append_all(v, b, 3));
    check_content(v, b, 3);

    vector_destroy(&v, NULL);
}

static void test_independent_vectors(void) {
    int a[] = {5, 1, 3};
    int b[] = {50, 10, 30};
    vector_t *v1 = vector_create();
    vector_t *v2 = vector_create();
    CHECK(v1 != NULL && v2 != NULL);

    CHECK(append_all(v1, a, 3));
    CHECK(append_all(v2, b, 3));

    CHECK(vector_pop_back(v1) == &a[2]);
    CHECK(vector_get_len(v1) == 2);
    CHECK(vector_get_len(v2) == 3); /* v2 untouched */
    check_content(v2, b, 3);

    vector_destroy(&v1, NULL);
    vector_destroy(&v2, NULL);
}

/* Random mix of every operation, checked against a plain-array model. */
static void test_random_vs_reference(void) {
    enum { OPS = 4000, POOL = OPS + 1 };
    int *pool = malloc(POOL * sizeof *pool);
    void **ref = malloc(POOL * sizeof *ref);
    CHECK(pool != NULL && ref != NULL);
    if (!pool || !ref) {
        free(pool);
        free(ref);
        return;
    }
    for (int i = 0; i < POOL; i++) pool[i] = i;

    vector_t *v = vector_create();
    CHECK(v != NULL);

    srand(2026);
    size_t pool_n = 0, ref_n = 0;

    for (int op = 0; op < OPS; op++) {
        int kind = rand() % 9;
        void *entry = &pool[pool_n];
        int used = 0;

        switch (kind) {
        case 0: /* append */
        case 1:
            CHECK(vector_append(v, entry) == VECT_SUCCESS);
            ref[ref_n++] = entry;
            used = 1;
            break;
        case 2: /* prepend */
            CHECK(vector_prepend(v, entry) == VECT_SUCCESS);
            memmove(ref + 1, ref, ref_n * sizeof *ref);
            ref[0] = entry;
            ref_n++;
            used = 1;
            break;
        case 3: { /* insert_at in [0, len] */
            size_t i = (size_t)rand() % (ref_n + 1);
            CHECK(vector_insert_at(v, i, entry) == VECT_SUCCESS);
            memmove(ref + i + 1, ref + i, (ref_n - i) * sizeof *ref);
            ref[i] = entry;
            ref_n++;
            used = 1;
            break;
        }
        case 4: /* replace_at */
            if (ref_n == 0) {
                CHECK(vector_replace_at(v, 0, entry) == VECT_ERR_OUT_OF_RANGE);
            } else {
                size_t i = (size_t)rand() % ref_n;
                CHECK(vector_replace_at(v, i, entry) == VECT_SUCCESS);
                ref[i] = entry;
                used = 1;
            }
            break;
        case 5: { /* pop_back */
            void *got = vector_pop_back(v);
            if (ref_n == 0) {
                CHECK(got == NULL);
            } else {
                CHECK(got == ref[ref_n - 1]);
                ref_n--;
            }
            break;
        }
        case 6: { /* pop_front */
            void *got = vector_pop_front(v);
            if (ref_n == 0) {
                CHECK(got == NULL);
            } else {
                CHECK(got == ref[0]);
                memmove(ref, ref + 1, (ref_n - 1) * sizeof *ref);
                ref_n--;
            }
            break;
        }
        case 7: { /* remove_at, sometimes out of range */
            size_t i = (size_t)rand() % (ref_n + 2);
            void *got = vector_remove_at(v, i);
            if (i >= ref_n) {
                CHECK(got == NULL);
            } else {
                CHECK(got == ref[i]);
                memmove(ref + i, ref + i + 1, (ref_n - i - 1) * sizeof *ref);
                ref_n--;
            }
            break;
        }
        case 8: /* find a value known to be present */
            if (ref_n > 0) {
                size_t i = (size_t)rand() % ref_n;
                int idx = vector_find(v, ref[i], cmp_int_ptr);
                /* pool values are unique, so the match is exact */
                CHECK(idx == (int)i);
            }
            break;
        }
        if (used) pool_n++;

        CHECK(vector_get_len(v) == ref_n);
        CHECK(vector_get_cap(v) >= ref_n);
        CHECK(vector_is_empty(v) == (ref_n == 0));
        for (size_t i = 0; i < ref_n; i++) {
            if (vector_at(v, i) != ref[i]) {
                CHECK(vector_at(v, i) == ref[i]);
                break;
            }
        }
        if (checks_failed > 20) break; /* avoid flooding on a broken vector */
    }

    vector_destroy(&v, NULL);
    free(pool);
    free(ref);
}

static int free_calls = 0;
static int free_order_ok = 1;
static int free_expected_next = 0;

static void counting_free(void *p) {
    free_calls++;
    free(p);
}

static void ordered_free(void *p) {
    if (*(int *)p != free_expected_next) free_order_ok = 0;
    free_expected_next++;
    free_calls++;
    free(p);
}

static void test_destroy_with_free_fn(void) {
    enum { N = 100, REMOVED = 30 };
    vector_t *v = vector_create();
    CHECK(v != NULL);

    for (int i = 0; i < N; i++) {
        int *p = malloc(sizeof *p);
        CHECK(p != NULL);
        if (!p) break;
        *p = i;
        CHECK(vector_append(v, p) == VECT_SUCCESS);
    }

    /* removed items belong to the caller: free them ourselves */
    for (int i = 0; i < REMOVED; i++)
        free(vector_pop_back(v));

    free_calls = 0;
    size_t left = vector_get_len(v);
    CHECK(left == N - REMOVED);
    CHECK(vector_destroy(&v, counting_free) == VECT_SUCCESS);
    CHECK(v == NULL);
    CHECK(free_calls == (int)left);

    /* empty vector: free_fn must not be called */
    v = vector_create();
    CHECK(v != NULL);
    free_calls = 0;
    CHECK(vector_destroy(&v, counting_free) == VECT_SUCCESS);
    CHECK(free_calls == 0);

    /* NULL vector with a free_fn: no crash, no calls */
    CHECK(vector_destroy(&v, counting_free) == VECT_ERR_NULL_PTR);
    CHECK(vector_destroy(NULL, counting_free) == VECT_ERR_NULL_PTR);
    CHECK(free_calls == 0);
}

static void test_destroy_free_order(void) {
    enum { N = 50 };
    vector_t *v = vector_create();
    CHECK(v != NULL);

    for (int i = 0; i < N; i++) {
        int *p = malloc(sizeof *p);
        CHECK(p != NULL);
        if (!p) break;
        *p = i;
        CHECK(vector_append(v, p) == VECT_SUCCESS);
    }

    /* documented: free_fn is called from the first item to the last */
    free_calls = 0;
    free_order_ok = 1;
    free_expected_next = 0;
    CHECK(vector_destroy(&v, ordered_free) == VECT_SUCCESS);
    CHECK(free_calls == N);
    CHECK(free_order_ok);
}

static void test_destroy_without_free_fn_keeps_items(void) {
    int a = 1, b = 2;
    vector_t *v = vector_create();
    CHECK(v != NULL);
    CHECK(vector_append(v, &a) == VECT_SUCCESS);
    CHECK(vector_append(v, &b) == VECT_SUCCESS);

    /* stack items: with NULL free_fn nothing may be freed (ASan would abort) */
    CHECK(vector_destroy(&v, NULL) == VECT_SUCCESS);
    CHECK(v == NULL);
    CHECK(a == 1 && b == 2);
}

int main(void) {
    RUN(test_create_destroy);
    RUN(test_destroy_null_safe);
    RUN(test_null_arguments);
    RUN(test_empty_vector);
    RUN(test_single_element);
    RUN(test_append_order);
    RUN(test_prepend_order);
    RUN(test_insert_at);
    RUN(test_insert_at_out_of_range);
    RUN(test_replace_at);
    RUN(test_getters_out_of_range);
    RUN(test_pop_back_front);
    RUN(test_remove_at);
    RUN(test_find);
    RUN(test_find_strings);
    RUN(test_growth_append);
    RUN(test_growth_exact_boundary);
    RUN(test_growth_prepend_and_insert);
    RUN(test_capacity_does_not_shrink);
    RUN(test_reuse_after_drain);
    RUN(test_independent_vectors);
    RUN(test_random_vs_reference);
    RUN(test_destroy_with_free_fn);
    RUN(test_destroy_free_order);
    RUN(test_destroy_without_free_fn_keeps_items);

    printf("\n%d tests, %d failed checks\n", tests_run, checks_failed);
    return checks_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}