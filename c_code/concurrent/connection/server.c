// C
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
#include <sys/select.h>

#define BUFFER_SIZE 1024

#ifndef SO_REUSEPORT
#define SO_REUSEPORT 15  // Define SO_REUSEPORT with a default value
#endif

// Global variables
char** books = NULL;
char* response = NULL;
size_t numOfBooks = 0;

// Helper Functions
void checkMemoryAllocation(void *ptr);
void putBooksInDataStructures();

// Server Interface
char* SearchBook(char *string);
char* DisplayCatalog(int M, int X, int Z);
char* OrderBook(char *x, char *y, int n);

int main(int argc, char const *argv[]) {
    // Get Server Port
    if (argc != 2) {
        fprintf(stderr, "(Failed) usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int SERVER_PORT = atoi(argv[1]);

    // Socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        perror("Creation of server socket file descriptor failed");
        exit(EXIT_FAILURE);
    }

    int socket_option = 1; // It is on to enable us to set socket options
    // SO_REUSEADDR allows reusing the same address and port immediately after the socket is closed
    // SO_REUSEPORT allows multiple sockets to bind to the same port. 
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &socket_option, sizeof(socket_option)) < 0) {
        perror("Creation of server socket file descriptor failed");
        exit(EXIT_FAILURE);
    };

    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT); // Host to network order

    socklen_t server_socket_address_length = sizeof(server_address);

    // Bind
    if(bind(server_fd, (struct sockaddr*)&server_address, server_socket_address_length) < 0){
        perror("Binding of server socket file descriptor failed");
        exit(EXIT_FAILURE);
    };

    // Listen
    int listen_queue_size = 5;
    if (listen(server_fd, listen_queue_size) < 0) {
        perror("Listening of server socket file descriptor failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started at %d\n", SERVER_PORT);

    // Before entering loop, create data structures
    putBooksInDataStructures();

    // For managing multiple clients
    fd_set master_set, read_set;
    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    int max_fd = server_fd;

    // Server Logic
    while (1) {
        read_set = master_set;
        if (select(max_fd + 1, &read_set, NULL, NULL, NULL) < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_set)) {
                if (i == server_fd) {
                    // New connection
                    struct sockaddr_in client_address;
                    socklen_t client_address_length = sizeof(client_address);
                    int client_fd = accept(server_fd, (struct sockaddr*)&client_address, &client_address_length);
                    if (client_fd < 0) {
                        perror("Accept error");
                        exit(EXIT_FAILURE);
                    }
                    FD_SET(client_fd, &master_set);
                    if (client_fd > max_fd) {
                        max_fd = client_fd;
                    }
                    printf("Client connected from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                } else {
                    // Handle data from a client
                    char buffer[BUFFER_SIZE] = {0};
                    ssize_t bytes_read_from_client = recv(i, buffer, BUFFER_SIZE, 0);
                    if (bytes_read_from_client <= 0) {
                        if (bytes_read_from_client == 0) {
                            printf("Client disconnected\n");
                        } else {
                            perror("Recv error");
                        }
                        close(i);
                        FD_CLR(i, &master_set);
                    } else {
                        char* token = strtok(buffer, "\n");
                        int option = atoi(token);
                        switch (option) {
                            case 1: {
                                int params[3] = {0};
                                int paramIndex = 0;
                                while (token != NULL && paramIndex < 3) {
                                    token = strtok(NULL, "\n");
                                    params[paramIndex] = atoi(token);
                                    paramIndex++;
                                }
                                response = DisplayCatalog(params[0], params[1], params[2]);
                                break;
                            }
                            case 2: {
                                token = strtok(NULL, "\n");
                                response = SearchBook(token);
                                break;
                            }
                            case 3: {
                                char params[3][64];
                                int paramIndex = 0;
                                while (token != NULL && paramIndex < 3) {
                                    token = strtok(NULL, "\n");
                                    strcpy(params[paramIndex], token);
                                    paramIndex++;
                                }
                                response = OrderBook(params[0], params[1], atoi(params[2]));
                                break;
                            }
                            default:
                                response = "\nInvalid Option. Try again\n";
                                break;
                        }
                        if (response != NULL) {
                            if (send(i, response, strlen(response), 0) < 0) {
                                perror("Send error");
                                exit(EXIT_FAILURE);
                            }
                            if (response != "\nInvalid Option. Try again\n") {
                                free(response);
                                response = NULL;
                            }
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
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
            books = (char**)realloc(books, (numOfBooks + 1) * sizeof(char*));
            checkMemoryAllocation(books);
            bookIndex = numOfBooks - 1;
            books[bookIndex] = (char*)malloc((strlen(book) + 1) * sizeof(char));
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
    
    int upperLimit;
    int lowerLimit;

    if(X < 1) {
        upperLimit = 1;
    } else {
        upperLimit = X;
    }
    
    // If Z is less than numOfBooks
    if(Z > numOfBooks) {
        lowerLimit = numOfBooks;
    } else {
        lowerLimit = Z + 1;
    }

    int catalogLength = 0;
    // Adding header length
    catalogLength += strlen(books[0]) + 1;
    for(int i = upperLimit; i < lowerLimit; i++) {
        catalogLength += strlen(books[i]) + 1;
    }

    // Allocate memory to catalog
    char* catalog = (char*)malloc(((catalogLength + 1) * sizeof(char)));
    checkMemoryAllocation(catalog);
    catalog[0] = '\0';

    // Add header
    strcat(catalog, books[0]);

    // Add books
    for(int i = upperLimit; i < M + 1; i++) {
        strcat(catalog, books[i]);
    }

    printf("\nINFO: DisplayCatalog done executing...\n");
    return catalog;
}

char* SearchBook(char* string) {
    printf("\nINFO: SearchBook started executing...\n");
    int columnNumber = 1; // 1-indexed
    int bookLineNumber = -1;
    char *result = NULL;

    for(int i = 1; i < numOfBooks; i++) {
        // Create duplicate to maintain original
        // strtok modifies original string
        char* line = strdup(books[i]);
        char* token = strtok(line, "\t");
        while(token != NULL) {
            if(columnNumber == 2 || columnNumber == 4) {
                if(strcmp(string, token) == 0) {
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
    }

    if(bookLineNumber == -1) {
        result =  (char*)malloc((strlen("No book found") + 1) * sizeof(char));
        checkMemoryAllocation(result);
        result[0] = '\0';
        strcat(result, "No book found");

    } else {
        size_t headerLength = strlen(books[0]);
        size_t bookLength = strlen(books[bookLineNumber]);

        result = (char*)malloc((headerLength + bookLength + 2) * sizeof(char));
        checkMemoryAllocation(result);
        result[0] = '\0';
        strcat(result, books[0]);
        strcat(result, books[bookLineNumber]);
    }
    printf("\nINFO: SearchBook done executing...\n");
    return result;

}

char* OrderBook(char* x, char* y, int n) {
    printf("\nINFO: OrderBook started executing...\n");
    char *search_book_response = SearchBook(x);
    
    // if book does not exist,
    if(strcmp(search_book_response, "No book found") == 0) {
        free(search_book_response);
        search_book_response = SearchBook(y);
        if(strcmp(search_book_response, "No book found") == 0) {
            return search_book_response;
        }
    }

    int columnNumber = 1; // 1-indexed
    int price;

    // get book
    char* line = strdup(search_book_response);
    char* book = strtok(line, "\n");
    book = strtok(NULL, "\n");
    char *book_field = strtok(book, "\t");

    // Look for price
    while(book_field != NULL) {
        if(columnNumber == 7) {
            price = atoi(book_field);
            break;
        }

        book_field = strtok(NULL, "\t");
        columnNumber++;
    }

    // Get created orders and create new order
    FILE *fptr;
    int lastOrderNumber, newOrderNumber, lastTotalPrice;

    fptr = fopen("created_orders.txt", "a+");
    
    if(fptr == NULL) {
        perror("ERROR: Error when opening created_orders.txt");
        exit(EXIT_FAILURE);
    }

    // If file empty, 0
    if(fscanf(fptr, "%d\t%d", &lastOrderNumber, &lastTotalPrice) != 2) {
        lastOrderNumber = 0;
    } else {
        fseek(fptr, 0, SEEK_END);
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

    char* response = NULL;
    int responseLength = strlen(search_book_response) + 1 + 
        strlen(unit_price_string) + 1 +
        strlen(quantity_string) + 1 +
        strlen(total_price_string) + 1 + 
        strlen(order_number_as_string) + 1;

    response = (char*)malloc(responseLength * sizeof(char));
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

void checkMemoryAllocation(void *ptr) {
    if (ptr == NULL) {
        perror("EEROR: Memory allocation failed");
        exit(EXIT_FAILURE);
    }
}
