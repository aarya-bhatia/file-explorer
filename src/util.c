#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

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

void get_human_size(size_t value, char *buffer, size_t n)
{
    static char *units[] = {"B", "K", "M", "G", "T"};
    int unit_index = 0;
    while (value > 1024 && unit_index + 1 < sizeof units / sizeof units[0]) {
        value = value / 1024;
        unit_index++;
    }

    if (value > 1024) {
        snprintf(buffer, n, "inf");
        return;
    }

    snprintf(buffer, n, "%zu%s", value, units[unit_index]);
}

void get_human_time(struct timespec t, char *buffer, size_t n)
{
    struct tm tm;
    time_t time = t.tv_sec;
    gmtime_r(&time, &tm);
    strftime(buffer, n, "%Y-%m-%d %H:%M:%S", &tm);
}

void get_perm_string(int perms, char *buffer, size_t n)
{
    if (n < 9) {
        buffer[0] = 0;
        return;
    }

    int i;
    char perms_str[] = "rwxrwxrwx";

    for (i = 0; i < 9; i++) {
        buffer[i] = (perms & (1 << (8 - i))) ? perms_str[i] : '-';
    }

    buffer[i] = 0;
}
