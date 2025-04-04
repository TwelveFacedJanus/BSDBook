/*=================================================================================================
 * 	
 * 	@FILENAME:	      bsdbook.h
 * 	@BRIEF:    	      File implements core functions for bsdbook application.
 * 	@DESCRIPTION:	  None.
 * 	@AUTHOR		      Daniil (TwelveFacedJanus) Ermolaev.
 * 	   | CONTACT:	  twofaced-janus@yandex.ru
 *	@CREATED AT:	  03.30.25
 *	@UPDATED_AT:	  04.03.25
 *
 *==================================================================================================*/

#ifndef BSDCORE_H_
#define BSDCORE_H_


#define _XOPEN_SOURCE 500
#define BSDBOOKSERVER_
#define DEFAULT_BOOKS_PATH "$HOME/books"

#if defined(BSDBOOKSERVER_)
#define PORT 8080
#define BUFFER_SIZE 4096
#endif // defined(BSDBOOKSERVER_)


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

// Libraries for server
#include <sys/socket.h>
#include <netinet/in.h>
#include <jansson.h>


/*===============================================================================================
 *
 * 	@BRIEF:
 * 		Structs that realize books and notes.
 * 	@DESCRIPTION:
 * 		None.
 * 	@PARAMETERS:
 * 		Note.name - char*;
 * 		Book.name - char*;
 * 		Book.notes - Note*.
 * 	@RETURN:
 * 		None.
 * 	@NOTES:
 * 		None.
 * 	@EXAMPLE:
 * 		```c
 * 		Note n = (Note){.name = "Jopa"};
 * 		Book k = (Book){.name = "JopaCore", .notes = { n }};
 * 		```
 * 	@UPDATES:
 *	 03.30.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [NEW]:
 *	 	      Structs has been created.
 *	 03.30.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [DOC]:
 *	 	      Documentation has been created.
 *   04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *            Implementation of structs moved to bsdcode.c file.
 *
 * =============================================================================================*/
typedef struct Note Note;
typedef struct Book Book;




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
 *     	03.29.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FIXED]:
 *     		     If something went wrong, function destroy program with EXIT_FAILURE error code,
 *   		     but allocated memory for
 *   		     structs and etc doesn't deallocate automatically.
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
char* get_default_books_path(const char* path);


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
 *    		     Documentation of this function has been created.
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 *==============================================================================================*/
int create_note(const char* bookname, const char* notename);

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Gets the content of a specific note from a book.
 *     @DESCRIPTION:
 *          Reads the content of a .bdsb note file and returns it.
 *     @PARAMETERS:
 *          - const char* book_name: Name of the book
 *          - const char* note_name: Name of the note (without .bdsb extension)
 *     @RETURN:
 *          - char*: Content of the note (must be freed by caller)
 *          - NULL if error occurs
 *     @NOTES:
 *          - Allocates memory for the returned content
 *     @EXAMPLE:
 *          ```c
 *          char* content = get_note_content("Programming", "C_Tips");
 *          if (content) {
 *              printf("%s\n", content);
 *              free(content);
 *          }
 *          ```
 *     @UPDATES:
 *       04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *                Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
char* get_note_content(const char* book_name, const char* note_name);


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
 *    		     Documentation of this function has been created.
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 *==============================================================================================*/
int create_book(const char* bookname);

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
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *          Implementation of this function moved to bsdcode.c file.
 *
 *==============================================================================================*/
void get_books();

/* =======================================================================================
 * 
 *    @BRIEF:
 *    		Returns array of books (directories) from $HOME/books folder.
 *    @DESCRIPTION:
 *    		Function use stdlib and stdio libraries for printing every book at $HOME/books directory.
 *    @PARAMETERS:
 *    		- count_of_books
 *    @RETURN:
 *    		- Book* - array of books.
 *    @NOTES:
 *    		None.
 *    @EXAMPLE:
 *    		```c
 *     		 int book_count;
 *           Book* books = get_books(&book_count);
 *           
 *           if (books) {
 *               for (int i = 0; i < book_count; i++) {
 *                   printf("Book: %s\n", books[i].name);
 *                   // Освобождаем память для каждого имени книги
 *                   free(books[i].name);
 *               }
 *               // Освобождаем массив книг
 *               free(books);
 *           }
 *           
 *           return 0;
 *    		```
 *    @UPDATES:
 *    	03.31.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [DOC]
 *    		Documentation for this function has been created.
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *          Implementation of this function moved to bsdcode.c file.
 *
 *==============================================================================================*/
Book* get_books_st(int* count);

