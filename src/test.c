#include "file.h"
#include "util.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void test_files()
{
    UI *ui = calloc(1, sizeof *ui);
    set_directory(ui, strdup("/home/aarya"));
    assert(ui->files);
    assert(ui->selected);
    assert(!strcmp(ui->dirname, "/home/aarya"));
    assert(ui->scroll == 0);
    assert(ui->selected == ui->files);

    ui->scroll++;

    set_directory(ui, strdup("/home/aarya/scripts"));

    int n = get_num_files(ui);
    ui->max_files = n / 2;

    File *f1 = get_top_file(ui);
    File *f2 = get_bottom_file(ui);
    assert(distance(f1, f2) == ui->max_files);
    assert(num_files_displayed(ui) == ui->max_files);
    assert(get_file_index(ui, f1) == ui->scroll);
    assert(get_file_index(ui, f2) == ui->scroll + ui->max_files - 1);

    free_file_list(ui);
    free(ui);
}

void test_util()
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

    char result[16];
    get_human_size((size_t)1025, result, sizeof result - 1);
    assert(!strcmp(result, "1K"));

    get_human_size((size_t)1024 * 1024 * 4, result, sizeof result - 1);
    assert(!strcmp(result, "4M"));

    get_human_size((size_t)1024 * 1024 * 1024 * 500, result, sizeof result - 1);
    assert(!strcmp(result, "500G"));

    time_t t = time(NULL);
    struct timespec ts;
    ts.tv_sec = t;
    ts.tv_nsec = 0;
    char buffer[256];
    get_human_time(ts, buffer, sizeof buffer - 1);
    puts(buffer);

    get_perm_string(0644, buffer, sizeof buffer - 1);
    puts(buffer);

    get_perm_string(0755, buffer, sizeof buffer - 1);
    puts(buffer);
}

int main()
{
    test_files();
}
