#pragma once

#include "file.h"

typedef int (*file_compare)(File *f1, File *f2);

File *merge_sort(File *head, file_compare cmp);
File *merge_sorted(File *a, File *b, file_compare cmp);
void split_list(File *source, File **front_ptr, File **back_ptr);
