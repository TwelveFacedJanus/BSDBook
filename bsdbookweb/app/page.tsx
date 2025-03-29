"use client";

import { useState, useEffect, useRef } from 'react';
import { MagnifyingGlassIcon, MoonIcon, SunIcon, BookOpenIcon, TrashIcon, PlusIcon } from '@heroicons/react/24/outline';
import { PencilIcon } from '@heroicons/react/24/outline';
import ReactMarkdown from 'react-markdown';
import remarkGfm from 'remark-gfm';

// Generate UUID (with fallback)
function generateUUID() {
  if (typeof crypto !== 'undefined' && crypto.randomUUID) {
    return crypto.randomUUID();
  }
  return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
    const r = Math.random() * 16 | 0;
    const v = c === 'x' ? r : (r & 0x3 | 0x8);
    return v.toString(16);
  });
}

// Date formatting utility
const formatDate = (date: Date | string | undefined): string => {
  if (!date) return '';
  const dateObj = date instanceof Date ? date : new Date(date);
  return isNaN(dateObj.getTime()) ? '' : dateObj.toLocaleDateString();
};

// Types
interface Note {
  id: string;
  title: string;
  content: string;
  createdAt: Date;
  updatedAt: Date;
  tags: string[];
  bookId?: string;
}

interface Book {
  id: string;
  name: string;
  notes: string[];
  createdAt: Date;
}

type NotePreview = Pick<Note, 'id' | 'title' | 'updatedAt' | 'tags' | 'bookId'>;

// Storage Utilities
const NOTES_KEY = "obsidian-notes";
const BOOKS_KEY = "obsidian-books";

const getNotes = (): Note[] => {
  if (typeof window === "undefined") return [];
  const notes = localStorage.getItem(NOTES_KEY);
  if (!notes) return [];
  
  return JSON.parse(notes).map((note: any) => ({
    ...note,
    createdAt: new Date(note.createdAt),
    updatedAt: new Date(note.updatedAt)
  }));
};

const saveNotes = (notes: Note[]) => {
  if (typeof window === "undefined") return;
  localStorage.setItem(NOTES_KEY, JSON.stringify(notes));
};

const getBooks = (): Book[] => {
  if (typeof window === "undefined") return [];
  const books = localStorage.getItem(BOOKS_KEY);
  return books ? JSON.parse(books).map((book: any) => ({
    ...book,
    createdAt: new Date(book.createdAt),
  })) : [];
};

const saveBooks = (books: Book[]) => {
  if (typeof window === "undefined") return;
  localStorage.setItem(BOOKS_KEY, JSON.stringify(books));
};

const getNotePreviews = (): NotePreview[] => {
  const notes = getNotes();
  return notes.map(({ id, title, updatedAt, tags, bookId }) => ({
    id,
    title,
    updatedAt: new Date(updatedAt),
    tags,
    bookId,
  }));
};

const getNoteById = (id: string): Note | undefined => {
  const notes = getNotes();
  const note = notes.find((note) => note.id === id);
  return note ? {
    ...note,
    createdAt: new Date(note.createdAt),
    updatedAt: new Date(note.updatedAt)
  } : undefined;
};

const createNote = (bookId?: string): Note => {
  const now = new Date();
  const newNote: Note = {
    id: generateUUID(),
    title: "Untitled Note",
    content: "",
    createdAt: now,
    updatedAt: now,
    tags: [],
    bookId,
  };

  const notes = getNotes();
  notes.push(newNote);
  saveNotes(notes);

  if (bookId) {
    const books = getBooks();
    const bookIndex = books.findIndex(book => book.id === bookId);
    if (bookIndex !== -1) {
      books[bookIndex].notes.push(newNote.id);
      saveBooks(books);
    }
  }

  return newNote;
};

const updateNote = (id: string, updates: Partial<Note>) => {
  const notes = getNotes();
  const noteIndex = notes.findIndex((note) => note.id === id);

  if (noteIndex !== -1) {
    notes[noteIndex] = {
      ...notes[noteIndex],
      ...updates,
      updatedAt: new Date(),
    };
    saveNotes(notes);
    return notes[noteIndex];
  }

  return null;
};

