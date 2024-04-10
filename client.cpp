#include <cstring> 
#include <iostream> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <string>

using namespace std;

int main(int argc, char* argv[]) 
{ 
    // creating socket 
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); 

    // specifying address 
    sockaddr_in serverAddress; 
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_port = htons(8080); 
    serverAddress.sin_addr.s_addr = INADDR_ANY; 

    // sending connection request 
    connect(clientSocket, (struct sockaddr*)&serverAddress,sizeof(serverAddress)); 

    // Communication loop
    while (true) {
        cout << "Enter a number (or type 'stop' to quit): ";
        string input;
        cin >> input;

        // Send input number to server
        send(clientSocket, input.c_str(), strlen(input.c_str()), 0); 
        
        if(input == "2" || input == "3" || input == "4"){
            cout << "Enter the book you wish to find" << endl;
            string query;
            cin >> query;
            const char* responseMsg = query.c_str();
            send(clientSocket, responseMsg, strlen(responseMsg), 0); 
            // break;
        }
        cout << "Number To Client: " << input << endl;

        char buffer[1024] = { 0 }; 
        recv(clientSocket, buffer, sizeof(buffer), 0);

        cout << "Server response:\n " << buffer << endl;
    }

    // closing socket 
    close(clientSocket); 

    return 0; 
}
