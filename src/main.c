#include "file.h"
#include <ncurses.h>
#include <stdio.h>
#include <sys/stat.h>

#define UI_MAX_LINES 50
#define UI_MAX_PATH_LENGTH 30

enum {
    COLOR_PAIR_NORMAL = 1,
    COLOR_PAIR_DIRECTORY,
    COLOR_PAIR_SYMLINK,

    COLOR_PAIR_BLUE,
    COLOR_PAIR_GREEN,
    COLOR_PAIR_RED,
    COLOR_PAIR_WHITE,
    COLOR_PAIR_YELLOW,
};

void quit(int);
void change_dir(char *);
void display_title_window();
void display_file_window();
void display_index_window();
void display_help_window();
void display_status_window();
void handle_up_key();
void handle_down_key();
void handle_left_key();
void handle_right_key();
void handle_scroll_down();
void handle_scroll_up();
void handle_start_find();
void handle_find_key(char);

UI *ui = NULL;

WINDOW *file_window = NULL;
WINDOW *title_window = NULL;
WINDOW *index_window = NULL;
WINDOW *status_window = NULL;
WINDOW *help_window = NULL;

char status_message[512];
const char *username;
char hostname[100];

/* Select next file in view */
void handle_down_key()
{
    if (!ui->selected || !ui->selected->next) {
        return;
    }

    if (ui->selected == get_bottom_file(ui)) {
        handle_scroll_down();
    }

    ui->selected = ui->selected->next;
}

/* Select prev file in view */
void handle_up_key()
{
    if (!ui->selected || !ui->selected->prev) {
        return;
    }

    if (ui->selected == get_top_file(ui)) {
        handle_scroll_up();
    }

    ui->selected = ui->selected->prev;
}

/* move into selected directory or perform file action */
void handle_right_key()
{
    if (!ui->selected) {
        return;
    }

    change_dir(path_join(ui->dirname, ui->selected->name));
}

/* Move to parent directory */
void handle_left_key()
{
    change_dir(get_parent(ui->dirname));
}

/* Scroll file view down */
void handle_scroll_down()
{
    if (!ui->files) {
        return;
    }

    if (num_files_displayed(ui) >= ui->max_files) {
        ui->scroll++;

        if (!is_displayed(ui, ui->selected)) {
            ui->selected = get_file_at_index(ui, ui->scroll);
        }
    }
}

/* Scroll file view up */
void handle_scroll_up()
{
    if (!ui->files) {
        return;
    }

    if (ui->scroll > 0) {
        ui->scroll--;

        if (!is_displayed(ui, ui->selected)) {
            ui->selected = get_file_at_index(ui, ui->scroll);
        }
    }
}

/* select top most file in the view */
void handle_goto_top()
{
    if (!ui->files) {
        return;
    }
    ui->selected = get_top_file(ui);
}

/* select bottom most file in the view */
void handle_goto_bottom()
{
    if (!ui->files) {
        return;
    }
    ui->selected = get_bottom_file(ui);
}

/* start find mode */
void handle_start_find()
{
    if (!ui->files || !ui->selected) {
        return;
    }
    sprintf(status_message, "f");
    ui->mode = FIND_MODE;
}

/* find file with char prefix and exit find mode */
void handle_find_key(char ch)
{
    assert(ui->selected);
    ui->mode = NORMAL_MODE;
    status_message[0] = 0;

    File *A = ui->selected;
    File *B = get_bottom_file(ui);

    while (A && B && A != B->next) {
        if (A->name[0] == ch) {
            ui->selected = A;
            break;
        }

        A = A->next;
    }
}

void change_dir(char *newdir)
{
    assert(newdir);

    // directory unchanged
    if (strcmp(ui->dirname, newdir) == 0) {
        return;
    }

    if (is_valid_dir(newdir)) {
        set_directory(ui, newdir);
        log_debug("cd: %s", ui->dirname);
    } else {
        log_error("cd failed: %s", newdir);
        free(newdir);
    }
}

void display_title_window()
{
    assert(ui->dirname);

    werase(title_window);
    wmove(title_window, 0, 0);

    if (strcmp(ui->dirname, "/") == 0) {
        wprintw(title_window, "%s@%s:/%s\n", username, hostname, ui->dirname);
    } else {
        wprintw(title_window, "%s@%s:%s\n", username, hostname, ui->dirname);
    }

    wrefresh(title_window);
}

void display_file_window()
{
    assert(ui->dirname);
    /* log_debug("display_files"); */

    werase(file_window);
    wmove(file_window, 0, 0);

    if (!ui->selected) {
        wrefresh(file_window);
        return;
    }

    int max_name_width = 0;

    for (File *f = ui->files; f; f = f->next) {
        max_name_width = MAX(max_name_width, strlen(f->name));
    }

    max_name_width = MIN(max_name_width, UI_MAX_PATH_LENGTH);

    char buffer[256];

    /* log_debug("top file: %s", get_top_file(ui)->name); */
    /* log_debug("bottom file: %s", get_bottom_file(ui)->name); */

    for (File *f = get_top_file(ui); f && f != get_bottom_file(ui)->next; f = f->next) {
        snprintf(buffer, UI_MAX_PATH_LENGTH, "%s", f->name);
        size_t name_width = strlen(buffer);

        if (f->name[0] == '.') {
        } else if (S_ISLNK(f->stat.st_mode)) {
            wattron(file_window, COLOR_PAIR(COLOR_PAIR_SYMLINK));
        } else if (S_ISDIR(f->stat.st_mode)) {
            wattron(file_window, COLOR_PAIR(COLOR_PAIR_DIRECTORY));
        } else {
            wattron(file_window, COLOR_PAIR(COLOR_PAIR_NORMAL));
        }

        if (ui->selected == f) {
            wattron(file_window, A_BOLD);
            wprintw(file_window, "> %s", buffer);
            wattroff(file_window, A_BOLD);
        } else {
            wprintw(file_window, "  %s", buffer);
        }

        wattron(file_window, COLOR_PAIR(COLOR_PAIR_NORMAL));

        // padding
        for (int j = 0; j < max_name_width - name_width + 1; j++) {
            wprintw(file_window, " ");
        }

        /* wattron(file_window, COLOR_PAIR(COLOR_PAIR_SYMLINK)); */
        get_human_size(f->stat.st_size, buffer, sizeof buffer - 1);
        wprintw(file_window, "%-6s", buffer);

        /* wattron(file_window, COLOR_PAIR(COLOR_PAIR_INFO1)); */
        get_human_time(f->stat.st_mtim, buffer, sizeof buffer - 1);
        wprintw(file_window, "%-20s", buffer);

        /* wattron(file_window, COLOR_PAIR(COLOR_PAIR_INFO2)); */
        get_perm_string(f->stat.st_mode, buffer, sizeof buffer - 1);
        wprintw(file_window, "%-16s", buffer);

        wprintw(file_window, "\n");
    }

    wrefresh(file_window);
}

