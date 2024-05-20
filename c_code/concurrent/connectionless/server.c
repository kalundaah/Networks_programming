// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define BUFFER_SIZE 1024

#ifndef SO_REUSEPORT
#define SO_REUSEPORT 15  // Define SO_REUSEPORT with a default value
#endif

// Global variables
char **books = NULL;
size_t numOfBooks = 0;

// Helper Functions
void checkMemoryAllocation(void *ptr);
void putBooksInDataStructures();

// Server Interface
char *SearchBook(char *string);
char *DisplayCatalog(int M, int X, int Z);
char *OrderBook(char *x, char *y, int n);

void *handle_request(void *arg);

typedef struct {
    int sock_fd;
    struct sockaddr_in client_address;
    socklen_t client_address_len;
    char buffer[BUFFER_SIZE];
} request_info;

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
    // SO_REUSEADDR allows reusing the same address and port immediately after the socket is closed
    // SO_REUSEPORT allows multiple sockets to bind to the same port.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &socket_option, sizeof(socket_option)) < 0) {
        perror("Setting socket options failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT); // Host to network order

    socklen_t server_socket_address_length = sizeof(server_address);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_address, server_socket_address_length) < 0) {
        perror("Binding of server socket file descriptor failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started at %d\n", SERVER_PORT);

    // Before entering loop, create data structures
    putBooksInDataStructures();

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        char buffer[BUFFER_SIZE];

        bzero(buffer, BUFFER_SIZE);
        int recv_len = recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_address, &client_address_len);
        if (recv_len < 0) {
            perror("ERROR reading from socket");
            continue;
        }

        request_info *info = malloc(sizeof(request_info));
        checkMemoryAllocation(info);

        info->sock_fd = server_fd;
        info->client_address = client_address;
        info->client_address_len = client_address_len;
        strncpy(info->buffer, buffer, BUFFER_SIZE);

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_request, info) != 0) {
            perror("Failed to create thread");
            free(info);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}

void *handle_request(void *arg) {
    request_info *info = (request_info *)arg;
    char response[BUFFER_SIZE];

    printf("Received message: %s\n", info->buffer);

    int option = info->buffer[0] - '0'; // Assuming the first character is the option number
    char *params = &info->buffer[1];    // Parameters start after the option number

    bzero(response, BUFFER_SIZE);

    switch (option) {
    case 1:
        {
            int params1[3] = {0};
            sscanf(params, "%d %d %d", &params1[0], &params1[1], &params1[2]);
            char *catalog = DisplayCatalog(params1[0], params1[1], params1[2]);
            snprintf(response, BUFFER_SIZE, "%s", catalog);
            free(catalog);
        }
        break;
    case 2:
        {
            char *result = SearchBook(params);
            snprintf(response, BUFFER_SIZE, "%s", result);
            free(result);
        }
        break;
    case 3:
        {
            char params3[3][64];
            sscanf(params, "%s %s %d", params3[0], params3[1], &params3[2]);
            char *order = OrderBook(params3[0], params3[1], atoi(params3[2]));
            snprintf(response, BUFFER_SIZE, "%s", order);
            free(order);
        }
        break;
    default:
        snprintf(response, BUFFER_SIZE, "Invalid option");
        break;
    }

    if (sendto(info->sock_fd, response, strlen(response), 0, (struct sockaddr *)&info->client_address, info->client_address_len) < 0) {
        perror("ERROR writing to socket");
    }

    free(info);
    pthread_exit(NULL);
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
            books = (char **)realloc(books, (numOfBooks + 1) * sizeof(char *));
            checkMemoryAllocation(books);
            bookIndex = numOfBooks - 1;
            books[bookIndex] = (char *)malloc((strlen(book) + 1) * sizeof(char));
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

    int upperLimit = (X < 1) ? 1 : X;
    int lowerLimit = (Z > numOfBooks) ? numOfBooks : Z + 1;

    int catalogLength = 0;
    catalogLength += strlen(books[0]) + 1;
    for (int i = upperLimit; i < lowerLimit; i++) {
        catalogLength += strlen(books[i]) + 1;
    }

    char *catalog = (char *)malloc((catalogLength + 1) * sizeof(char));
    checkMemoryAllocation(catalog);
    catalog[0] = '\0';

    strcat(catalog, books[0]);
    for (int i = upperLimit; i < M + 1; i++) {
        strcat(catalog, books[i]);
    }

    printf("\nINFO: DisplayCatalog done executing...\n");
    return catalog;
}

