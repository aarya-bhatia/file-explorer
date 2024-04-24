#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

size_t num_files(char **vector)
{
    if (!vector) {
        return 0;
    }

    size_t i;
    for (i = 0; vector[i] != NULL; i++) {
    }

    return i;
}

const char *rstrstr(const char *str, const char *substr)
{
    char *s = strstr(str, substr);
    if (!s) {
        return NULL;
    }

    char *t = NULL;
    while ((t = strstr(s + 1, substr))) {
        s = t;
    }

    return s;
}

size_t vec_size(char **vec)
{
    if (!vec) {
        return 0;
    }

    size_t i;
    for (i = 0; vec[i] != NULL; i++) {
    }

    return i;
}

char **vec_copy(char **dst, char **src)
{
    assert(src);
    assert(dst);

    int n_dst = vec_size(dst);
    for (int i = 0; i < n_dst; i++) {
        free(dst[i]);
    }

    int n_src = vec_size(src);
    dst = realloc(dst, (n_src + 1) * sizeof(*dst));
    for (int i = 0; i < n_src; i++) {
        dst[i] = strdup(src[i]);
    }
    dst[n_src] = 0;
    return dst;
}

void vec_free(char **vec)
{
    if (!vec) {
        return;
    }

    for (int i = 0; i < vec_size(vec); i++) {
        free(vec[i]);
    }

    free(vec);
}
