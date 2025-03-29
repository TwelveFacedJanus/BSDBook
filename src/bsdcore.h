#ifndef BSDCORE_H_
#define BSDCORE_H_


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
#include <ncurses.h>



#define DEFAULT_BOOKS_PATH "$HOME/books"

/* ==============================================================================================
 *
 *     @BRIEF:
 *     		This functions generate string path to $HOME/folder.
 *     @DESCRIPTION:
 *     		Function use stdlib.h and stdio.h files. It's exit from program if something went
 *     		wrong.
 *     @PARAMETERS:
 *     		- None (void)
 *     @RETURN:
 *     		- Character string of $HOME/folder.
 *     @NOTES:
 *   		- Its allocate memory for default_books_path. Maybe i need to dealloc memory for it?
 *   		- [FIXED UPDATE]: 03.29.25. Memory deallocation and now returns "" string if something went
 *   		wrong.
 *     @EXAMPLE:
 *     		```c
 *     		#include <stdio.h>
 *     		#include <stdlib.h>
 *
 *     		int main(void) {
 *			const char* books_path = get_default_books_path();
 *			if (books_path) {
 *				...;
 *			}
 *     		}
 *     		```
 *     @UPDATES:
 *     	03.29.25 [ Daniil (TwelveFacedJanus) Ermolaev ] - [FIXED]:
 *     		If something went wrong, function destroy program with EXIT_FAILURE error code,
 *   		but allocated memory for
 *   		structs and etc doesn't deallocate automatically.
 *
 =========================================================================================*/
char* get_default_books_path(const char* path) {
    char* home_dir = getenv("HOME");
    if (home_dir == NULL) {
        perror("Unable to get HOME directory");
        return "";
    }
    char* default_books_path = malloc(strlen(home_dir) + strlen(path) + 1);
    if (default_books_path == NULL) {
	free(default_books_path);
        perror("Unable to allocate memory");
        return "";
    }
    sprintf(default_books_path, "%s/%s", home_dir, path);
    return default_books_path;
}


/* =======================================================================================
 * 
 *    @BRIEF:
 *    		Function that creates .bdsb document ( note ) in $HOME/books folder.
 *    @DESCRIPTION:
 *    		Function use stdlib and stdio libraries for creating note in $HOME/books directory.
 *    @PARAMETERS:
 *    		const char *bookname - name of book;
 *    		const char *notename - name of note.
 *    @RETURN:
 *    		1) 0 if all ok and note has been created;
 *    		2) -1 if something went wrong.
 *    @NOTES:
 *    		None.
 *    @EXAMPLE:
 *    		```c
 *    		if (create_note("myfuckingnote", "mybooks") == 0) {
 *			printf("Note has been created!\n");
 *    		}
 *    		```
 *    @UPDATES:
 *    	03.29.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [DOC]
 *    		Documentation of this function has been created.
 *
 *==============================================================================================*/
int create_note(const char* bookname, const char* notename)
{
    char* default_books_path = get_default_books_path("/books");
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

/* =======================================================================================
 * 
 *    @BRIEF:
 *    		Function that creates new books with bookname.
 *    @DESCRIPTION:
 *    		Function use stdlib and stdio libraries for creating book in $HOME/books directory.
 *    @PARAMETERS:
 *    		const char *bookname - name of new book.
 *    @RETURN:
 *    		1) 0 if all ok and note has been created;
 *    		2) -1 if something went wrong.
 *    @NOTES:
 *    		None.
 *    @EXAMPLE:
 *    		```c
 *    		if (create_book("mybooks") == 0) {
 *			printf("Book has been created!\n");
 *    		}
 *    		```
 *    @UPDATES:
 *    	03.29.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [DOC]
 *    		Documentation of this function has been created.
 *
 *==============================================================================================*/
int create_book(const char* bookname)
{
    char* default_books_path = get_default_books_path("/books");
    char book_path[1024];
    snprintf(book_path, sizeof(book_path), "%s/%s", default_books_path, bookname);

    free(default_books_path);

    if (mkdir(book_path, 0755) == 0)
    {
        free(&book_path);
        printf("Book has been created!\n");
        return 0;
    }
    printf("Failed to create book directory: %s\n", strerror(errno));
    return -1;
}

/* =======================================================================================
 * 
 *    @BRIEF:
 *    		Prints books (directories) from $HOME/books folder.
 *    @DESCRIPTION:
 *    		Function use stdlib and stdio libraries for printing every book at $HOME/books directory.
 *    @PARAMETERS:
 *    		- None.
 *    @RETURN:
 *    		- None.
 *    @NOTES:
 *    		None.
 *    @EXAMPLE:
 *    		```c
 *    		get_books();
 *    		```
 *    @UPDATES:
 *    	03.29.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [DOC]
 *    		Documentation of this function has been created.
 *
 *==============================================================================================*/

void get_books() {
    char* default_books_path = get_default_books_path("/books");
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
    char* default_books_path = get_default_books_path("/books");
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
    char* default_books_path = get_default_books_path("/books");
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


#endif
