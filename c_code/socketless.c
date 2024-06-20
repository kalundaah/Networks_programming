#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// POSIX operating system API - provides the read, write, fork, exit, exec functions
#include <unistd.h>

// Types
#include <sys/types.h>  // UNIX data types
#include <sys/socket.h> // Socket related data types
#include <netinet/in.h> // Internet address family structures and constants
#include <netdb.h>
#include <arpa/inet.h>

#include <time.h>

#define BUFFER_SIZE 1024

#ifndef SO_REUSEPORT
#define SO_REUSEPORT 15 // Define SO_REUSEPORT with a default value
#endif

// Global variables
char **books = NULL;
char *response = NULL;
size_t numOfBooks = 0;

// Helper Functions
void checkMemoryAllocation(void *ptr);
void putBooksInDataStructures();

// Putting books in data structures
void putBooksInDataStructures()
{
    // Open file
    FILE *fptr;
    fptr = fopen("books2.txt", "r");

    // Error handling if file did not open
    if (fptr == NULL)
    {
        perror("\nERROR: The file has not opened");
        exit(EXIT_FAILURE);
    }
    else
    {
        // File opened successfully
        printf("\nINFO: File opened successfully\n");
        // Temporary book storage for one book
        char *book = NULL;
        size_t lineSize = 0;
        int bookIndex = 0;

        // Read the file line by line looking at each book and storing it in the books array
        while (getline(&book, &lineSize, fptr) != -1)
        {
            // Increase total number of books
            numOfBooks++;
            // Reallocate memory for books
            books = (char **)realloc(books, (numOfBooks + 1) * sizeof(char *));
            // Error checking
            checkMemoryAllocation(books);
            // Set book index
            bookIndex = numOfBooks - 1;
            // Allocate memory for new book coming in
            books[bookIndex] = (char *)malloc((strlen(book) + 1) * sizeof(char));
            // Error checking
            checkMemoryAllocation(books[bookIndex]);
            // Copy book to books array
            strcpy(books[bookIndex], book);
        }

        printf("\nINFO: Data successfully read from file\n");
        // Free pointers
        free(book);
        fclose(fptr);
        printf("\nINFO: The file is now closed.\n");
    }
}

char *DisplayCatalog(int M, int X, int Z)
{
    printf("\nINFO: DisplayCatalog started executing...\n");

    int upperLimit;
    int lowerLimit;

    if (X < 1)
    {
        upperLimit = 1;
    }
    else
    {
        upperLimit = X;
    }

    // If Z is less than numOfBooks
    if (Z > numOfBooks)
    {
        lowerLimit = numOfBooks;
    }
    else
    {
        lowerLimit = Z + 1;
    }

    int catalogLength = 0;
    // Adding header length
    catalogLength += strlen(books[0]) + 1;
    for (int i = upperLimit; i < lowerLimit; i++)
    {
        catalogLength += strlen(books[i]) + 1;
    }

    // Allocate memory to catalog
    char *catalog = (char *)malloc(((catalogLength + 1) * sizeof(char)));
    checkMemoryAllocation(catalog);
    catalog[0] = '\0';

    // Add header
    strcat(catalog, books[0]);

    // Add books
    for (int i = upperLimit; i < M + 1; i++)
    {
        strcat(catalog, books[i]);
    }

    printf("\nINFO: DisplayCatalog done executing...\n");
    return catalog;
}

char *SearchBook(char *string)
{
    printf("\nINFO: SearchBook started executing...\n");
    int columnNumber = 1; // 1-indexed
    int bookLineNumber = -1;
    char *result = NULL;

    for (int i = 1; i < numOfBooks; i++)
    {
        // Create duplicate to maintain original
        // strtok modifies original string
        char *line = strdup(books[i]);
        char *token = strtok(line, "\t");
        while (token != NULL)
        {
            if (columnNumber == 2 || columnNumber == 4)
            {
                if (strcmp(string, token) == 0)
                {
                    bookLineNumber = i;
                }
            }

            if (bookLineNumber != -1)
            {
                break;
            }
            token = strtok(NULL, "\t");
            columnNumber++;
        }

        // Reset Column Number
        columnNumber = 1;
    }

    if (bookLineNumber == -1)
    {
        result = (char *)malloc((strlen("No book found") + 1) * sizeof(char));
        checkMemoryAllocation(result);
        result[0] = '\0';
        strcat(result, "No book found");
    }
    else
    {
        size_t headerLength = strlen(books[0]);
        size_t bookLength = strlen(books[bookLineNumber]);

        result = (char *)malloc((headerLength + bookLength + 2) * sizeof(char));
        checkMemoryAllocation(result);
        result[0] = '\0';
        strcat(result, books[0]);
        strcat(result, books[bookLineNumber]);
    }
    printf("\nINFO: SearchBook done executing...\n");
    return result;
}