/* =======================================================================================
 * 
 *    @BRIEF:
 *          Returns array of notes from specified book.
 *    @DESCRIPTION:
 *          Scans the specified book directory and returns all notes (.bdsb files) as an array
 *          of Note structures.
 *    @PARAMETERS:
 *          - const char* bookname: Name of the book to scan
 *          - int* count: Pointer to store the number of notes found
 *    @RETURN:
 *          - Note*: Dynamically allocated array of Note structures
 *          - NULL if error occurs
 *    @NOTES:
 *          - Caller is responsible for freeing both the array and individual note names
 *          - Only returns .bdsb files
 *    @EXAMPLE:
 *          ```c
 *          int note_count;
 *          Note* notes = get_notes_st("mybook", &note_count);
 *          if (notes) {
 *              for (int i = 0; i < note_count; i++) {
 *                  printf("Note: %s\n", notes[i].name);
 *                  free(notes[i].name);
 *              }
 *              free(notes);
 *          }
 *          ```
 *    @UPDATES:
 *      03.31.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [NEW]
 *               Function created.
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 * =======================================================================================*/
Note* get_notes_st(const char* bookname, int* count);

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Callback function for nftw() to delete files and directories recursively.
 *     @DESCRIPTION:
 *          This function is used as a callback by nftw() to remove files and directories.
 *          It's called for each file/directory in the hierarchy.
 *     @PARAMETERS:
 *          - const char* fpath: Path to the current file/directory
 *          - const struct stat* sb: Pointer to stat structure with file info
 *          - int typeflag: File type flag
 *          - struct FTW* ftwbuf: FTW structure with additional info
 *     @RETURN:
 *          - 0 on success, -1 on error
 *     @NOTES:
 *          - Used internally by delete_folder_recursive()
 *     @EXAMPLE:
 *          ```c
 *          nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
 *          ```
 *     @UPDATES:
 *       04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *                Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