const deleteNote = (id: string) => {
  const notes = getNotes();
  const noteToDelete = notes.find(note => note.id === id);
  const updatedNotes = notes.filter((note) => note.id !== id);
  saveNotes(updatedNotes);

  if (noteToDelete?.bookId) {
    const books = getBooks();
    const bookIndex = books.findIndex(book => book.id === noteToDelete.bookId);
    if (bookIndex !== -1) {
      books[bookIndex].notes = books[bookIndex].notes.filter(noteId => noteId !== id);
      saveBooks(books);
    }
  }

  return updatedNotes;
};

const createBook = (name: string): Book => {
  const newBook: Book = {
    id: generateUUID(),
    name: name || "Untitled Book",
    notes: [],
    createdAt: new Date(),
  };

  const books = getBooks();
  books.push(newBook);
  saveBooks(books);
  return newBook;
};

const deleteBook = (id: string) => {
  const books = getBooks();
  const bookToDelete = books.find(book => book.id === id);
  
  if (!bookToDelete) return;

  // Remove book references from notes
  const notes = getNotes();
  notes.forEach(note => {
    if (note.bookId === id) {
      note.bookId = undefined;
    }
  });
  saveNotes(notes);

  // Remove the book
  const updatedBooks = books.filter(book => book.id !== id);
  saveBooks(updatedBooks);

  return updatedBooks;
};

const moveNoteToBook = (noteId: string, targetBookId: string | null) => {
  const notes = getNotes();
  const noteIndex = notes.findIndex(note => note.id === noteId);
  
  if (noteIndex === -1) return { notes: getNotes(), books: getBooks() };

  const originalBookId = notes[noteIndex].bookId;
  
  // Remove from original book if it was in one
  if (originalBookId) {
    const books = getBooks();
    const originalBookIndex = books.findIndex(book => book.id === originalBookId);
    if (originalBookIndex !== -1) {
      books[originalBookIndex].notes = books[originalBookIndex].notes.filter(id => id !== noteId);
      saveBooks(books);
    }
  }

  // Add to new book if target is not null (null means "no book")
  if (targetBookId) {
    const books = getBooks();
    const targetBookIndex = books.findIndex(book => book.id === targetBookId);
    if (targetBookIndex !== -1 && !books[targetBookIndex].notes.includes(noteId)) {
      books[targetBookIndex].notes.push(noteId);
      saveBooks(books);
    }
  }

  // Update note's book reference
  notes[noteIndex].bookId = targetBookId || undefined;
  saveNotes(notes);

  return { notes: getNotes(), books: getBooks() };
};

// Components
function ThemeToggle() {
  const [darkMode, setDarkMode] = useState(false);

  useEffect(() => {
    const savedMode = localStorage.getItem("darkMode");
    const systemPrefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
    const initialMode = savedMode ? savedMode === 'true' : systemPrefersDark;
    
    setDarkMode(initialMode);
    document.documentElement.classList.toggle("dark", initialMode);
  }, []);

  const toggleTheme = () => {
    const newMode = !darkMode;
    setDarkMode(newMode);
    localStorage.setItem("darkMode", String(newMode));
    document.documentElement.classList.toggle("dark", newMode);
  };

  return (
    <button
      onClick={toggleTheme}
      className="p-2 rounded-full hover:bg-gray-700 transition-colors"
      aria-label="Toggle dark mode"
    >
      {darkMode ? (
        <SunIcon className="h-5 w-5 text-yellow-300" />
      ) : (
        <MoonIcon className="h-5 w-5 text-gray-400" />
      )}
    </button>
  );
}

function SearchBar() {
  return (
    <div className="mb-4 relative">
      <MagnifyingGlassIcon className="absolute left-3 top-1/2 -translate-y-1/2 h-4 w-4 text-gray-500" />
      <input
        type="text"
        placeholder="Search notes..."
        className="w-full pl-10 pr-4 py-2 bg-gray-800 rounded text-sm focus:outline-none focus:ring-1 focus:ring-purple-500"
      />
    </div>
  );
}

