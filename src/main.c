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

void change_dir(char *);
void display_top();
void display_files();
void display_bottom();

char **file_vec = NULL;
struct stat *file_stats = NULL;
char *dirname = NULL;
int num_files = 0;
int selected = 0;
int scroll_offset = 0;
int path_changed = 0;

enum { NORMAL_MODE, FIND_MODE } ui_mode = 0;

char status_message[512];

WINDOW *file_window = NULL;
WINDOW *top_window = NULL;
WINDOW *bottom_window = NULL;

const char *username;
char hostname[100];

int max_files;

/* Select next file in view */
void handle_down_key()
{
    // check if bottom-most file is in view
    if (selected - scroll_offset + 1 < MIN(max_files, num_files)) {
        selected++;
    }
}

/* Select prev file in view */
void handle_up_key()
{
    if (selected > 0) {
        selected--;
    }
}

/* If directory is selected, move into directory
 * TODO: If file is selected, perform file preview action */
void handle_right_key()
{
    change_dir(path_join(dirname, file_vec[selected]));
}

/* Move to parent directory */
void handle_left_key()
{
    change_dir(get_parent(dirname));
}

/* Scroll file view down */
void handle_scroll_down()
{
    if (num_files - scroll_offset > max_files) {
        scroll_offset++;
        selected = MAX(selected, scroll_offset);
    }
}

/* Scroll file view up */
void handle_scroll_up()
{
    if (scroll_offset > 0) {
        scroll_offset--;
        selected = MIN(selected, scroll_offset);
    }
}

/* select top most file in the view */
void handle_goto_top()
{
    selected = scroll_offset;
}

/* select bottom most file in the view */
void handle_goto_bottom()
{
    if (num_files > 0) {
        selected = MIN(scroll_offset + max_files, num_files) - 1;
    }
}

void handle_start_find()
{
    sprintf(status_message, "f");
    ui_mode = FIND_MODE;
}

void handle_find_key(char ch)
{
    ui_mode = NORMAL_MODE;
    status_message[0] = 0;

    for (int i = selected + 1; i < MIN(max_files, num_files); i++) {
        if (*file_vec[i] == ch) {
            selected = i;
            break;
        }
    }
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
    selected = 0;
    scroll_offset = 0;
}

void display_top()
{
    werase(top_window);
    wmove(top_window, 0, 0);

    if (strcmp(dirname, "/") == 0) {
        wprintw(top_window, "%s@%s:/%s\n\n", username, hostname, file_vec[selected]);
    } else {
        wprintw(top_window, "%s@%s:%s/%s\n\n", username, hostname, dirname, file_vec[selected]);
    }

    wrefresh(top_window);
}

void display_files()
{
    assert(file_vec);
    assert(file_stats);

    werase(file_window);
    wmove(file_window, 0, 0);

    int longest_name = 0;
    for (int i = scroll_offset; i < scroll_offset + max_files && file_vec[i] != NULL; i++) {
        longest_name = MAX(longest_name, strlen(file_vec[i]));
    }

    char buffer[256];

    for (int i = scroll_offset; i < scroll_offset + max_files && file_vec[i] != NULL; i++) {
        if (selected == i) {
            wprintw(file_window, "> %s", file_vec[i]);
        } else {
            wprintw(file_window, "  %s", file_vec[i]);
        }

        // padding
        for (int j = 0; j < longest_name - strlen(file_vec[i]) + 1; j++) {
            wprintw(file_window, " ");
        }

        get_human_size(file_stats[i].st_size, buffer, sizeof buffer - 1);
        wprintw(file_window, "%-6s", buffer);

        get_human_time(file_stats[i].st_mtim, buffer, sizeof buffer - 1);
        wprintw(file_window, "%-20s", buffer);

        get_perm_string(file_stats[i].st_mode, buffer, sizeof buffer - 1);
        wprintw(file_window, "%-16s", buffer);

        wprintw(file_window, "\n");
    }

    wrefresh(file_window);
}

void display_bottom()
{
    werase(bottom_window);
    wmove(bottom_window, 0, 0);
    wprintw(bottom_window, "\n[%d/%d]", selected, num_files);
    if (strlen(status_message) > 0) {
        wprintw(bottom_window, "\t\t%s", status_message);
    }
    wprintw(bottom_window, "\n");
    wrefresh(bottom_window);
}

void quit(int exit_code)
{
    delwin(top_window);
    delwin(file_window);
    delwin(bottom_window);
    endwin();
    exit(exit_code);
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
    curs_set(0);

    max_files = MIN(LINES / 2, 20);
    top_window = newwin(2, COLS, 0, 0);
    file_window = newwin(max_files, COLS, 2, 0);
    bottom_window = newwin(2, COLS, 2 + max_files, 0);

    log_debug("max files: %d", max_files);

    refresh();

    // main loop
    while (1) {
        if (!file_vec || path_changed) {
            char **new_file_vec = list_files(dirname);
            if (new_file_vec == NULL) {
                break;
            }

            vec_free(file_vec);
            file_vec = new_file_vec;

            num_files = vec_size(file_vec);

            free(file_stats);
            file_stats = calloc(num_files, sizeof *file_stats);
            char filename[256];
            for (int i = 0; i < num_files; i++) {
                snprintf(filename, sizeof(filename) - 1, "%s/%s", dirname, file_vec[i]);
                if (stat(filename, &file_stats[i]) < 0) {
                    log_error("stat failed: %s", file_vec[i]);
                }
            }
        }

        display_top();
        display_bottom();
        display_files();

        int ch = getch();

        switch (ui_mode) {
            case NORMAL_MODE:

                switch (ch) {
                    case 'q':
                        quit(0);
                        break;

                    case 'j':
                    case KEY_DOWN:
                        handle_down_key();
                        break;

                    case 'k':
                    case KEY_UP:
                        handle_up_key();
                        break;

                    case '\n':
                    case 'l':
                        handle_right_key();
                        break;

                    case 'h':
                    case '-':
                        handle_left_key();
                        break;

                    case 'g':
                        handle_goto_top();
                        break;

                    case 'G':
                        handle_goto_bottom();
                        break;

                    case 'f':
                        handle_start_find();
                        break;

                    case 'J':
                        handle_scroll_down();
                        break;

                    case 'K':
                        handle_scroll_up();
                        break;
                }

                break;
            case FIND_MODE:
                handle_find_key(ch);
                break;
        }
    }

    quit(0);
}
