# Changelog
All notable changes to this project will be documented in this file.

## v1.8.2
### Changed
 - Created new workflow for autorelease. It will work at release branch only.

## v1.8.3
### Changed
 - Created new workflow for autorelease. It will work at release branch only.

## v1.8.4
### Changed
 - Test release

## v1.8.5
### Changed
 - Okay, this is the last test update. In the next updates i will do something more accuracity.

## v1.8.6
### Changed
 - Created new functions `get_books_st` and `get_notes_st` for `bsdbook.h` file.
 - Removed `INSTALL_unix.md` file.
 - `Makefile` updates. Now we can clean data. Now we generate libraries into `lib` folder.
 - Created `server.c` that will be used for server. Now we can see books and notes using endpoints.
 - In the next update, I will create new Makefile instructions for server compiling and use valgrind for fixing some memory leaks.
 - `server.go` and `server.rs` removed.
### Next Release
 - Endpoint for editing note content.
 - Endpoint for editing note name.
 - Endpoint for edition book name.
 - Endpoint for moving note to another book.
 - Endpoint for creating new books or notes.
 - Endpoint for removing books and notes.
 - Some `Makefile` updates.
 - Valgrind using.