char *OrderBook(char *x, char *y, int n)
{
    printf("\nINFO: OrderBook started executing...\n");
    char *search_book_response = SearchBook(x);

    // if book does not exist,
    if (strcmp(search_book_response, "No book found") == 0)
    {
        free(search_book_response);
        search_book_response = SearchBook(y);
        if (strcmp(search_book_response, "No book found") == 0)
        {
            return search_book_response;
        }
    }

    int columnNumber = 1; // 1-indexed
    int price;

    // get book
    char *line = strdup(search_book_response);

    // Extract header
    char *book = strtok(line, "\n");

    // Get book details
    book = strtok(NULL, "\n");
    char *book_field = strtok(book, "\t");

    // Look for price
    while (book_field != NULL)
    {
        if (columnNumber == 7)
        {
            price = atoi(book_field);
            break;
        }

        book_field = strtok(NULL, "\t");
        columnNumber++;
    }

    // Get created orders and create new order
    FILE *fptr;
    int lastOrderNumber = 0, newOrderNumber, lastTotalPrice;

    fptr = fopen("created_orders.txt", "a+");

    if (fptr == NULL)
    {
        perror("ERROR: Error when opening created_orders.txt");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fptr) != NULL)
    {
        sscanf(buffer, "%d\t%d", &lastOrderNumber, &lastTotalPrice);
    }

    newOrderNumber = lastOrderNumber + 1;
    lastTotalPrice = price * n;

    // Add to file
    fprintf(fptr, "%d\t%d\n", newOrderNumber, lastTotalPrice);
    fclose(fptr);

    char order_number_as_string[64];
    sprintf(order_number_as_string, "\n\nOrder Number: %d\n", newOrderNumber);

    char unit_price_string[64];
    sprintf(unit_price_string, "Unit Price: %d\n", price);

    char quantity_string[32];
    sprintf(quantity_string, "Quantity: %d\n", n);

    char total_price_string[64];
    sprintf(total_price_string, "Total Price: %d\n", price * n);

    char *response = NULL;
    int responseLength = strlen(search_book_response) + 1 +
                         strlen(unit_price_string) + 1 +
                         strlen(quantity_string) + 1 +
                         strlen(total_price_string) + 1 +
                         strlen(order_number_as_string) + 1;

    response = (char *)malloc(responseLength * sizeof(char));
    checkMemoryAllocation(response);

    response[0] = '\0';

    strcat(response, search_book_response);
    strcat(response, order_number_as_string);
    strcat(response, unit_price_string);
    strcat(response, quantity_string);
    strcat(response, total_price_string);

    printf("\nINFO: OrderBook done executing...\n");
    return response;
}

