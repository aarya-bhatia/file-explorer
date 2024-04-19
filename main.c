#include "log.h"
#include "util.h"
#include <assert.h>
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
struct Window status_window;

const char *username;
char hostname[100];

int get_last_line()
{
    return MIN(file_window.end_line, file_window.start_line + num_files(file_vec) - 1);
}

int count_window_lines(struct Window w)
{
    return w.end_line - w.start_line + 1;
}

const char *get_selected_file()
{
    return file_vec[y - file_window.start_line];
}

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

void display_status()
{
    move(0, 0);
    printw("%s@%s:%s/%s\n\n", username, hostname, dirname, get_selected_file());
    /* printw("%s\n\n", dirname); */
}

void display_files()
{
    for (int i = 0; i < count_window_lines(file_window) && file_vec[i] != NULL; i++) {
        move(i + file_window.start_line, 0);
        /* clrtoeol(); */
        printw("%s\n", file_vec[i]);
    }

    move(y, 0);
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

    gethostname(hostname, sizeof hostname - 1);

    username = getenv("USER");
    assert(username);

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    status_window.start_line = 0;
    status_window.end_line = 1;

    file_window.start_line = 2;
    file_window.end_line = LINES - 1;

    y = file_window.start_line;

    // main loop
    while (1) {
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

        erase();
        display_status();
        display_files();
        refresh();

        int ch = getch();
        if (ch == 'q') {
            break;
        } else if (ch == KEY_DOWN || ch == 'j') {
            if (y < get_last_line()) {
                y++;
            }
        } else if (ch == KEY_UP || ch == 'k') {
            if (y > file_window.start_line) {
                y--;
            }
        } else if (ch == '\n' || ch == 'l') {
            change_dir(path_join(dirname, get_selected_file()));
        } else if (ch == 'h' || ch == '-') {
            change_dir(get_parent(dirname));
        } else if (ch == 'g') {
            y = file_window.start_line;
        } else if (ch == 'G') {
            y = get_last_line();
        }
    }

    endwin();
    return 0;
}
