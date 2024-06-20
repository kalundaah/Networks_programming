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

// Server Interface
char *SearchBook(char *string);
char *DisplayCatalog(int M, int X, int Z);
char *OrderBook(char *x, char *y, int n);
char *PayForBook(int orderno, int Amount);

int main(int argc, char const *argv[])
{
    // Get Server Port
    if (argc != 2)
    {
        fprintf(stderr, "(Failed) usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int SERVER_PORT = atoi(argv[1]);

    // Socket
    int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0)
    {
        perror("Creation of server socket file descriptor failed");
        exit(EXIT_FAILURE);
    }

    int socket_option = 1; // It is on to enable us to set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &socket_option, sizeof(socket_option)) < 0)
    {
        perror("Setting socket options failed");
        exit(EXIT_FAILURE);
    };

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT); // Host to network order

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Binding of server socket file descriptor failed");
        exit(EXIT_FAILURE);
    };

    printf("Server started at %d\n", SERVER_PORT);

    // Before entering loop, create data structures
    putBooksInDataStructures();

    // Read from client
    ssize_t bytes_read_from_client;
    char buffer[BUFFER_SIZE] = {0};

    // Server Logic
    while (1)
    {
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);

        // Receive
        bytes_read_from_client = recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_address, &client_address_length);
        if (bytes_read_from_client < 0)
        {
            perror("ERROR: Error reading from client");
            continue;
        }

        // Handle each request in a new process
        pid_t pid = fork();
        if (pid == 0)
        { // Child process
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
            int client_port = ntohs(client_address.sin_port);
            printf("Client connected from %s:%d\n", client_ip, client_port);

            char *token = strtok(buffer, "\n");
            int option = atoi(token);
            switch (option)
            {
            case 1:
            {
                int params1[3] = {0};
                int paramIndex1 = 0;
                while (token != NULL && paramIndex1 < 3)
                {
                    token = strtok(NULL, "\n");
                    params1[paramIndex1] = atoi(token);
                    paramIndex1++;
                }
                response = DisplayCatalog(params1[0], params1[1], params1[2]);
            }
            break;
            case 2:
                token = strtok(NULL, "\n");
                response = SearchBook(token);
                break;
            case 3:
            {
                char params3[3][64];
                int paramIndex3 = 0;
                while (token != NULL && paramIndex3 < 3)
                {
                    token = strtok(NULL, "\n");
                    strcpy(params3[paramIndex3], token);
                    paramIndex3++;
                }
                response = OrderBook(params3[0], params3[1], atoi(params3[2]));
            }
            break;
            case 4:
            {
                char params4[2][64];
                int paramIndex4 = 0;
                while (token != NULL && paramIndex4 < 2)
                {
                    token = strtok(NULL, "\n");
                    strcpy(params4[paramIndex4], token);
                    paramIndex4++;
                }
                response = PayForBook(atoi(params4[0]), atoi(params4[1]));
            }
            break;
            default:
                response = "\nInvalid Option. Try again\n";
                break;exit(0);
            }

            if (response != NULL)
            {
                sendto(server_fd, response, strlen(response), 0, (struct sockaddr *)&client_address, client_address_length);
                if (response != "\nInvalid Option. Try again\n")
                {
                    free(response);
                    response = NULL;
                }
            }
            exit(0);
        }
    }

    return 0;
}

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

void checkMemoryAllocation(void *ptr)
{
    if (ptr == NULL)
    {
        perror("ERROR: Memory allocation failed");
        exit(EXIT_FAILURE);
    }
}
