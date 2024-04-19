#include "log.h"
#include "util.h"
#include <fcntl.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct Window {
    int start_line;
    int end_line;
};

char **file_vec = NULL;
char *dirname = NULL;
int path_changed = 0;
int y = 0;

struct Window file_window;

void change_dir(char *newdir)
{
    if (!(newdir && strcmp(dirname, newdir) != 0 && is_valid_dir(newdir))) {
        log_error("cd failed: %s", newdir);
        free(newdir);
        return;
    }

    free(dirname);
    dirname = newdir;
    log_debug("cd: %s", dirname);
    path_changed = 1;
    y = file_window.start_line;
}

void display_files()
{
    for (int i = file_window.start_line; i < file_window.end_line && file_vec[i] != NULL; i++) {
        move(i, 0);
        clrtoeol();
        printw("%s\n", file_vec[i]);
    }

    move(y, file_window.start_line);
}

int main(int argc, const char *argv[])
{
    int logfile = open("stderr.log", O_CREAT | O_APPEND | O_WRONLY, 0640);
    dup2(logfile, 2);
    close(logfile);

    if (argc < 2) {
        const char *home_path = getenv("HOME");
        if (!home_path) {
            log_error("getenv");
            return 1;
        }

        dirname = strdup(home_path);

    } else {
        const char *start_path = argv[1];
        if (!(dirname = realpath(start_path, NULL))) {
            log_error("realpath");
            return 1;
        }
    }

    log_debug("current directory: %s", dirname);

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    file_window.start_line = 0;
    file_window.end_line = LINES - 1;

    // main loop
    while (1) {
        erase();

        if (!file_vec || path_changed) {
            char **new_file_vec = list_files(dirname);
            if (new_file_vec == NULL) {
                break;
            }

            for (char **itr = file_vec; itr != NULL && *itr != NULL; itr++) {
                free(*itr);
            }

            if (file_vec) {
                free(file_vec);
            }

            file_vec = new_file_vec;
        }

        display_files();
        refresh();

        int ch = getch();
        if (ch == 'q') {
            break;
        } else if (ch == KEY_DOWN || ch == 'j') {
            if (y < file_window.end_line && y < num_files(file_vec) - 1) {
                y++;
            }
        } else if (ch == KEY_UP || ch == 'k') {
            if (y > file_window.start_line) {
                y--;
            }
        } else if (ch == '\n' || ch == 'l') {
            change_dir(path_join(dirname, file_vec[y]));
        } else if (ch == 'h' || ch == '-') {
            change_dir(get_parent(dirname));
        } else if (ch == 'g') {
            y = file_window.start_line;
        } else if (ch == 'G') {
            y = MIN(file_window.end_line, num_files(file_vec) - 1);
        }
    }

    endwin();
    return 0;
}
