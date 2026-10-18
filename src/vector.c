#include <ds_lib/vector.h>

#define VECTOR_INIT_CAP 10

struct vector_t {
    void **vector;
    size_t len;
    size_t dim;
};


