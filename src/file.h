#pragma once

#include "log.h"
#include "util.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct File {
    char *name;
    struct stat stat;
    struct File *next;
    struct File *prev;
} File;

typedef struct UI {
    enum { NORMAL_MODE, FIND_MODE } mode;
    File *selected;
    File *files;
    int scroll;
    char *dirname;
    int max_files;
} UI;

bool is_displayed(UI *ui, File *f);
File *get_bottom_file(UI *ui);
File *get_file_at_index(UI *ui, int index);
File *get_top_file(UI *ui);
int distance(File *A, File *B);
int get_file_index(UI *ui, File *f);
int get_num_files(UI *ui);
int num_files_displayed(UI *ui);
void add_file(UI *ui, File *file);
void free_file_list(UI *ui);
void refresh_file_list(UI *ui);
void set_directory(UI *ui, char *dirname);
