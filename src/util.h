#pragma once

#include <stdbool.h>
#include <sys/types.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

size_t vec_size(char **vec);
char **vec_copy(char **dst, char **src);
void vec_free(char **vec);

char **list_files(const char *dirname);
char *path_join(const char *current, const char *filename);
const char *rstrstr(const char *str, const char *substr);
char *get_parent(const char *dirname);
bool is_valid_dir(const char *dirname);

void get_human_size(size_t value, char *buffer, size_t n);
void get_human_time(struct timespec t, char *buffer, size_t n);
void get_perm_string(int perms, char *buffer, size_t n);
