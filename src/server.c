#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <jansson.h>
#include "bsdcore.h"

#define PORT 8080
#define BUFFER_SIZE 4096

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
 *          None.
 *
 =========================================================================================*/
json_t* books_to_json(Book* books, int count) {
    json_t* root = json_array();
    if (!root) return NULL;

    for (int i = 0; i < count; i++) {
        json_t* book_obj = json_object();
        if (!book_obj) {
            json_decref(root);
            return NULL;
        }

        json_object_set_new(book_obj, "name", json_string(books[i].name));
        json_object_set_new(book_obj, "notes_count", json_integer(books[i].notes_count));

        json_array_append_new(root, book_obj);
    }

    return root;
}

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
 *          None.
 *
 =========================================================================================*/
json_t* notes_to_json(Note* notes, int count) {
    json_t* root = json_array();
    if (!root) return NULL;

    for (int i = 0; i < count; i++) {
        json_t* note_obj = json_object();
        if (!note_obj) {
            json_decref(root);
            return NULL;
        }

        json_object_set_new(note_obj, "name", json_string(notes[i].name));
        json_array_append_new(root, note_obj);
    }

    return root;
}

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
 *          None.
 *
 =========================================================================================*/
int handle_note_content_request(int client_socket, const char* path) {
    char book_name[256] = {0};
    char note_name[256] = {0};
    
    // Parse book and note names from path
    if (sscanf(path, "/book/%255[^/]/%255s", book_name, note_name) != 2) {
        const char* bad_request = "HTTP/1.1 400 Bad Request\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "\r\n"
                                 "400 Bad Request - Invalid path format\r\n";
        write(client_socket, bad_request, strlen(bad_request));
        return -1;
    }

    // Get note content
    char* content = get_note_content(book_name, note_name);
    if (!content) {
        const char* not_found = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Type: text/plain\r\n"
                               "\r\n"
                               "404 Note Not Found\r\n";
        write(client_socket, not_found, strlen(not_found));
        return -1;
    }

    // Build response
    char response_header[512];
    snprintf(response_header, sizeof(response_header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "\r\n",
            strlen(content));

    // Send header and content
    write(client_socket, response_header, strlen(response_header));
    write(client_socket, content, strlen(content));

    free(content);
    return 0;
}



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
 *          None.
 *
 =========================================================================================*/
// Updated handle_http_request with complete implementation
int handle_http_request(int client_socket, const char* request) {
    char path[256] = {0};
    if (sscanf(request, "GET %255s HTTP/1.1", path) != 1) {
        const char* bad_request = "HTTP/1.1 400 Bad Request\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "\r\n"
                                 "400 Bad Request\r\n";
        write(client_socket, bad_request, strlen(bad_request));
        return -1;
    }

    if (strcmp(path, "/books") == 0) {
        // Handle books listing
        int book_count = 0;
        Book* books = get_books_st(&book_count);
        if (!books) {
            const char* not_found = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "\r\n"
                                   "404 No Books Found\r\n";
            write(client_socket, not_found, strlen(not_found));
            return -1;
        }

        json_t* books_json = books_to_json(books, book_count);
        if (!books_json) {
            const char* server_error = "HTTP/1.1 500 Internal Server Error\r\n"
                                     "Content-Type: text/plain\r\n"
                                     "\r\n"
                                     "500 JSON Conversion Failed\r\n";
            write(client_socket, server_error, strlen(server_error));
            
            // Free books array
            for (int i = 0; i < book_count; i++) {
                free(books[i].name);
            }
            free(books);
            return -1;
        }

        char* json_str = json_dumps(books_json, JSON_INDENT(2));
        json_decref(books_json);
        
        // Free books array
        for (int i = 0; i < book_count; i++) {
            free(books[i].name);
        }
        free(books);

        if (!json_str) {
            const char* server_error = "HTTP/1.1 500 Internal Server Error\r\n"
                                     "Content-Type: text/plain\r\n"
                                     "\r\n"
                                     "500 JSON Serialization Failed\r\n";
            write(client_socket, server_error, strlen(server_error));
            return -1;
        }

        char response_header[512];
        snprintf(response_header, sizeof(response_header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %zu\r\n"
                "\r\n",
                strlen(json_str));

        write(client_socket, response_header, strlen(response_header));
        write(client_socket, json_str, strlen(json_str));
        free(json_str);
    }
    else if (strncmp(path, "/books/", 7) == 0) {
        // Handle notes listing for a book
        char book_name[256] = {0};
        strncpy(book_name, path + 7, sizeof(book_name) - 1);

        int note_count = 0;
        Note* notes = get_notes_st(book_name, &note_count);
        if (!notes) {
            const char* not_found = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "\r\n"
                                   "404 No Notes Found\r\n";
            write(client_socket, not_found, strlen(not_found));
            return -1;
        }

        json_t* notes_json = notes_to_json(notes, note_count);
        if (!notes_json) {
            const char* server_error = "HTTP/1.1 500 Internal Server Error\r\n"
                                     "Content-Type: text/plain\r\n"
                                     "\r\n"
                                     "500 JSON Conversion Failed\r\n";
            write(client_socket, server_error, strlen(server_error));
            
            // Free notes array
            for (int i = 0; i < note_count; i++) {
                free(notes[i].name);
            }
            free(notes);
            return -1;
        }

        char* json_str = json_dumps(notes_json, JSON_INDENT(2));
        json_decref(notes_json);
        
        // Free notes array
        for (int i = 0; i < note_count; i++) {
            free(notes[i].name);
        }
        free(notes);

        if (!json_str) {
            const char* server_error = "HTTP/1.1 500 Internal Server Error\r\n"
                                     "Content-Type: text/plain\r\n"
                                     "\r\n"
                                     "500 JSON Serialization Failed\r\n";
            write(client_socket, server_error, strlen(server_error));
            return -1;
        }

        char response_header[512];
        snprintf(response_header, sizeof(response_header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %zu\r\n"
                "\r\n",
                strlen(json_str));

        write(client_socket, response_header, strlen(response_header));
        write(client_socket, json_str, strlen(json_str));
        free(json_str);
    }
    else if (strncmp(path, "/book/", 6) == 0) {
        // Handle note content request
        return handle_note_content_request(client_socket, path);
    }
    else {
        const char* not_found = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Type: text/plain\r\n"
                               "\r\n"
                               "404 Not Found\r\n";
        write(client_socket, not_found, strlen(not_found));
        return -1;
    }

    return 0;
}

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
 *          None.
 *
 =========================================================================================*/
int run_http_server() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return -1;
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return -1;
    }

    printf("BSDBook HTTP server running on port %d\n", PORT);

    while (1) {
        // Accept connection
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        // Read request
        char buffer[BUFFER_SIZE] = {0};
        ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0) {
            perror("read");
            close(client_socket);
            continue;
        }

        // Handle request
        handle_http_request(client_socket, buffer);
        close(client_socket);
    }

    return 0;
}


int main() {
	run_http_server();
}
