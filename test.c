#include "util.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char **vector = list_files("/");
    for (char **itr = vector; *itr != NULL; itr++) {
        puts(*itr);
        free(*itr);
    }

    free(vector);

    char *path = path_join("/", "home");
    assert(!strcmp(path, "/home"));

    assert(is_valid_dir(path));
    assert(is_valid_dir("/"));
    assert(!is_valid_dir("/root"));
    assert(is_valid_dir("/home/aarya/repos"));

    free(path);

    path = get_parent("/home/aarya");
    assert(!strcmp(path, "/home"));
    free(path);

    path = get_parent("/home");
    assert(!strcmp(path, "/"));
    free(path);

    path = get_parent("/");
    assert(!strcmp(path, "/"));
    free(path);

    return 0;
}
