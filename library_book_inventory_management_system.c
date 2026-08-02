#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TITLE_LEN    50
#define AUTHOR_LEN   50
#define CATEGORY_LEN 30
#define DATA_FILE    "books.txt"

//  Book structure 
typedef struct {
    int id;
    char title[TITLE_LEN];
    char author[AUTHOR_LEN];
    char category[CATEGORY_LEN];
    int copies;
} Book;

//  Global state 
Book *books = NULL; // dynamic array of books
int bookCount = 0; // number of books currently stored

// Utility functions

// Clears leftover input in the buffer after scanf
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

// Reads a line of text safely into dest (removes trailing newline)
void readLine(char *dest, int size) {
    if (fgets(dest, size, stdin) != NULL) {
        size_t len = strlen(dest);
        if (len > 0 && dest[len - 1] == '\n')
            dest[len - 1] = '\0';
    }
}

// Returns 1 if the given ID already exists, 0 otherwise
int idExists(int id) {
    int i;
    for (i = 0; i < bookCount; i++) {
        if (books[i].id == id)
            return 1;
    }
    return 0;
}

// Finds the array index of a book by ID, or -1 if not found
int findIndexById(int id) {
    int i;
    for (i = 0; i < bookCount; i++) {
        if (books[i].id == id)
            return i;
    }
    return -1;
}

// File handling

// Loads books from the text data file into memory at program start.
// Each line is stored as: id|title|author|category|copies
void loadBooks(void) {
    FILE *fp = fopen(DATA_FILE, "r");
    if (fp == NULL) {
        return;
    }

    char line[200];
    while (fgets(line, sizeof(line), fp) != NULL) {
        Book temp;
        char *token;

        // Remove trailing newline, if any
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (strlen(line) == 0)
            continue; // skip blank lines

        token = strtok(line, "|");
        if (token == NULL) continue;
        temp.id = atoi(token);

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        strncpy(temp.title, token, TITLE_LEN - 1);
        temp.title[TITLE_LEN - 1] = '\0';

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        strncpy(temp.author, token, AUTHOR_LEN - 1);
        temp.author[AUTHOR_LEN - 1] = '\0';

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        strncpy(temp.category, token, CATEGORY_LEN - 1);
        temp.category[CATEGORY_LEN - 1] = '\0';

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        temp.copies = atoi(token);

        Book *newArr = realloc(books, (bookCount + 1) * sizeof(Book));
        if (newArr == NULL) {
            printf("Error: Memory allocation failed while loading file.\n");
            fclose(fp);
            return;
        }
        books = newArr;
        books[bookCount] = temp;
        bookCount++;
    }

    fclose(fp);
}

// Saves all books currently in memory to the text data file.
// Each line is written as: id|title|author|category|copies
void saveBooks(void) {
    FILE *fp = fopen(DATA_FILE, "w");
    if (fp == NULL) {
        printf("Error: Could not open file for saving.\n");
        return;
    }

    int i;
    for (i = 0; i < bookCount; i++) {
        fprintf(fp, "%d|%s|%s|%s|%d\n",
                books[i].id, books[i].title, books[i].author,
                books[i].category, books[i].copies);
    }

    fclose(fp);
}

// Core operations