function BookList({
  books,
  selectedBookId,
  selectedNoteId,
  onSelectBook,
  onSelectNote,
  onCreateNote,
  onDeleteBook,
  onDeleteNote,
  onDrop,
  onDragOver,
  onDragStart,
}: {
  books: Book[];
  selectedBookId: string | null;
  selectedNoteId: string | null;
  onSelectBook: (id: string) => void;
  onSelectNote: (id: string) => void;
  onCreateNote: (bookId: string) => void;
  onDeleteBook: (id: string) => void;
  onDeleteNote: (id: string) => void;
  onDrop: (e: React.DragEvent, bookId: string | null) => void;
  onDragOver: (e: React.DragEvent) => void;
  onDragStart: (e: React.DragEvent, noteId: string) => void;
}) {
  const [contextMenu, setContextMenu] = useState<{
    visible: boolean;
    x: number;
    y: number;
    type: 'book' | 'note';
    id: string;
  } | null>(null);
  const [expandedBooks, setExpandedBooks] = useState<Set<string>>(new Set());

  const toggleBookExpansion = (bookId: string) => {
    setExpandedBooks(prev => {
      const newSet = new Set(prev);
      if (newSet.has(bookId)) {
        newSet.delete(bookId);
      } else {
        newSet.add(bookId);
      }
      return newSet;
    });
  };

  const handleContextMenu = (e: React.MouseEvent, type: 'book' | 'note', id: string) => {
    e.preventDefault();
    e.stopPropagation();
    setContextMenu({
      visible: true,
      x: e.clientX,
      y: e.clientY,
      type,
      id,
    });
  };

  const closeContextMenu = () => {
    setContextMenu(null);
  };

  useEffect(() => {
    const handleClickOutside = () => closeContextMenu();
    window.addEventListener("click", handleClickOutside);
    return () => window.removeEventListener("click", handleClickOutside);
  }, []);

  const handleDeleteBook = () => {
    if (contextMenu?.type === 'book' && confirm("Are you sure you want to delete this book? The notes will remain but will be moved out of this book.")) {
      onDeleteBook(contextMenu.id);
      closeContextMenu();
    }
  };

  const handleDeleteNote = () => {
    if (contextMenu?.type === 'note' && confirm("Are you sure you want to delete this note?")) {
      onDeleteNote(contextMenu.id);
      closeContextMenu();
    }
  };

  const handleRenameBook = () => {
    if (contextMenu?.type === 'book') {
      const book = books.find(b => b.id === contextMenu.id);
      const newName = prompt("Enter new book name:", book?.name || "");
      if (newName && newName !== book?.name) {
        const updatedBooks = books.map(b => 
          b.id === contextMenu.id ? {...b, name: newName} : b
        );
        saveBooks(updatedBooks);
        onSelectBook(contextMenu.id);
      }
      closeContextMenu();
    }
  };

  const handleRenameNote = () => {
    if (contextMenu?.type === 'note') {
      const note = getNoteById(contextMenu.id);
      const newTitle = prompt("Enter new note title:", note?.title || "");
      if (newTitle && newTitle !== note?.title) {
        updateNote(contextMenu.id, { title: newTitle });
        onSelectNote(contextMenu.id);
      }
      closeContextMenu();
    }
  };

  // Get all notes grouped by book
  const allNotes = getNotes();
  const notesByBook: Record<string, NotePreview[]> = {};
  books.forEach(book => {
    notesByBook[book.id] = book.notes
      .map(noteId => allNotes.find(n => n.id === noteId))
      .filter(Boolean) as NotePreview[];
  });

  return (
    <div className="mb-4">
      <h3 className="text-xs font-semibold text-gray-500 uppercase tracking-wider mb-2">Books</h3>
      <ul className="space-y-1">
        {(books.length !== 1) && (
          <li
            onDrop={(e) => onDrop(e, null)}
            onDragOver={onDragOver}
            className={`p-1 rounded ${selectedBookId === null ? 'bg-gray-700/30' : ''}`}
          >
            <div className="flex items-center text-sm text-gray-300">
              <span className="ml-6">No Book</span>
            </div>
          </li>
        )}
        {books.map((book) => (
          <li 
            key={book.id}
            data-is-book="true"
            onDrop={(e) => onDrop(e, book.id)}
            onDragOver={onDragOver}
            className="rounded"
          >
            <div 
              className="group flex items-center justify-between p-1 hover:bg-gray-800/50"
              onContextMenu={(e) => handleContextMenu(e, 'book', book.id)}
            >
              <div className="flex items-center flex-1">
                <button
                  onClick={() => toggleBookExpansion(book.id)}
                  className="mr-1 text-gray-500 hover:text-gray-300 w-4 h-4 flex items-center justify-center"
                >
                  {expandedBooks.has(book.id) ? '▼' : '▶'}
                </button>
                <button
                  onClick={() => onSelectBook(book.id)}
                  className="flex items-center text-sm text-gray-300 hover:text-purple-300 transition-colors flex-1 text-left"
                >
                  <BookOpenIcon className="h-4 w-4 mr-2" />
                  <span className="truncate">{book.name}</span>
                </button>
              </div>
              <button
                onClick={(e) => {
                  e.stopPropagation();
                  onCreateNote(book.id);
                }}
                className="opacity-0 group-hover:opacity-100 text-xs text-gray-500 hover:text-purple-300 transition-opacity ml-2"
              >
                + Add Note
              </button>
            </div>
            
            {/* Notes under this book */}
            {expandedBooks.has(book.id) && (
              <ul className="ml-6 mt-1 space-y-1">
                {notesByBook[book.id]?.map((note) => (
                  <li key={note.id}>
                    <button
                      draggable
                      onDragStart={(e) => onDragStart(e, note.id)}
                      onClick={() => onSelectNote(note.id)}
                      onContextMenu={(e) => handleContextMenu(e, 'note', note.id)}
                      className={`w-full text-left px-3 py-1 rounded text-sm flex justify-between items-center transition-colors ${
                        selectedNoteId === note.id
                          ? "bg-purple-900/50 text-purple-300"
                          : "hover:bg-gray-800/50 text-gray-300"
                      }`}
                    >
                      <span className="truncate">{note.title}</span>
                      <span className="text-xs text-gray-500 ml-2">
                        {formatDate(note.updatedAt)}
                      </span>
                    </button>
                  </li>
                ))}
              </ul>
            )}
          </li>
        ))}
      </ul>

      {/* Context Menu */}
      {contextMenu && (
        <div
          className="fixed bg-gray-800 border border-gray-700 rounded shadow-lg py-1 z-50"
          style={{
            left: `${contextMenu.x}px`,
            top: `${contextMenu.y}px`,
          }}
          onClick={(e) => e.stopPropagation()}
        >
          {contextMenu.type === 'book' ? (
            <>
              <button
                onClick={() => {
                  onCreateNote(contextMenu.id);
                  closeContextMenu();
                }}
                className="w-full text-left px-4 py-2 text-sm text-green-400 hover:bg-gray-700 flex items-center"
              >
                <PlusIcon className="h-4 w-4 mr-2" />
                Add Note
              </button>
              <button
                onClick={handleRenameBook}
                className="w-full text-left px-4 py-2 text-sm text-blue-400 hover:bg-gray-700 flex items-center"
              >
                <PencilIcon className="h-4 w-4 mr-2" />
                Rename Book
              </button>
              <button
                onClick={handleDeleteBook}
                className="w-full text-left px-4 py-2 text-sm text-red-400 hover:bg-gray-700 flex items-center"
              >
                <TrashIcon className="h-4 w-4 mr-2" />
                Delete Book
              </button>
            </>
          ) : (
            <>
              <button
                onClick={handleRenameNote}
                className="w-full text-left px-4 py-2 text-sm text-blue-400 hover:bg-gray-700 flex items-center"
              >
                <PencilIcon className="h-4 w-4 mr-2" />
                Rename Note
              </button>
              <button
                onClick={handleDeleteNote}
                className="w-full text-left px-4 py-2 text-sm text-red-400 hover:bg-gray-700 flex items-center"
              >
                <TrashIcon className="h-4 w-4 mr-2" />
                Delete Note
              </button>
            </>
          )}
        </div>
      )}
    </div>
  );
}

