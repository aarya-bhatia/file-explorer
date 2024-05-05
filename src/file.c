#include "file.h"
#include <dirent.h>
#include <sys/stat.h>

File *get_file_at_index(UI *ui, int index)
{
    File *f = ui->files;
    for (int i = 0; i < index && f != NULL; i++) {
        f = f->next;
    }
    return f;
}

File *get_bottom_file(UI *ui)
{
    File *f = get_file_at_index(ui, ui->scroll);
    if (!f) {
        return NULL;
    }

    for (int i = 0; i < ui->max_files - 1 && f->next; i++) {
        f = f->next;
    }

    return f;
}

File *get_top_file(UI *ui)
{
    return get_file_at_index(ui, ui->scroll);
}

int distance(File *A, File *B)
{
    int i = 0;
    while (A && B && A != B->next) {
        i++;
        A = A->next;
    }

    return i;
}

bool is_displayed(UI *ui, File *f)
{
    if (get_num_files(ui) == 0) {
        return false;
    }

    File *A = get_top_file(ui);
    File *B = get_bottom_file(ui);

    while (A && B && A != B->next) {
        if (A == f) {
            return true;
        }
        A = A->next;
    }

    return false;
}

int num_files_displayed(UI *ui)
{
    if (!ui->files) {
        return 0;
    }

    return distance(get_top_file(ui), get_bottom_file(ui));
}

void free_file_list(UI *ui)
{
    while (ui->files) {
        File *tmp = ui->files->next;
        free(ui->files);
        ui->files = tmp;
    }

    ui->files = NULL;
    ui->selected = NULL;
}

void set_directory(UI *ui, char *dirname)
{
    free(ui->dirname);
    ui->dirname = dirname;
    ui->scroll = 0;
    free_file_list(ui);
    refresh_file_list(ui);
    ui->selected = ui->files;
}

void refresh_file_list(UI *ui)
{
    char **file_vec = list_files(ui->dirname);
    assert(file_vec);

    int ndir = 0;
    int nlnk = 0;

    for (int i = 0; file_vec[i]; i++) {
        File *node = calloc(1, sizeof *node);
        node->name = file_vec[i];
        char filename[256];
        snprintf(filename, sizeof(filename) - 1, "%s/%s", ui->dirname, file_vec[i]);

        if (lstat(filename, &node->stat) < 0) {
            log_error("stat failed: %s", file_vec[i]);
            free(file_vec[i]);
            free(node);
            continue;
        }

        if (S_ISDIR(node->stat.st_mode)) {
            ndir++;
        } else if (S_ISLNK(node->stat.st_mode)) {
            nlnk++;
        }

        add_file(ui, node);
    }

    free(file_vec);

    log_debug("total dirs: %d, links: %d", ndir, nlnk);
}

void add_file(UI *ui, File *file)
{
    file->prev = file->next = NULL;

    if (!ui->files) {
        ui->files = file;
        return;
    }

    File *f = ui->files;
    for (; f->next; f = f->next) {
    }

    f->next = file;
    file->prev = f;
}

void sort_file_list(UI *ui)
{
}

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

        if (entry->d_name[0] != '.') {
            vector[size++] = strdup(entry->d_name);
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

int get_file_index(UI *ui, File *f)
{
    File *itr = ui->files;
    for (int i = 0; itr; i++, itr = itr->next) {
        if (itr == f) {
            return i;
        }
    }

    assert(0);
}

int get_num_files(UI *ui)
{
    File *itr = ui->files;
    int i = 0;
    for (; itr; i++, itr = itr->next) {
    }

    return i;
}
