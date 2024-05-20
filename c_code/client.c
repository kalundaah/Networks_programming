// C
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// POSIX operating system API - provides the read, write, fork, exit, exec functions
#include <unistd.h>

// Types
#include <sys/types.h> // UNIX data types
#include <sys/socket.h> // Socket related data types
#include <netinet/in.h> // Internet address family structures and constants
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>


#define BUFFER_SIZE 1024

void checkMemoryAllocation(void *ptr);

int main(int argc, char const *argv[]) {
    // Get Client Host and Port
    if(argc != 3) {
        fprintf(stderr, "(Failed) usage: %s <host> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *SERVER_HOSTNAME;
    SERVER_HOSTNAME = strdup(argv[1]);
    int SERVER_PORT = atoi(argv[2]);

    // Socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0) {
        perror("Creation of client socket file descriptor failed");
        exit(EXIT_FAILURE);
    }

    
    // Getting the server IP address via DNS
    // If hostname is IP address, then IP address will be returned
    struct hostent *server_host;
    server_host = gethostbyname(SERVER_HOSTNAME);
    if(server_host == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", SERVER_HOSTNAME);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    bzero((char *)&server_address, sizeof(server_address)); // Clearing server address
    server_address.sin_family = AF_INET;
    bcopy(
        (char*)server_host->h_addr_list[0],
        (char*)&server_address.sin_addr.s_addr,
        server_host->h_length
    );
    server_address.sin_port = htons(SERVER_PORT);
    
    // Connect
    if(connect(
        client_fd,
        (struct sockaddr*)&server_address,
        sizeof(server_address)
    ) < 0) {
        perror("Connection of client socket file descriptor failed");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE] = { 0 };

    int write_read_return;

    while (1) {        
        // Print menu on client side  
        printf("\n\n====\n");      
        printf("Menu\n");
        printf("====\n");
        printf("1. Display Catalog\n");
        printf("2. Search Book\n");
        printf("3. Order Book\n");
        printf("4. Pay For Book\n");
        printf("\n");

        char option[4];
        char params[128];

        while(1) {
            printf("Choose Option: ");
            bzero(option, sizeof(option));
            fgets(option, sizeof(option), stdin); // get input
            printf("option: %s", option);
            
            int isValidNumber = 1;
            
            // Checking if number
            for(int i = 0; i < strlen(option) - 1; i++) {
                if(!isdigit(option[i])) {
                    isValidNumber = 0;
                }    
            }

            if(!isValidNumber) {
                printf("\nERROR: Please put a number.\n");
                continue;
            }

            // Checking if valid option
            if(atoi(option) > 4) {
                printf("\nERROR: Choose number in the list.\n");
                continue;
            }

            break;
        }
        
        switch (atoi(option))
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
                snprintf(params, sizeof(params),"%s%s%s", M, X, Z);
                break;
            case 2:
                char string[128];
                 printf("\nSearch Book Parameters\n");
                printf("-----------------------------");
                printf("\nstring: ");
                fgets(string, sizeof(string), stdin);

                // Concatenating into one large string
                snprintf(params, sizeof(params),"%s", string);
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
                snprintf(params, sizeof(params),"%s%s%s", x, y, n);
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

                snprintf(params, sizeof(params),"%s%s", orderno, Amount);
                break;
            default:
                break;
        }

         // clear buffer
        bzero(buffer, BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "%s%s", option, params);

        // send to server
        if (send(client_fd, buffer, BUFFER_SIZE, 0) < 0) {
            perror("ERROR writing to socket");
            exit(EXIT_FAILURE);
        }
        // clear buffer
        bzero(buffer, BUFFER_SIZE);
        // receive from server
        if (read(client_fd, buffer, BUFFER_SIZE) < 0) {
            perror("ERROR writing to socket");
            exit(EXIT_FAILURE);
        }
        
        printf("+++++++++\n");
        printf("+RESULTS+\n");
        printf("+++++++++\n");
        printf("\n%s", buffer);
    }
    
    close(client_fd);
    return 0;
}


void checkMemoryAllocation(void *ptr) {
    if (ptr == NULL) {
        perror("EEROR: Memory allocation failed");
        exit(EXIT_FAILURE);
    }
}