#include "./bsdcore.h"

typedef struct Note
{
	char* name;
} Note;


typedef struct Book
{
	char* name;
 	Note* notes;
    int32_t notes_count;
} Book;

char* get_default_books_path(const char* path)
{
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

char* get_note_content(const char* book_name, const char* note_name) {
    char* default_books_path = get_default_books_path("/books");
    char note_path[1024];
    snprintf(note_path, sizeof(note_path), "%s/%s/%s.bdsb", default_books_path, book_name, note_name);

    FILE* file = fopen(note_path, "r");
    if (!file) {
        free(default_books_path);
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer for content
    char* content = malloc(file_size + 1);
    if (!content) {
        fclose(file);
        free(default_books_path);
        return NULL;
    }

    // Read file content
    size_t bytes_read = fread(content, 1, file_size, file);
    content[bytes_read] = '\0';

    fclose(file);
    free(default_books_path);
    return content;
}

int create_book(const char* bookname)
{
    char* default_books_path = get_default_books_path("/books");
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

void get_books()
{
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

Book* get_books_st(int* count)
{
    char *default_books_path = get_default_books_path("/books");
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    Book* books = NULL;
    int book_count = 0;

    dir = opendir(default_books_path);
    if (!dir) {
        perror("opendir");
        free(default_books_path);
        *count = 0;
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", default_books_path, entry->d_name);

        if (stat(fullpath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            book_count++;
        }
    }

    books = (Book*)malloc(book_count * sizeof(Book));
    if (!books) {
        perror("malloc");
        closedir(dir);
        free(default_books_path);
        *count = 0;
        return NULL;
    }

    rewinddir(dir);
    int i = 0;
    while ((entry = readdir(dir)) != NULL && i < book_count) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", default_books_path, entry->d_name);

        if (stat(fullpath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            books[i].name = strdup(entry->d_name);
            books[i].notes = NULL;
            books[i].notes_count = 0;
            i++;
        }
    }

    closedir(dir);
    free(default_books_path);
    *count = book_count;
    return books;
}

Note* get_notes_st(const char* bookname, int* count)
 {
    char* default_books_path = get_default_books_path("/books");
    char book_path[1024];
    snprintf(book_path, sizeof(book_path), "%s/%s", default_books_path, bookname);

    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;
    Note* notes = NULL;
    int note_count = 0;

    // First pass - count .bdsb files
    dir = opendir(book_path);
    if (!dir) {
        perror("opendir");
        free(default_books_path);
        *count = 0;
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", book_path, entry->d_name);

        if (stat(fullpath, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
            // Check if file has .bdsb extension
            char* ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".bdsb") == 0) {
                note_count++;
            }
        }
    }

    // Allocate memory for notes array
    notes = (Note*)malloc(note_count * sizeof(Note));
    if (!notes) {
        perror("malloc");
        closedir(dir);
        free(default_books_path);
        *count = 0;
        return NULL;
    }

    // Second pass - fill the array
    rewinddir(dir);
    int i = 0;
    while ((entry = readdir(dir)) != NULL && i < note_count) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", book_path, entry->d_name);

        if (stat(fullpath, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
            char* ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".bdsb") == 0) {
                // Remove .bdsb extension for the note name
                *ext = '\0';
                notes[i].name = strdup(entry->d_name);
                i++;
            }
        }
    }

    closedir(dir);
    free(default_books_path);
    *count = note_count;
    return notes;
}

int unlink_cb(const char* fpath, const struct stat *sb, int typeflag, struct FTW* ftwbuf)
{
    int rv = remove(fpath);
    if (rv)
        perror("ERROR unlinking cb.\n");
    return rv;
}

int delete_folder_recursive(const char* fpath)
{
    return nftw(fpath, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

__attribute__((visibility("default")))
void show_welcome_and_help()
{
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
int is_directory(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0; // Cannot access, assume not a directory
    return S_ISDIR(statbuf.st_mode);
}

int is_regular_file(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0; // Cannot access, assume not a regular file
    return S_ISREG(statbuf.st_mode);
}

void find_by_tag(const char* tag)
{
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

void show_todos()
{
    find_by_tag("#todo");
}

void show_links()
{
    find_by_tag("#link");
}

void print_notes_from_book(const char *book_name)
{
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

json_t* books_to_json(Book* books, int count)
{
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

int handle_http_request(int client_socket, const char* request)
{
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

int run_http_server()
{
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