char *PayForBook(int orderno, int Amount)
{
    printf("\nINFO: PayForBook started executing...\n");
    FILE *createdPtr, *paidPtr, *tempPtr;

    printf("Order number and amount: %d, %d", orderno, Amount);

    char *line = NULL;
    char *paidLine = (char *)malloc(128 * sizeof(char));
    char *response = (char *)malloc(128 * sizeof(char));
    ;
    size_t lineLength = 0;
    ssize_t read = 0;
    int wasOrderCreated = 0;

    createdPtr = fopen("created_orders.txt", "r");
    if (createdPtr == NULL)
    {
        perror("ERROR: Error when opening created_orders.txt");
        exit(EXIT_FAILURE);
    }

    paidPtr = fopen("paid_orders.txt", "a");
    if (paidPtr == NULL)
    {
        perror("ERROR: Error when opening paid_orders.txt");
        exit(EXIT_FAILURE);
    }

    tempPtr = fopen("temp.txt", "w");
    if (paidPtr == NULL)
    {
        perror("ERROR: Error when opening temp.txt");
        exit(EXIT_FAILURE);
    }

    int currentOrderNumber;
    int currentTotalPrice;

    int foundAnyOrder = 0;
    while (getline(&line, &lineLength, createdPtr) != -1)
    {
        foundAnyOrder = 1;
        printf("\nLine found: %s\n", line);
        if (sscanf(line, "%d\t%d", &currentOrderNumber, &currentTotalPrice) == 2)
        {
            printf("\nCurrent Order Number: %d \n", currentOrderNumber);
            if (currentOrderNumber == orderno)
            {
                printf("\nCurrent Order Number: %d \n", currentOrderNumber);
                wasOrderCreated = 1;
                strcpy(paidLine, line);
                printf("\nOrder %d was created and found\n", orderno);
                continue;
            }
            fprintf(tempPtr, "%s", line);
        }
    }

    if (foundAnyOrder == 0)
    {
        sprintf(response, "\nNo orders have been created.\n");
        printf("\nINFO: PayForBook started executing...\n");
        remove("temp.txt");
        return response;
    }

    fclose(createdPtr);
    fclose(tempPtr);

    if (wasOrderCreated)
    {
        // Check if amount sent is enough,
        if (Amount >= currentTotalPrice)
        {
            // Replace the original file with the temporary file
            if (remove("created_orders.txt") != 0)
            {
                perror("ERROR: Error deleting original file");
                exit(EXIT_FAILURE);
            }
            else if (rename("temp.txt", "created_orders.txt") != 0)
            {
                perror("ERROR: Error renaming temporary file");
                exit(EXIT_FAILURE);
            }

            // Append to file
            fprintf(paidPtr, "%s", paidLine);
            fclose(paidPtr);
            int balance = Amount - currentTotalPrice;
            sprintf(response, "\nOrder Number: %d.\nPayment Status: SUCCESSFUL.\nAmount Paid: %d\nBalance: %d\n", orderno, Amount, balance);
            printf("\nINFO: PayForBook started executing...\n");
            return response;
        }
        else
        {
            int insufficient = currentTotalPrice - Amount;
            sprintf(response, "\nOrder Number: %d.\nPayment Status: FAILED(Insufficient Amount).\nAmount Paid: %d\nMoney To Add: %d\n", orderno, Amount, insufficient);
            printf("\nINFO: PayForBook started executing...\n");
            return response;
        }
    }
    else
    {
        // No matching order number found, so remove the temporary file
        remove("temp.txt");

        sprintf(response, "Order Number: %d.\nPayment Status: FAILED(Invalid Order Number).\n", orderno);
        printf("\nINFO: PayForBook started executing...\n");
        return response;
    }
}
int main()
{

    int option;
    char *response;

    char params[128];

    scanf("%d", &option);

    switch (option)
    {
    case 1:
        // get query parameters
        char M[20];
        char X[20];
        char Z[20];

        printf("\nDisplay Catalog Parameters\n");
        printf("-----------------------------");
        printf("\nM: ");
        fgets(M, sizeof(M), stdin);
        printf("X: ");
        fgets(X, sizeof(X), stdin);
        printf("Z: ");
        fgets(Z, sizeof(Z), stdin);

        // Concatenating into one large string
        snprintf(params, sizeof(params), "%s%s%s", M, X, Z);
        break;
    case 2:
        char string[128];
        printf("\nSearch Book Parameters\n");
        printf("-----------------------------");
        printf("\nstring: ");
        fgets(string, sizeof(string), stdin);

        // Concatenating into one large string
        snprintf(params, sizeof(params), "%s", string);
        break;
    case 3:
        // get query parameters
        char x[20];
        char y[20];
        char n[20];

        printf("\nOrder Book Catalog Parameters\n");
        printf("-----------------------------");
        printf("\nx: ");
        fgets(x, sizeof(x), stdin);
        printf("y: ");
        fgets(y, sizeof(y), stdin);
        printf("n: ");
        fgets(n, sizeof(n), stdin);

        // Concatenating into one large string
        snprintf(params, sizeof(params), "%s%s%s", x, y, n);
        break;
    case 4:
        char orderno[64];
        char Amount[64];

        printf("\nOrder Book Catalog Parameters\n");
        printf("-----------------------------");
        printf("\norderno: ");
        fgets(orderno, sizeof(orderno), stdin);
        printf("Amount: ");
        fgets(Amount, sizeof(Amount), stdin);

        snprintf(params, sizeof(params), "%s%s", orderno, Amount);
        break;
    default:
        break;
    }

    return 0;
}

void checkMemoryAllocation(void *ptr)
{
    if (ptr == NULL)
    {
        perror("\nERROR: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}
