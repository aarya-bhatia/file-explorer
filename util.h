#pragma once

#include <stdbool.h>
#include <sys/types.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

size_t vec_size(char **vec);
char **vec_copy(char **dst, char **src);
void vec_free(char **vec);

size_t num_files(char **vector);
char **list_files(const char *dirname);
char *path_join(const char *current, const char *filename);
const char *rstrstr(const char *str, const char *substr);
char *get_parent(const char *dirname);
bool is_valid_dir(const char *dirname);
