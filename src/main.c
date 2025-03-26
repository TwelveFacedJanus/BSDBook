#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <ftw.h>
#include <errno.h>
#include <time.h>
#include <ncurses.h> // For TUI

#define DEFAULT_BOOKS_PATH "$HOME/books"

char* get_default_books_path() {
    char* home_dir = getenv("HOME");
    if (home_dir == NULL) {
        perror("Unable to get HOME directory");
        exit(EXIT_FAILURE);
    }
    char* default_books_path = malloc(strlen(home_dir) + strlen("/books") + 1);
    if (default_books_path == NULL) {
        perror("Unable to allocate memory");
        exit(EXIT_FAILURE);
    }
    sprintf(default_books_path, "%s/books", home_dir);
    return default_books_path;
}

int create_note(const char* bookname, const char* notename)
{
    char* default_books_path = get_default_books_path();
    char note_path[1024];
    snprintf(note_path, sizeof(note_path), "%s/%s/%s.bdsb", default_books_path, bookname, notename);

    char book_path[1024];
    snprintf(book_path, sizeof(book_path), "%s/%s", default_books_path, bookname);

    free(default_books_path);

    struct stat book_info;
    if (stat(book_path, &book_info) != 0 || !(book_info.st_mode & S_IFDIR))
    {
        printf("Book directory does not exist. Please run 'bsdbook init default'.\n");
        return -1;
    }

    if (access(note_path, F_OK) == 0)
    {
        printf("Note already exists!\n");
        return -1;
    }

    FILE* fp = fopen(note_path, "a");
    if (fp == NULL)
    {
        printf("Error creating Note: %s\n", strerror(errno));
        return -1;
    }
    fclose(fp);
    printf("Note has been created!\n");
    return 0;
}

int create_book(const char* bookname)
{
    char* default_books_path = get_default_books_path();
    char book_path[1024];
    snprintf(book_path, sizeof(book_path), "%s/%s", default_books_path, bookname);

    free(default_books_path);

    if (mkdir(book_path, 0755) == 0)
    {
        printf("Book has been created!\n");
        return 0;
    }
    printf("Failed to create book directory: %s\n", strerror(errno));
    return -1;
}

int install_bsdbook()
{
    char* default_books_path = get_default_books_path();
    struct stat info;
    if (stat(default_books_path, &info) != 0) {
        printf("Directory does not exist. Creating...\n");
        if (mkdir(default_books_path, 0755) == 0) {
            printf("Directory created!\n");
            free(default_books_path);
            return 0;
        }
        printf("Failed to create directory: %s\n", strerror(errno));
        free(default_books_path);
        return -1;
    } else if (info.st_mode & S_IFDIR) {
        printf("Directory already exists.\n");
        free(default_books_path);
        return -1;
    }
    printf("Path exists but is not a directory.\n");
    free(default_books_path);
    return -1;
}

