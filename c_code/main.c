#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Global variables
char** books = NULL;
size_t numOfBooks = 0;

// Helper Functions
void checkMemoryAllocation(void *ptr);
void putBooksInDataStructures();

// Server Interface
char* SearchBook(char *string);
char* DisplayCatalog(int M, int X, int Z);

int main() {
   putBooksInDataStructures();

   char* catalog = DisplayCatalog(1,2,30);
   printf("%s\n",catalog);
   char* bookFound = SearchBook("Pelican Brief, The");
   printf("%s\n", bookFound);
   return 0;
}

void putBooksInDataStructures() {
   // Open file
   FILE* fptr;
   fptr = fopen("books2.txt", "r");
   
   if(fptr == NULL) {
      perror("\nERROR: The file has not opened");
      exit(EXIT_FAILURE);
   } else {
      printf("\nINFO: File opened successfully\n");
      char *book = NULL;
      size_t lineSize = 0;
      int bookIndex = 0;

      while (getline(&book, &lineSize, fptr) != -1) {
         numOfBooks++;
         books = realloc(books, numOfBooks * sizeof(char*));
         checkMemoryAllocation(books);
         bookIndex = numOfBooks - 1;
         books[bookIndex] = malloc((strlen(book) + 1) * sizeof(char));
         checkMemoryAllocation(books[bookIndex]);
         strcpy(books[bookIndex], book);
      }
      
      printf("\nINFO: Data successfully read from file\n");
      free(book);
      fclose(fptr);
      printf("\nINFO: The file is now closed.\n");
   }
}

char* DisplayCatalog(int M, int X, int Z) {
   printf("\nINFO: DisplayCatalog started executing...\n");
   int catalogLength = 0;
   int upperLimit = X;
   int lowerLimit;
   // If Z is bigger than numOfBooks
   if(Z > numOfBooks) {
      lowerLimit = numOfBooks;
   } else {
      lowerLimit = Z + 1;
   }

   // Catalog Length
   catalogLength += strlen(books[0]);
   for(int i = upperLimit; i < lowerLimit; i++) {
      catalogLength += strlen(books[i]);
   }

   // Allocate memory to catalog
   char* catalog = malloc((catalogLength * sizeof(char)) + 1);
   checkMemoryAllocation(catalog);
   
   // Add header
   int currentPos = 0;
   strcpy(catalog + currentPos, books[0]);
   currentPos += strlen(books[0]);

   // Add books
   for(int i = upperLimit; i < lowerLimit; i++) {
      strcpy(catalog + currentPos, books[i]);
      currentPos += strlen(books[i]);
   }

   printf("\nINFO: DisplayCatalog done executing...\n");
   return catalog;
}

char* SearchBook(char* string) {
   printf("\nINFO: SearchBook started executing...\n");
   int columnNumber = 1; // 1-indexed
   int bookLineNumber = -1;
   char *result;
   
   for(int i = 1; i < numOfBooks; i++) {
      // Create duplicate to maintain original
      // strtok modifies original string
      char* line = strdup(books[i]);
      char* token = strtok(line, "\t");
      while(token != NULL) {
         if(columnNumber == 2 || columnNumber == 4) {
            if(strcmp(string, token) == 0) {
               bookLineNumber = i;
               break;
            }
         }
         token = strtok(NULL, "\t");
         columnNumber++;
      }
      
      // Reset Column Number
      columnNumber = 1;
   }

   if(bookLineNumber == -1) {
      result = "No book found";
   } else {
      result = books[bookLineNumber];

      size_t headerLength = strlen(books[0]);
      size_t bookLength = strlen(books[bookLineNumber]);

      result = malloc((headerLength + bookLength + 1) * sizeof(char));
      strcpy(result, books[0]);
      strcat(result, books[bookLineNumber]);
   }
   printf("\nINFO: SearchBook done executing...\n");
   return result;
   
}

char OrderBook() {
   
}

void checkMemoryAllocation(void *ptr) {
    if (ptr == NULL) {
        perror("EEROR: Memory allocation failed");
        exit(EXIT_FAILURE);
    }
}