function Editor({
  note,
  onUpdate,
}: {
  note: Note | null;
  onUpdate: (updatedNote: Note) => void;
}) {
  const [content, setContent] = useState("");
  const [title, setTitle] = useState("");

  useEffect(() => {
    if (note) {
      setTitle(note.title);
      setContent(note.content);
    } else {
      setTitle("");
      setContent("");
    }
  }, [note]);

  const handleTitleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const newTitle = e.target.value;
    setTitle(newTitle);
    if (note) {
      const updatedNote = updateNote(note.id, { title: newTitle });
      if (updatedNote) onUpdate(updatedNote);
    }
  };

  const handleContentChange = (e: React.ChangeEvent<HTMLTextAreaElement>) => {
    const newContent = e.target.value;
    setContent(newContent);
    if (note) {
      const updatedNote = updateNote(note.id, { content: newContent });
      if (updatedNote) onUpdate(updatedNote);
    }
  };

  if (!note) {
    return (
      <div className="flex h-full items-center justify-center text-gray-500">
        <p>Select a note or create a new one</p>
      </div>
    );
  }

  return (
    <div className="h-full flex flex-col">
      <div className="p-4 border-b border-gray-700">
        <input
          type="text"
          value={title}
          onChange={handleTitleChange}
          className="w-full text-2xl font-bold bg-transparent focus:outline-none focus:border-purple-500"
          placeholder="Note title"
        />
      </div>

      <div className="flex-1 flex overflow-hidden">
        {/* Editor Panel */}
        <div className="flex-1 border-r border-gray-700">
          <textarea
            value={content}
            onChange={handleContentChange}
            className="w-full h-full p-4 bg-transparent resize-none focus:outline-none"
            placeholder="Start writing your note here (supports Markdown)..."
          />
        </div>

        {/* Preview Panel */}
        <div className="flex-1 p-4 overflow-y-auto prose dark:prose-invert max-w-none">
          <ReactMarkdown remarkPlugins={[remarkGfm]}>{content}</ReactMarkdown>
        </div>
      </div>

      <div className="p-4 border-t border-gray-700 text-sm text-gray-500">
        <p>
          Last updated: {formatDate(note.updatedAt)}
          {note.tags.length > 0 && (
            <>
              {" • "}
              Tags: {note.tags.join(", ")}
            </>
          )}
        </p>
      </div>
    </div>
  );
}