// Adds a new book to the inventory
void addBook(void) {
    Book newBook;

    printf("Enter Book ID: ");
    if (scanf("%d", &newBook.id) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (idExists(newBook.id)) {
        printf("Error: Book ID %d already exists.\n", newBook.id);
        return;
    }

    printf("Enter Title: ");
    readLine(newBook.title, TITLE_LEN);

    printf("Enter Author: ");
    readLine(newBook.author, AUTHOR_LEN);

    printf("Enter Category: ");
    readLine(newBook.category, CATEGORY_LEN);

    printf("Enter Number of Copies: ");
    if (scanf("%d", &newBook.copies) != 1 || newBook.copies < 0) {
        printf("Invalid number of copies.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    // Grow the dynamic array by one slot
    Book *newArr = realloc(books, (bookCount + 1) * sizeof(Book));
    if (newArr == NULL) {
        printf("Error: Memory allocation failed.\n");
        return;
    }
    books = newArr;
    books[bookCount] = newBook;
    bookCount++;

    saveBooks();
    printf("Book added successfully.\n");
}

// Displays all books in a table format
void displayAllBooks(void) {
    if (bookCount == 0) {
        printf("No books in inventory.\n");
        return;
    }

    int i;
    printf("\n%-6s %-20s %-20s %-15s %-8s\n",
           "ID", "Title", "Author", "Category", "Copies");
    printf("\n");
    for (i = 0; i < bookCount; i++) {
        printf("%-6d %-20s %-20s %-15s %-8d\n",
               books[i].id, books[i].title, books[i].author,
               books[i].category, books[i].copies);
    }
}

// Updates an existing book's information
void updateBook(void) {
    int id, index;

    printf("Enter Book ID to update: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    index = findIndexById(id);
    if (index == -1) {
        printf("Error: Book ID %d not found.\n", id);
        return;
    }

    printf("Enter new Title: ");
    readLine(books[index].title, TITLE_LEN);

    printf("Enter new Author: ");
    readLine(books[index].author, AUTHOR_LEN);

    printf("Enter new Category: ");
    readLine(books[index].category, CATEGORY_LEN);

    printf("Enter new Number of Copies: ");
    if (scanf("%d", &books[index].copies) != 1 || books[index].copies < 0) {
        printf("Invalid number of copies.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    saveBooks();
    printf("Book updated successfully.\n");
}

// Deletes a book by ID
void deleteBook(void) {
    int id, index, i;

    printf("Enter Book ID to delete: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    index = findIndexById(id);
    if (index == -1) {
        printf("Error: Book ID %d not found.\n", id);
        return;
    }

    // Shift all later records one position back
    for (i = index; i < bookCount - 1; i++) {
        books[i] = books[i + 1];
    }
    bookCount--;

    if (bookCount == 0) {
        free(books);
        books = NULL;
    } else {
        Book *newArr = realloc(books, bookCount * sizeof(Book));
        if (newArr != NULL) {
            books = newArr;
        }
        // If realloc fails when shrinking, the old block is still usable, so we simply keep using 'books' as is.
    }

    saveBooks();
    printf("Book deleted successfully.\n");
}

// Search functions

// Manual (linear) search by Book ID
void searchById(void) {
    int id, index;

    printf("Enter Book ID to search: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    index = findIndexById(id);
    if (index == -1) {
        printf("Book with ID %d not found.\n", id);
        return;
    }

    printf("\nBook Found:\n");
    printf("ID: %d\nTitle: %s\nAuthor: %s\nCategory: %s\nCopies: %d\n", books[index].id, books[index].title, books[index].author, books[index].category, books[index].copies);
}

// Manual (linear) search by Title (case-sensitive substring match)
void searchByTitle(void) {
    char keyword[TITLE_LEN];
    int i, found = 0;

    printf("Enter Title to search: ");
    readLine(keyword, TITLE_LEN);

    for (i = 0; i < bookCount; i++) {
        if (strstr(books[i].title, keyword) != NULL) {
            if (!found) {
                printf("\nMatching Books:\n");
                found = 1;
            }
            printf("ID: %d | Title: %s | Author: %s | Category: %s | Copies: %d\n", books[i].id, books[i].title, books[i].author, books[i].category, books[i].copies);
        }
    }

    if (!found) {
        printf("No books found matching \"%s\".\n", keyword);
    }
}

// Sorting functions (using bubble sort)
void sortById(void) {
    int i, j;
    Book tempBook;

    for (i = 0; i < bookCount - 1; i++) {
        for (j = 0; j < bookCount - 1 - i; j++) {
            if (books[j].id > books[j + 1].id) {
                tempBook = books[j];
                books[j] = books[j + 1];
                books[j + 1] = tempBook;
            }
        }
    }
    printf("Books sorted by ID.\n");
}

void sortByTitle(void) {
    int i, j;
    Book tempBook;

    for (i = 0; i < bookCount - 1; i++) {
        for (j = 0; j < bookCount - 1 - i; j++) {
            if (strcmp(books[j].title, books[j + 1].title) > 0) {
                tempBook = books[j];
                books[j] = books[j + 1];
                books[j + 1] = tempBook;
            }
        }
    }
    printf("Books sorted by Title.\n");
}

void sortByCopies(void) {
    int i, j;
    Book tempBook;

    for (i = 0; i < bookCount - 1; i++) {
        for (j = 0; j < bookCount - 1 - i; j++) {
            if (books[j].copies > books[j + 1].copies) {
                tempBook = books[j];
                books[j] = books[j + 1];
                books[j + 1] = tempBook;
            }
        }
    }
    printf("Books sorted by Number of Copies.\n");
}

// Reports

void generateReport(void) {
    if (bookCount == 0) {
        printf("No books in inventory. No report to generate.\n");
        return;
    }

    int i, totalCopies = 0, maxIndex = 0;

    // Total copies and book with the highest number of copies
    for (i = 0; i < bookCount; i++) {
        totalCopies += books[i].copies;
        if (books[i].copies > books[maxIndex].copies) {
            maxIndex = i;
        }
    }

    printf("\n===== Inventory Report =====\n");
    printf("Total number of books: %d\n", bookCount);
    printf("Total copies available: %d\n", totalCopies);
    printf("Book with highest copies: %s (ID: %d, Copies: %d)\n", books[maxIndex].title, books[maxIndex].id, books[maxIndex].copies);

    // Count books per category using simple manual grouping
    char seenCategories[100][CATEGORY_LEN];
    int categoryCounts[100];
    int categoryTotal = 0;
    int j, k, matched;

    for (i = 0; i < bookCount; i++) {
        matched = 0;
        for (j = 0; j < categoryTotal; j++) {
            if (strcmp(seenCategories[j], books[i].category) == 0) {
                categoryCounts[j]++;
                matched = 1;
                break;
            }
        }
        if (!matched && categoryTotal < 100) {
            strcpy(seenCategories[categoryTotal], books[i].category);
            categoryCounts[categoryTotal] = 1;
            categoryTotal++;
        }
    }

    printf("\nBooks per Category:\n");
    for (k = 0; k < categoryTotal; k++) {
        printf("  %s: %d\n", seenCategories[k], categoryCounts[k]);
    }
    printf("=============================\n");
}

// Menu
void showMenu(void) {
    printf("\n==========================================\n");
    printf("   Library Book Inventory Management\n");
    printf("==========================================\n");
    printf(" 1. Add a book\n");
    printf(" 2. Display all books\n");
    printf(" 3. Update a book\n");
    printf(" 4. Delete a book\n");
    printf(" 5. Search by Book ID\n");
    printf(" 6. Search by Title\n");
    printf(" 7. Sort by Book ID\n");
    printf(" 8. Sort by Title\n");
    printf(" 9. Sort by Number of Copies\n");
    printf("10. Generate Inventory Report\n");
    printf("11. Exit\n");
    printf("=======================================\n");
    printf("Select an option [1-11]: ");
}

int main(void) {
    int choice;

    loadBooks();

    while (1) {
        showMenu();

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        if (choice == 1) {
            addBook();
        } else if (choice == 2) {
            displayAllBooks();
        } else if (choice == 3) {
            updateBook();
        } else if (choice == 4) {
            deleteBook();
        } else if (choice == 5) {
            searchById();
        } else if (choice == 6) {
            searchByTitle();
        } else if (choice == 7) {
            sortById();
            saveBooks();
        } else if (choice == 8) {
            sortByTitle();
            saveBooks();
        } else if (choice == 9) {
            sortByCopies();
            saveBooks();
        } else if (choice == 10) {
            generateReport();
        } else if (choice == 11) {
            printf("Saving data and exiting. Goodbye!\n");
            saveBooks();
            free(books);
            break;
        } else {
            printf("Invalid option. Please choose a number between 1 and 11.\n");
        }
    }

    return 0;
}

