#include "log.h"
#include "util.h"
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char **list_files(const char *dirname)
{
    char **vector = calloc(1, sizeof *vector);
    size_t capacity = 1;
    size_t size = 0;

    DIR *dirp = opendir(dirname);
    if (!dirp) {
        log_error("opendir");
        free(vector);
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dirp)) != NULL) {
        if (size + 1 >= capacity) {
            capacity *= 2;
            vector = realloc(vector, capacity * sizeof *vector);
        }

        if ((entry->d_type == DT_REG) || (entry->d_type == DT_DIR)) {
            if (entry->d_name[0] != '.') {
                vector[size++] = strdup(entry->d_name);
            }
        }
    }

    vector = realloc(vector, (size + 1) * sizeof *vector);
    vector[size] = NULL;

    closedir(dirp);

    return vector;
}

char *path_join(const char *current, const char *filename)
{
    char *newdir = calloc(1, strlen(current) + strlen(filename) + 2);
    sprintf(newdir, "%s/%s", current, filename);

    char *resolved = realpath(newdir, NULL);
    free(newdir);

    return resolved;
}

char *get_parent(const char *dirname)
{
    if (strcmp(dirname, "/") == 0 || !strstr(dirname + 1, "/")) {
        return strdup("/");
    }

    const char *ptr = rstrstr(dirname, "/");
    return strndup(dirname, ptr - dirname);
}

bool is_valid_dir(const char *dirname)
{
    struct stat s;
    return stat(dirname, &s) == 0 && S_ISDIR(s.st_mode) && access(dirname, R_OK | X_OK) == 0;
}