int unlink_cb(const char* fpath, const struct stat *sb, int typeflag, struct FTW* ftwbuf);

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Deletes a folder and all its contents recursively.
 *     @DESCRIPTION:
 *          Uses nftw() to traverse directory tree and delete all files and subdirectories.
 *     @PARAMETERS:
 *          - const char* fpath: Path to directory to delete
 *     @RETURN:
 *          - 0 on success, -1 on error
 *     @NOTES:
 *          - Uses unlink_cb() as callback function
 *          - Deletes everything including the root directory
 *     @EXAMPLE:
 *          ```c
 *          delete_folder_recursive("/path/to/folder");
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
int delete_folder_recursive(const char* fpath);

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Displays welcome message and usage help.
 *     @DESCRIPTION:
 *          Prints basic information about the program and available commands.
 *     @PARAMETERS:
 *          - None
 *     @RETURN:
 *          - None
 *     @NOTES:
 *          - Called when program starts without arguments or with help flag
 *     @EXAMPLE:
 *          ```c
 *          show_welcome_and_help();
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
__attribute__((visibility("default")))
void show_welcome_and_help();

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Checks if a path is a directory.
 *     @DESCRIPTION:
 *          Uses stat() to check if the given path refers to a directory.
 *     @PARAMETERS:
 *          - const char* path: Path to check
 *     @RETURN:
 *          - 1 if path is directory, 0 otherwise
 *     @NOTES:
 *          - Returns 0 if path cannot be accessed
 *     @EXAMPLE:
 *          ```c
 *          if (is_directory("/path/to/check")) {
 *              // It's a directory
 *          }
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
int is_directory(const char *path);

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Checks if a path is a regular file.
 *     @DESCRIPTION:
 *          Uses stat() to check if the given path refers to a regular file.
 *     @PARAMETERS:
 *          - const char* path: Path to check
 *     @RETURN:
 *          - 1 if path is regular file, 0 otherwise
 *     @NOTES:
 *          - Returns 0 if path cannot be accessed
 *     @EXAMPLE:
 *          ```c
 *          if (is_regular_file("/path/to/check")) {
 *              // It's a regular file
 *          }
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
int is_regular_file(const char *path);

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Searches all notes for lines containing a specific tag.
 *     @DESCRIPTION:
 *          Recursively searches through all books and notes for lines containing the given tag.
 *          Prints matching lines with book and note information.
 *     @PARAMETERS:
 *          - const char* tag: Tag to search for (e.g. "#todo")
 *     @RETURN:
 *          - None
 *     @NOTES:
 *          - Uses get_default_books_path() to locate books directory
 *          - Prints results to stdout
 *     @EXAMPLE:
 *          ```c
 *          find_by_tag("#important");
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
void find_by_tag(const char* tag);

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Shows all TODO items from all notes.
 *     @DESCRIPTION:
 *          Wrapper around find_by_tag() that searches for "#todo" tags.
 *     @PARAMETERS:
 *          - None
 *     @RETURN:
 *          - None
 *     @NOTES:
 *          - Calls find_by_tag("#todo")
 *     @EXAMPLE:
 *          ```c
 *          show_todos();
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
void show_todos();

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Shows all link items from all notes.
 *     @DESCRIPTION:
 *          Wrapper around find_by_tag() that searches for "#link" tags.
 *     @PARAMETERS:
 *          - None
 *     @RETURN:
 *          - None
 *     @NOTES:
 *          - Calls find_by_tag("#link")
 *     @EXAMPLE:
 *          ```c
 *          show_links();
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
void show_links();

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Lists all notes in a book with their last modification time.
 *     @DESCRIPTION:
 *          Prints all notes in the specified book along with their last edited timestamp.
 *     @PARAMETERS:
 *          - const char* book_name: Name of book to show notes from
 *     @RETURN:
 *          - None
 *     @NOTES:
 *          - Uses stat() to get file modification time
 *          - Prints to stdout
 *     @EXAMPLE:
 *          ```c
 *          print_notes_from_book("mybook");
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
void print_notes_from_book(const char *book_name);

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Converts an array of Book structures to JSON format.
 *     @DESCRIPTION:
 *          Takes a Book array and converts it to a JSON array of objects.
 *     @PARAMETERS:
 *          - Book* books: Array of Book structures
 *          - int count: Number of books in the array
 *     @RETURN:
 *          - json_t*: JSON array containing book data
 *          - NULL if error occurs
 *     @NOTES:
 *          - Caller is responsible for freeing the returned JSON object
 *     @EXAMPLE:
 *          ```c
 *          json_t* books_json = books_to_json(books, count);
 *          if (books_json) {
 *              char* json_str = json_dumps(books_json, JSON_INDENT(2));
 *              printf("%s\n", json_str);
 *              free(json_str);
 *              json_decref(books_json);
 *          }
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
json_t* books_to_json(Book* books, int count);


/* ==============================================================================================
 *
 *     @BRIEF:
 *          Converts an array of Note structures to JSON format.
 *     @DESCRIPTION:
 *          Takes a Note array and converts it to a JSON array of objects.
 *     @PARAMETERS:
 *          - Note* notes: Array of Note structures
 *          - int count: Number of notes in the array
 *     @RETURN:
 *          - json_t*: JSON array containing note data
 *          - NULL if error occurs
 *     @NOTES:
 *          - Caller is responsible for freeing the returned JSON object
 *     @EXAMPLE:
 *          ```c
 *          json_t* notes_json = notes_to_json(notes, count);
 *          if (notes_json) {
 *              char* json_str = json_dumps(notes_json, JSON_INDENT(2));
 *              printf("%s\n", json_str);
 *              free(json_str);
 *              json_decref(notes_json);
 *          }
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
json_t* notes_to_json(Note* notes, int count);

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Handles requests for specific note content.
 *     @DESCRIPTION:
 *          Processes requests to /book/{book_name}/{note_name} endpoint.
 *     @PARAMETERS:
 *          - int client_socket: Client socket descriptor
 *          - const char* path: Request path
 *     @RETURN:
 *          - 0 on success, -1 on error
 *     @NOTES:
 *          - Returns note content as plain text
 *     @EXAMPLE:
 *          ```c
 *          handle_note_content_request(client_sock, "/book/Programming/C_Tips");
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
int handle_note_content_request(int client_socket, const char* path);

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Updated HTTP request handler with note content support.
 *     @DESCRIPTION:
 *          Handles all HTTP requests including the new note content endpoint.
 *     @PARAMETERS:
 *          - int client_socket: Client socket descriptor
 *          - const char* request: HTTP request string
 *     @RETURN:
 *          - 0 on success, -1 on error
 *     @NOTES:
 *          - Now supports /books, /books/{book}, and /book/{book}/{note}
 *     @EXAMPLE:
 *          ```c
 *          handle_http_request(client_sock, "GET /book/Programming/C_Tips HTTP/1.1");
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
int handle_http_request(int client_socket, const char* request);

/* ==============================================================================================
 *
 *     @BRIEF:
 *          Main HTTP server function.
 *     @DESCRIPTION:
 *          Creates and runs an HTTP server that listens for connections and handles requests.
 *     @PARAMETERS:
 *          - None
 *     @RETURN:
 *          - 0 on success, -1 on error
 *     @NOTES:
 *          - Listens on port 8080 by default
 *          - Uses handle_http_request() to process each request
 *     @EXAMPLE:
 *          ```c
 *          int main() {
 *              return run_http_server();
 *          }
 *          ```
 *     @UPDATES:
 *      04.03.25 - [ Daniil (TwelveFacedJanus) Ermolaev ] - [FEATURE]:
 *               Implementation of this function moved to bsdcode.c file.
 *
 =========================================================================================*/
int run_http_server();
#endif