// Main App Component
export default function Home() {
  const [notes, setNotes] = useState<NotePreview[]>([]);
  const [books, setBooks] = useState<Book[]>([]);
  const [selectedNoteId, setSelectedNoteId] = useState<string | null>(null);
  const [selectedBookId, setSelectedBookId] = useState<string | null>(null);
  const [selectedNote, setSelectedNote] = useState<Note | null>(null);
  const [isClient, setIsClient] = useState(false);
  const [sidebarContextMenu, setSidebarContextMenu] = useState<{
    visible: boolean;
    x: number;
    y: number;
  } | null>(null);
  const [draggedNoteId, setDraggedNoteId] = useState<string | null>(null);

  useEffect(() => {
    setIsClient(true);
    const loadedNotes = getNotePreviews();
    const loadedBooks = getBooks();
    setNotes(loadedNotes);
    setBooks(loadedBooks);
    
    if (loadedNotes.length > 0 && !selectedNoteId) {
      setSelectedNoteId(loadedNotes[0].id);
    }
  }, []);

  useEffect(() => {
    if (selectedNoteId && isClient) {
      const note = getNoteById(selectedNoteId);
      setSelectedNote(note || null);
    } else {
      setSelectedNote(null);
    }
  }, [selectedNoteId, isClient, notes]);

  const handleSidebarContextMenu = (e: React.MouseEvent) => {
    const target = e.target as HTMLElement;
    const isBookElement = target.closest('[data-is-book]');
    
    if (!isBookElement) {
      e.preventDefault();
      setSidebarContextMenu({
        visible: true,
        x: e.clientX,
        y: e.clientY,
      });
    }
  };

  const closeSidebarContextMenu = () => {
    setSidebarContextMenu(null);
  };

  const handleCreateNote = (bookId?: string) => {
    const newNote = createNote(bookId);
    const updatedNotes = getNotePreviews();
    const updatedBooks = getBooks();
    setNotes(updatedNotes);
    setBooks(updatedBooks);
    setSelectedNoteId(newNote.id);
  };

  const handleCreateBook = () => {
    const bookName = prompt("Enter book name:");
    if (bookName) {
      createBook(bookName);
      const updatedBooks = getBooks();
      setBooks(updatedBooks);
      closeSidebarContextMenu();
    }
  };

  const handleSelectNote = (id: string) => {
    setSelectedNoteId(id);
  };

  const handleSelectBook = (id: string | null) => {
    setSelectedBookId(id);
  };

  const handleUpdateNote = (updatedNote: Note) => {
    const updatedNotes = getNotePreviews();
    setNotes(updatedNotes);
    if (selectedNoteId === updatedNote.id) {
      setSelectedNote(updatedNote);
    }
  };

  const handleDeleteNote = (id: string) => {
    deleteNote(id);
    const updatedNotes = getNotePreviews();
    const updatedBooks = getBooks();
    setNotes(updatedNotes);
    setBooks(updatedBooks);
    
    if (selectedNoteId === id) {
      setSelectedNoteId(updatedNotes.length > 0 ? updatedNotes[0].id : null);
    }
  };

  const handleDeleteBook = (id: string) => {
    const updatedBooks = deleteBook(id);
    if (updatedBooks) {
      setBooks(updatedBooks);
      
      if (selectedBookId === id) {
        setSelectedBookId(null);
      }
      
      const updatedNotes = getNotePreviews();
      setNotes(updatedNotes);
    }
  };

  const handleDragStart = (e: React.DragEvent, noteId: string) => {
    setDraggedNoteId(noteId);
    e.dataTransfer.setData("text/plain", noteId);
    e.dataTransfer.effectAllowed = "move";
  };

  const handleDragOver = (e: React.DragEvent) => {
    e.preventDefault();
    e.dataTransfer.dropEffect = "move";
  };

  const handleDrop = (e: React.DragEvent, targetBookId: string | null) => {
    e.preventDefault();
    if (!draggedNoteId) return;
  
    const { notes: updatedNotes, books: updatedBooks } = moveNoteToBook(draggedNoteId, targetBookId);
    setNotes(updatedNotes);
    setBooks(updatedBooks);
    
    if (selectedNoteId === draggedNoteId) {
      const updatedNote = getNoteById(draggedNoteId);
      if (updatedNote) {
        setSelectedNote(updatedNote);
      }
    }
  
    setDraggedNoteId(null);
  };

  useEffect(() => {
    const handleClickOutside = () => closeSidebarContextMenu();
    window.addEventListener("click", handleClickOutside);
    return () => window.removeEventListener("click", handleClickOutside);
  }, []);

  return (
    <div className="min-h-screen bg-gray-100 text-gray-900 dark:bg-gray-900 dark:text-gray-100">
      <div className="flex">
        {/* Sidebar */}
        <div 
          className="fixed left-0 top-0 h-screen w-64 border-r border-gray-300 dark:border-gray-700 bg-gray-50/50 dark:bg-gray-900/50 p-4 flex flex-col"
          onContextMenu={handleSidebarContextMenu}
          style={{ zIndex: 40 }}
        >
          <div className="mb-6 flex items-center justify-between">
            <h1 className="text-xl font-semibold text-purple-600 dark:text-purple-400">BSDBook</h1>
            <ThemeToggle />
          </div>

          <SearchBar />

          <button
            onClick={() => handleCreateNote()}
            className="mb-4 w-full rounded bg-purple-600 px-4 py-2 text-sm font-medium text-white hover:bg-purple-700 transition-colors"
          >
            New Note
          </button>

          <BookList
            books={books}
            selectedBookId={selectedBookId}
            selectedNoteId={selectedNoteId}
            onSelectBook={handleSelectBook}
            onSelectNote={handleSelectNote}
            onCreateNote={handleCreateNote}
            onDeleteBook={handleDeleteBook}
            onDeleteNote={handleDeleteNote}
            onDrop={handleDrop}
            onDragOver={handleDragOver}
            onDragStart={handleDragStart}
          />

          <div className="mt-auto pt-4 text-xs text-gray-500 dark:text-gray-400">
            <p>Notes: {notes.length} • Books: {books.length}</p>
          </div>

          {/* Sidebar Context Menu */}
          {sidebarContextMenu && (
            <div
              className="fixed bg-gray-800 border border-gray-700 rounded shadow-lg py-1 z-50"
              style={{
                left: `${sidebarContextMenu.x}px`,
                top: `${sidebarContextMenu.y}px`,
              }}
              onClick={(e) => e.stopPropagation()}
            >
              <button
                onClick={handleCreateBook}
                className="w-full text-left px-4 py-2 text-sm text-green-400 hover:bg-gray-700 flex items-center"
              >
                <BookOpenIcon className="h-4 w-4 mr-2" />
                Create Book
              </button>
            </div>
          )}
        </div>

        {/* Main Content */}
        <div className="ml-64 flex-1 min-h-screen">
          <Editor note={selectedNote} onUpdate={handleUpdateNote} />
        </div>
      </div>
    </div>
  );
}