void display_index_window()
{
    assert(ui->dirname);

    werase(index_window);
    wmove(index_window, 0, 0);

    if (!ui->selected) {
        wrefresh(index_window);
        return;
    }

    char text[256];
    int len = snprintf(text, sizeof text - 1, "[%d/%d]", get_file_index(ui, ui->selected), get_num_files(ui));
    int width = COLS - COLS / 2;
    if (len > width) {
        text[width] = 0;
    }

    int padding = width - MIN(width, len);
    for (int i = 0; i < padding; i++) {
        wprintw(index_window, " ");
    }

    wprintw(index_window, "%s\n", text);
    wrefresh(index_window);
}

void display_status_window()
{
    werase(status_window);
    wmove(status_window, 0, 0);

    if (ui->mode == NORMAL_MODE) {
        wprintw(status_window, "-- NORMAL --");
    } else if (ui->mode == FIND_MODE) {
        wprintw(status_window, "-- FIND --");
    }

    if (strlen(status_message) > 0) {
        wprintw(status_window, "  %s", status_message);
    }

    wrefresh(status_window);
}

void display_help_window()
{
    werase(help_window);
    wmove(help_window, 0, 0);

    char keys[] = {'?', 'q', 'f', 'g', 'G', 'k', 'j', 'h', 'l', 'K', 'J', 'o'};
    char *items[] = {"Help", "Exit", "Jump",  "Goto Top",  "Goto Bottom", "Up",
                     "Down", "Left", "Right", "Scroll Up", "Scroll Down", "OtherCmd"};
    int count_per_row = 6;
    const int item_width = COLS / count_per_row;

    char trim[item_width + 1];

    for (int i = 0; i < sizeof(keys); i++) {
        snprintf(trim, item_width - 2, "%s", items[i]);

        wattron(help_window, COLOR_PAIR(COLOR_PAIR_RED));
        wprintw(help_window, "%c", keys[i]);
        wattron(help_window, COLOR_PAIR(COLOR_PAIR_WHITE));
        wprintw(help_window, " %s", trim);

        for (int j = 0; j < item_width - strlen(trim) - 2; j++) {
            wprintw(help_window, " ");
        }

        if (i % count_per_row == count_per_row - 1) {
            wprintw(help_window, "\n");
        }
    }

    wrefresh(help_window);
}

void quit(int exit_code)
{
    delwin(title_window);
    delwin(file_window);
    delwin(index_window);
    endwin();
    exit(exit_code);
}

int main(int argc, const char *argv[])
{
    int logfile = open("stderr.log", O_CREAT | O_APPEND | O_WRONLY, 0640);
    dup2(logfile, 2);
    close(logfile);

    ui = calloc(1, sizeof *ui);

    if (argc < 2) {
        const char *home_path = getenv("HOME");
        if (!home_path) {
            log_error("getenv");
            return 1;
        }

        set_directory(ui, strdup(home_path));

    } else {
        char *dirname = realpath(argv[1], NULL);
        if (!dirname) {
            log_error("realpath");
            return 1;
        }

        set_directory(ui, dirname);
    }

    log_debug("current directory: %s", ui->dirname);

    gethostname(hostname, sizeof hostname - 1);

    username = getenv("USER");
    assert(username);

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (!has_colors()) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }

    ui->max_width = COLS;
    ui->max_files = MIN(LINES / 2, UI_MAX_LINES);
    title_window = newwin(2, COLS / 2, 0, 0);
    index_window = newwin(2, COLS - (COLS / 2), 0, COLS / 2);
    file_window = newwin(ui->max_files, COLS, 2, 0);
    status_window = newwin(1, COLS, 1 + 2 + ui->max_files, 0);
    help_window = newwin(2, COLS, 2 + 3 + ui->max_files, 0);

    log_debug("max files: %d", ui->max_files);

    clear();

    start_color();

    /* index, foreground, background */
    init_pair(COLOR_PAIR_NORMAL, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_PAIR_DIRECTORY, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_PAIR_SYMLINK, COLOR_YELLOW, COLOR_BLACK);

    init_pair(COLOR_PAIR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_PAIR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_PAIR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_PAIR_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_PAIR_YELLOW, COLOR_YELLOW, COLOR_BLACK);

    refresh();

    // main loop
    while (1) {
        display_title_window();
        display_index_window();
        display_file_window();
        display_help_window();
        display_status_window();

        int ch = getch();

        switch (ui->mode) {
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
