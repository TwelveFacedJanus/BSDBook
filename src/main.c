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
#include "./bsdcore.h"

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
            char* default_books_path = get_default_books_path("/books");
            char book_path[1024];
            snprintf(book_path, sizeof(book_path), "%s/%s", default_books_path, argv[3]);
            free(default_books_path);
            if (delete_folder_recursive(book_path) == 0)
                printf("Book has been deleted!\n");
        } else if (argc >= 5 && strcmp(argv[2], "note") == 0) {
            char* default_books_path = get_default_books_path("/books");
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
        char* default_books_path = get_default_books_path("/books");
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