void get_books() {
    char* default_books_path = get_default_books_path();
    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;

    dir = opendir(default_books_path);
    if (!dir) {
        perror("opendir");
        free(default_books_path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", default_books_path, entry->d_name);

        if (stat(fullpath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            printf("%s\n", entry->d_name);
        }
    }
    closedir(dir);
    free(default_books_path);
}

int unlink_cb(const char* fpath, const struct stat *sb, int typeflag, struct FTW* ftwbuf) {
    int rv = remove(fpath);
    if (rv)
        perror("ERROR unlinking cb.\n");
    return rv;
}

int delete_folder_recursive(const char* fpath) {
    return nftw(fpath, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void show_welcome_and_help() {
    printf("Welcome to BSDNotes!\n");
    printf("Usage:\n");
    printf("  ./bsdnotes install                  - Install BSDNotes\n");
    printf("  ./bsdnotes create book <book_name>  - Create a new book\n");
    printf("  ./bsdnotes create note <book_name> <note_name> - Create a new note in a book\n");
    printf("  ./bsdnotes delete book <book_name>  - Delete a book\n");
    printf("  ./bsdnotes delete note <book_name> <note_name> - Delete a note from a book\n");
    printf("  ./bsdnotes show <book_name>         - Show all notes in a book\n");
    printf("  ./bsdnotes books                    - List all books\n");
    printf("  ./bsdnotes edit <book_name> <note_name> - Edit a note in a book using NeoVim\n");
    printf("  ./bsdnotes show todos               - Show all lines with #todo tag from all notes\n");
    printf("  ./bsdnotes --tui                    - Open BSDNotes in TUI mode\n");
}

int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0; // Cannot access, assume not a directory
    return S_ISDIR(statbuf.st_mode);
}

int is_regular_file(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0; // Cannot access, assume not a regular file
    return S_ISREG(statbuf.st_mode);
}

void find_by_tag(const char* tag) {
    char* default_books_path = get_default_books_path();
    DIR *books_dir = opendir(default_books_path);
    if (!books_dir) {
        perror("Unable to open 'books' directory");
        free(default_books_path);
        return;
    }

    struct dirent *book_entry;
    while ((book_entry = readdir(books_dir))) {
        char book_path[1024];
        snprintf(book_path, sizeof(book_path), "%s/%s", default_books_path, book_entry->d_name);

        if (is_directory(book_path) && strcmp(book_entry->d_name, ".") != 0 && strcmp(book_entry->d_name, "..") != 0) {
            DIR *notes_dir = opendir(book_path);
            if (!notes_dir) {
                perror("Unable to open book directory");
                continue;
            }

            struct dirent *note_entry;
            while ((note_entry = readdir(notes_dir))) {
                char note_path[1024];
                snprintf(note_path, sizeof(note_path), "%s/%s", book_path, note_entry->d_name);

                if (is_regular_file(note_path)) {
                    FILE *note_file = fopen(note_path, "r");
                    if (!note_file) {
                        perror("Unable to open note file");
                        continue;
                    }

                    char line[1024];
                    int line_number = 1;
                    while (fgets(line, sizeof(line), note_file)) {
                        if (strstr(line, tag)) {
                            printf("[Book: %s, Note: %s, Line %d] %s", book_entry->d_name, note_entry->d_name, line_number, line);
                        }
                        line_number++;
                    }

                    fclose(note_file);
                }
            }

            closedir(notes_dir);
        }
    }

    closedir(books_dir);
    free(default_books_path);
}

void show_todos() {
    find_by_tag("#todo");
}

void show_links() {
    find_by_tag("#link");
}


void print_notes_from_book(const char *book_name) {
    char* default_books_path = get_default_books_path();
    char book_path[1024];
    snprintf(book_path, sizeof(book_path), "%s/%s", default_books_path, book_name);

    DIR *notes_dir = opendir(book_path);
    if (!notes_dir) {
        perror("Unable to open book directory");
        free(default_books_path);
        return;
    }

    printf("Notes in book '%s':\n", book_name);
    struct dirent *note_entry;
    while ((note_entry = readdir(notes_dir))) {
        char note_path[1024];
        snprintf(note_path, sizeof(note_path), "%s/%s", book_path, note_entry->d_name);

        if (is_regular_file(note_path)) {
            // Get file metadata
            struct stat file_stat;
            if (stat(note_path, &file_stat) != 0) {
                perror("Unable to get file stats");
                continue;
            }

            // Convert last modification time to a readable format
            char time_buf[80];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));

            // Print note name and last editing time
            printf("- %s (Last Edited: %s)\n", note_entry->d_name, time_buf);
        }
    }

    closedir(notes_dir);
    free(default_books_path);
}

void show_tui_help(WINDOW *win) {
    // Clear the window and display help information
    wclear(win);
    wprintw(win, "BSDNotes TUI Help\n");
    wprintw(win, "-----------------\n");
    wprintw(win, "1 - List books\n");
    wprintw(win, "2 - Show todos\n");
    wprintw(win, "h - Show this help screen\n");
    wprintw(win, "q - Quit\n");
    wrefresh(win);
}

void show_tui() {
    // Initialize ncurses
    initscr(); // Start curses mode
    cbreak();  // Disable line buffering
    noecho();  // Don't echo input
    keypad(stdscr, TRUE); // Enable special keys

    // Create a window for help information
    WINDOW *help_win = newwin(10, 50, 0, 0);
    box(help_win, 0, 0); // Draw a border around the window

    // Print a welcome message
    printw("Welcome to BSDNotes TUI!\n");
    printw("Press 'h' for help, 'q' to quit.\n");
    refresh();

    // Main loop
    int ch;
    while ((ch = getch()) != 'q') {
        // Handle input
        switch (ch) {
            case '1':
                printw("You pressed 1: Listing books...\n");
                // Call get_books() or similar function
                break;
            case '2':
                printw("You pressed 2: Showing todos...\n");
                // Call show_todos() or similar function
                break;
            case 'h':
                show_tui_help(help_win);
                break;
            default:
                printw("Unknown command. Press 'h' for help.\n");
                break;
        }
        refresh();
    }

    // End ncurses
    delwin(help_win);
    endwin();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        show_welcome_and_help();
        return 0;
    }

    // Check for --tui flag
    if (argc >= 2 && strcmp(argv[1], "--tui") == 0) {
        show_tui();
        return 0;
    }

    if (strcmp(argv[1], "install") == 0) {
        install_bsdbook();
    } else if (argc >= 2 && strcmp(argv[1], "delete") == 0) {
        if (argc >= 4 && strcmp(argv[2], "book") == 0) {
            char* default_books_path = get_default_books_path();
            char book_path[1024];
            snprintf(book_path, sizeof(book_path), "%s/%s", default_books_path, argv[3]);
            free(default_books_path);
            if (delete_folder_recursive(book_path) == 0)
                printf("Book has been deleted!\n");
        } else if (argc >= 5 && strcmp(argv[2], "note") == 0) {
            char* default_books_path = get_default_books_path();
            char note_path[1024];
            snprintf(note_path, sizeof(note_path), "%s/%s/%s.bdsb", default_books_path, argv[3], argv[4]);
            free(default_books_path);
            if (remove(note_path) == 0)
                printf("Note has been deleted!\n");
        }
    } else if (argc >= 3 && strcmp(argv[1], "create") == 0) {
        if (strcmp(argv[2], "book") == 0 && argc >= 4) {
            create_book(argv[3]);
        } else if (strcmp(argv[2], "note") == 0 && argc >= 5) {
            create_note(argv[3], argv[4]);
        }
    } else if (strcmp(argv[1], "show") == 0) {
        if (argc >= 3 && strcmp(argv[2], "todos") == 0) {
            show_todos();
        } else if (argc >= 3 && strcmp(argv[2], "links") == 0) {
            show_links();
        } else if (argc >= 3) {
            print_notes_from_book(argv[2]);
        }
    } else if (strcmp(argv[1], "books") == 0) {
        get_books();
    } else if (strcmp(argv[1], "edit") == 0 && argc >= 4) {
        char* default_books_path = get_default_books_path();
        char note_path[1024];
        snprintf(note_path, sizeof(note_path), "%s/%s/%s.bdsb", default_books_path, argv[2], argv[3]);
        free(default_books_path);
        char command[1024];
        snprintf(command, sizeof(command), "nvim %s", note_path);
        system(command); // Open the note in NeoVim
    } else {
        show_welcome_and_help();
    }

    return 0;
}
