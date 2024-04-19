#include "log.h"
#include "util.h"
#include <fcntl.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char **file_vec = NULL;
char *dirname = NULL;
int path_changed = 0;
int y = 0;

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
    y = 0;
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

        for (int i = 0; i < LINES && file_vec[i] != NULL; i++) {
            printw("%s\n", file_vec[i]);
        }

        move(y, 0);
        refresh();

        int ch = getch();
        if (ch == 'q') {
            break;
        } else if (ch == KEY_DOWN || ch == 'j') {
            if (y < LINES - 1 && y < num_files(file_vec) - 1) {
                y++;
            }
        } else if (ch == KEY_UP || ch == 'k') {
            if (y > 0) {
                y--;
            }
        } else if (ch == '\n' || ch == 'l') {
            change_dir(path_join(dirname, file_vec[y]));
        } else if (ch == 'h' || ch == '-') {
            change_dir(get_parent(dirname));
        } else if (ch == 'g') {
            y = 0;
        } else if (ch == 'G') {
            y = MAX(0, num_files(file_vec)-1);
        }
    }

    endwin();
    return 0;
}
