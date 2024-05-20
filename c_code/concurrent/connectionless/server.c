#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// POSIX operating system API - provides the read, write, fork, exit, exec functions
#include <unistd.h>

// Types
#include <sys/types.h> // UNIX data types
#include <sys/socket.h> // Socket related data types
#include <netinet/in.h> // Internet address family structures and constants
#include <netdb.h>
#include <arpa/inet.h>

#include <time.h>

#define BUFFER_SIZE 1024

#ifndef SO_REUSEPORT
#define SO_REUSEPORT 15  // Define SO_REUSEPORT with a default value
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

int main(int argc, char const *argv[]) {
    // Get Server Port
    if (argc != 2) {
        fprintf(stderr, "(Failed) usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int SERVER_PORT = atoi(argv[1]);

    // Socket
    int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        perror("Creation of server socket file descriptor failed");
        exit(EXIT_FAILURE);
    }

    int socket_option = 1; // It is on to enable us to set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &socket_option, sizeof(socket_option)) < 0) {
        perror("Setting socket options failed");
        exit(EXIT_FAILURE);
    };

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT); // Host to network order

    // Bind
    if (bind(server_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
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
    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);

        // Receive
        bytes_read_from_client = recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_address, &client_address_length);
        if (bytes_read_from_client < 0) {
            perror("ERROR: Error reading from client");
            continue;
        }

        // Handle each request in a new process
        pid_t pid = fork();
        if (pid == 0) { // Child process
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
            int client_port = ntohs(client_address.sin_port);
            printf("Client connected from %s:%d\n", client_ip, client_port);

            char *token = strtok(buffer, "\n");
            int option = atoi(token);
            switch (option) {
                case 1:
                    {
                        int params1[3] = {0};
                        int paramIndex1 = 0;
                        while (token != NULL && paramIndex1 < 3) {
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
                        while (token != NULL && paramIndex3 < 3) {
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
                        while (token != NULL && paramIndex4 < 2) {
                            token = strtok(NULL, "\n");
                            strcpy(params4[paramIndex4], token);
                            paramIndex4++;
                        }
                        response = PayForBook(atoi(params4[0]), atoi(params4[1]));
                    }
                    break;
                default:
                    response = "\nInvalid Option. Try again\n";
                    break;
            }

            if (response != NULL) {
                sendto(server_fd, response, strlen(response), 0, (struct sockaddr *) &client_address, client_address_length);
                if (response != "\nInvalid Option. Try again\n") {
                    free(response);
                    response = NULL;
                }
            }
            exit(0);
        }
    }

    return 0;
}

void putBooksInDataStructures() {
    // Open file
    FILE *fptr;
    fptr = fopen("books2.txt", "r");

    if (fptr == NULL) {
        perror("\nERROR: The file has not opened");
        exit(EXIT_FAILURE);
    } else {
        printf("\nINFO: File opened successfully\n");
        char *book = NULL;
        size_t lineSize = 0;
        int bookIndex = 0;

        while (getline(&book, &lineSize, fptr) != -1) {
            numOfBooks++;
            books = (char **) realloc(books, (numOfBooks + 1) * sizeof(char *));
            checkMemoryAllocation(books);
            bookIndex = numOfBooks - 1;
            books[bookIndex] = (char *) malloc((strlen(book) + 1) * sizeof(char));
            checkMemoryAllocation(books[bookIndex]);
            strcpy(books[bookIndex], book);
        }

        printf("\nINFO: Data successfully read from file\n");
        free(book);
        fclose(fptr);
        printf("\nINFO: The file is now closed.\n");
    }
}

char *DisplayCatalog(int M, int X, int Z) {
    printf("\nINFO: DisplayCatalog started executing...\n");

    int upperLimit;
    int lowerLimit;

    if (X < 1) {
        upperLimit = 1;
    } else {
        upperLimit = X;
    }

    // If Z is less than numOfBooks
    if (Z > numOfBooks) {
        lowerLimit = numOfBooks;
    } else {
        lowerLimit = Z + 1;
    }

    int catalogLength = 0;
    // Adding header length
    catalogLength += strlen(books[0]) + 1;
    for (int i = upperLimit; i < lowerLimit; i++) {
        catalogLength += strlen(books[i]) + 1;
    }

    // Allocate memory to catalog
    char *catalog = (char *) malloc(((catalogLength + 1) * sizeof(char)));
    checkMemoryAllocation(catalog);
    catalog[0] = '\0';

    // Add header
    strcat(catalog, books[0]);

    // Add books
    for (int i = upperLimit; i < M + 1; i++) {
        strcat(catalog, books[i]);
    }

    printf("\nINFO: DisplayCatalog done executing...\n");
    return catalog;
}

char *SearchBook(char *string) {
    printf("\nINFO: SearchBook started executing...\n");
    int columnNumber = 1; // 1-indexed
    int bookLineNumber = -1;
    char *result = NULL;

    for (int i = 1; i < numOfBooks; i++) {
        // Create duplicate to maintain original
        // strtok modifies original string
        char *line = strdup(books[i]);
        char *token = strtok(line, "\t");
        while (token != NULL) {
            if (columnNumber == 2 || columnNumber == 4) {
                if (strcmp(string, token) == 0) {
                    bookLineNumber = i;
                }
            }

            if (bookLineNumber != -1) {
                break;
            }
            token = strtok(NULL, "\t");
            columnNumber++;
        }

        // Reset Column Number
        columnNumber = 1;

        // Free memory
        free(line);
        if (bookLineNumber != -1) {
            break;
        }
    }

    if (bookLineNumber != -1) {
        result = strdup(books[bookLineNumber]);
        printf("\nINFO: SearchBook done executing...\n");
        return result;
    } else {
        printf("\nINFO: SearchBook done executing...\n");
        return "\nBook does not exist in our Catalog. Try again\n";
    }
}

char *OrderBook(char *x, char *y, int n) {
    printf("\nINFO: OrderBook started executing...\n");

    char *order = (char *) malloc(((numOfBooks + 1) * sizeof(char)));
    checkMemoryAllocation(order);

    int orderNumber;
    orderNumber = rand();
    sprintf(order, "Order Number: %d\n", orderNumber);

    printf("\nINFO: OrderBook done executing...\n");
    return order;
}

char *PayForBook(int orderno, int Amount) {
    printf("\nINFO: PayForBook started executing...\n");

    char *result = (char *) malloc(((numOfBooks + 1) * sizeof(char)));
    checkMemoryAllocation(result);
    sprintf(result, "Order Number: %d\nAmount Paid: %d\n", orderno, Amount);

    printf("\nINFO: PayForBook done executing...\n");
    return result;
}

void checkMemoryAllocation(void *ptr) {
    if (ptr == NULL) {
        perror("\nERROR: Memory allocation failed");
        exit(EXIT_FAILURE);
    }
}
