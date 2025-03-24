#ifndef FS_H_
#define FS_H_

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
#include "settings.h"


int create_note(const char* bookname, const char* notename)
{
	char* note_path[1024];
	snprintf(note_path, sizeof(note_path), "%s/%s/%s.bdsb", default_books_path, bookname, notename);

	char* book_path[1024];
	snprintf(book_path, sizeof(book_path), "%s/%s", default_books_path, bookname);

	struct stat book_info;
	if (stat(book_path, &book_info) != 0 || !(book_info.st_mode & S_IFDIR))
	{
		printf("Book directory does not exists. Please run 'bsdbook init default'.\n");
		//return Ok { Err::FOLDER_DOES_NOT_EXISTS };
		return -1;
	}

	if (access(note_path, F_OK) == 0)
	{
		printf("Note already exists!\n");
		//return Ok { Err::FILE_EXISTS };
		return -1;
	}

	FILE* fp = fopen(note_path, "a");
	if (fp == NULL)
	{
		printf("Error creating Note: %s\n", strerror(errno));
		//return Ok { Err::FILE_CREATING_ERROR }
		return -1;
	}
	fclose(fp);
	printf("Note has been created!\n");
	//return Ok { Err::OK };
	return 0;
}

int create_book(const char* bookname)
{
	char book_path[1024];
	snprintf(book_path, sizeof(book_path), "%s/%s", default_books_path, bookname);

	if (mkdir(book_path, 0755) == 0)
	{
		printf("Book has been created!\n");
		//return Ok { Err::BOOK_HAS_BEEN_CREATED };
		return 0;
	}
	printf("Failed to create book directory: %s\n", strerror(errno));
	//return Ok { Err::BOOK_CREATING_ERROR };
	return -1;
}

int install_bsdbook()
{
	struct stat info;
	if (stat(default_books_path, &info) != 0) {
		printf("Directory does not exists. Creating...\n");
		if (mkdir(default_books_path, 0755) == 0) {
			printf("Directory created!\n");
			//return Ok { Err::OK };
			return 0;
		}
		printf("Failed to create directory: %s\n", strerror(errno));
		//return Ok { Err::ERROR };
		return -1;
	} else if (info.st_mode & S_IFDIR) {
		printf("Directory already exists.\n");
		//return Ok { Err::DIR_ALREADY_EXISTS }
		return -1;
	}
	printf("Path exists but is not a directory.\n");
	//return Ok { Err::EXISTS_BUT_NOT_DIR };
	return -1;
}
/* DEPRECATED:
void print_notes_from_book(const char *book_name) {
    char book_path[1024];
    snprintf(book_path, sizeof(book_path), "books/%s", book_name);

    DIR *notes_dir = opendir(book_path);
    if (!notes_dir) {
        perror("Unable to open book directory");
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
}
*/

void get_books() {
	DIR* dir;
	struct dirent* entry;
	struct stat statbuf;

	dir = opendir(default_books_path);
	if (!dir) {
		perror("opendir");
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, ".") == 0) {
			continue;
		}
		char fullpath[1024];
		snprintf(fullpath, sizeof(fullpath), "%s/%s", default_books_path, entry->d_name);

		if (stat(fullpath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
			printf("%s\n", entry->d_name);
		} 
	}
	closedir(dir);
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

#endif // FS_H_