char *SearchBook(char *string) {
    printf("\nINFO: SearchBook started executing...\n");
    int columnNumber = 1;
    int bookLineNumber = -1;
    char *result = NULL;

    for (int i = 1; i < numOfBooks; i++) {
        char *line = strdup(books[i]);
        char *token = strtok(line, "\t");
        while (token != NULL) {
            if (columnNumber == 2 || columnNumber == 4) {
                if (strcmp(string, token) == 0) {
                    bookLineNumber = i;
                    break;
                }
            }
            token = strtok(NULL, "\t");
            columnNumber++;
        }
        free(line);
        if (bookLineNumber != -1) {
            break;
        }
        columnNumber = 1;
    }

    if (bookLineNumber == -1) {
        result = (char *)malloc((strlen("No book found") + 1) * sizeof(char));
        checkMemoryAllocation(result);
        strcpy(result, "No book found");
    } else {
        size_t headerLength = strlen(books[0]);
        size_t bookLength = strlen(books[bookLineNumber]);
        result = (char *)malloc((headerLength + bookLength + 2) * sizeof(char));
        checkMemoryAllocation(result);
        strcpy(result, books[0]);
        strcat(result, books[bookLineNumber]);
    }

    printf("\nINFO: SearchBook done executing...\n");
    return result;
}

char *OrderBook(char *x, char *y, int n) {
    printf("\nINFO: OrderBook started executing...\n");
    char *search_book_response = SearchBook(x);

    if (strcmp(search_book_response, "No book found") == 0) {
        free(search_book_response);
        search_book_response = SearchBook(y);
        if (strcmp(search_book_response, "No book found") == 0) {
            return search_book_response;
        }
    }

    int columnNumber = 1;
    int price;
    char *line = strdup(search_book_response);
    char *book = strtok(line, "\n");
    book = strtok(NULL, "\n");
    char *book_field = strtok(book, "\t");

    while (book_field != NULL) {
        if (columnNumber == 7) {
            price = atoi(book_field);
            break;
        }
        book_field = strtok(NULL, "\t");
        columnNumber++;
    }
    free(line);

    FILE *fptr;
    int lastOrderNumber, newOrderNumber, lastTotalPrice;
    fptr = fopen("created_orders.txt", "a+");

    if (fptr == NULL) {
        perror("ERROR: Error when opening created_orders.txt");
        exit(EXIT_FAILURE);
    }

    if (fscanf(fptr, "%d\t%d", &lastOrderNumber, &lastTotalPrice) != 2) {
        lastOrderNumber = 0;
    } else {
        fseek(fptr, 0, SEEK_END);
    }

    newOrderNumber = lastOrderNumber + 1;
    lastTotalPrice = price * n;
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

    int responseLength = strlen(search_book_response) + strlen(unit_price_string) + strlen(quantity_string) +
                         strlen(total_price_string) + strlen(order_number_as_string) + 5;
    char *response = (char *)malloc(responseLength * sizeof(char));
    checkMemoryAllocation(response);
    response[0] = '\0';

    strcat(response, search_book_response);
    strcat(response, order_number_as_string);
    strcat(response, unit_price_string);
    strcat(response, quantity_string);
    strcat(response, total_price_string);

    free(search_book_response);
    printf("\nINFO: OrderBook done executing...\n");
    return response;
}

void checkMemoryAllocation(void *ptr) {
    if (ptr == NULL) {
        perror("ERROR: Memory allocation failed");
        exit(EXIT_FAILURE);
    }
}

