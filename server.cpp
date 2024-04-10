#include <cstring> 
#include <iostream> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std; 

// Placeholders for processing
string line;
string cell;
vector<string> row;

// Storage
vector<vector<string>> books_vector;

string DisplayCatalog() {
    
    string catalog = "";
    for (const auto& book: books_vector) {
        for(const auto& book_field: book) {
            catalog += book_field;
            catalog += "\t\t";         
        }
        // To separate each book entry with a line break
        catalog += "\n";
    }
    return catalog;
}

vector<string> searchforItemVector(string query) {
    vector <string> found_row;
    
    bool found = false; // The boolean that completes the logic

    for (const auto& row : books_vector) {
        for (const auto& cell : row) {
            if (cell == query) {
                found = true;
                found_row = row;
                break; // Value found, no need to continue searching
            }
        }
        if (found) {
            break; // Value found in some row, no need to continue searching
        }
    }

    return found_row;
}

string searchforItem(string query){
    string book_found = "";
    vector<string> found_row = searchforItemVector(query);
    for(const auto& book_field: found_row) {
        book_found += book_field;
        book_found += "\t\t";
    }

    return book_found;

}

int Purchaseitem(string query){
    vector <string> item_vector = searchforItemVector(query);
    
    // grab price cell
    string priceholder = item_vector.at(item_vector.size() - 1);
    
    // Convert to integer
    int price = stoi(priceholder);
    return price;
}

void PayforItem(string query){
    int price = Purchaseitem(query);
    cout << "Paid " << price << " for the book\n" << endl;
}

int ReadFromFile() {
    // Open  file
    ifstream books("books2.txt");

    // If opening has an error
    if (!books) {
        cerr << "Error opening file books2.txt" << endl;
        return 1;
    }
    
    // Read file
    while(!books.eof()) {
        // Get A line
        getline(books, line);

        // Converting a string to a stream so that I can use the getline function to split it
        istringstream line_stream(line);

        // Get cells separated by tabs
        while(getline(line_stream, cell, '\t')) {
            row.push_back(cell);
        }

        // Add to list of books
        books_vector.push_back(row);
        
        // Clear temporary vector
        row.clear();
    }
    // Close file
    books.close();

    return 0;
}

string displayMenu() {

    string menu = "1. Display Catalog \n2. Search for Item\n3. Purchase Item\n4. Pay for Item";
    return menu;
    
}


int main() 
{
    cout << "WAITING";
    int read = ReadFromFile();
    if(read == 1){
        cout << "Book vector could not be created";
    }
    // creating socket 
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); 

    // specifying the address 
    sockaddr_in serverAddress; 
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_port = htons(8080); 
    serverAddress.sin_addr.s_addr = INADDR_ANY; 

    // binding socket. 
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)); 

    // listening to the assigned socket 
    listen(serverSocket, 5); 
    
    bool hold = true;

    // accepting connection request 
    int clientSocket = accept(serverSocket, nullptr, nullptr); 
    int price;
    while (hold == true) {

        char buffer[1024] = { 0 }; 
        recv(clientSocket, buffer, sizeof(buffer), 0); 

        int num = atoi(buffer); // Convert received message to integer
        
        cout << "Number From Client: \n" << num << endl;
        
        if(num == -1){
            hold = false;
            // Close client socket
            close(clientSocket);
            break;  
        }
        switch(num){
            case 0: //menu
            {
                cout << "Client asked for menu\n";
                string response = displayMenu();
                const char* responseMsg = response.c_str();
                // Send response back to client
                send(clientSocket, responseMsg, strlen(responseMsg), 0);
                break;
            }    
            case 1: // display catalogue
            {
                cout << "Client asked for catalogue\n";
                string response = DisplayCatalog();
                const char* responseMsg = response.c_str();
                // Send response back to client
                send(clientSocket, responseMsg, strlen(responseMsg), 0);
                break;
            }
            case 2: // search for item
            {
                cout << "Entered case 2\n" << endl;
                char buffer[1024] = { 0 }; 
                recv(clientSocket, buffer, sizeof(buffer), 0);
                cout << "Buffer in case 2: " << buffer << endl;
                // buffer becomes query
                // string query = str(buffer);
                string query(buffer);
                string response = searchforItem(query);
                const char* responseMsg = response.c_str();
                // Send response back to client
                send(clientSocket, responseMsg, strlen(responseMsg), 0);
                break;
            }
            case 3: // purchase item
            {
                char buffer[1024] = { 0 }; 
                recv(clientSocket, buffer, sizeof(buffer), 0);
                
                // buffer becomes query
                string query(buffer);
                int price = Purchaseitem(query);
                
                
                // Send response back to client
                send(clientSocket, to_string(price).c_str(), strlen(to_string(price).c_str()), 0); 
                break;
            }
            case 4: // payforitem
            {
                char buffer[1024] = { 0 }; 
                recv(clientSocket, buffer, sizeof(buffer), 0);
                
                // buffer becomes query
                string query(buffer);
                PayforItem(query);
                string response = "PAYMENT CONFIRMED";
                const char* responseMsg = response.c_str();
                // Send response back to client
                send(clientSocket, responseMsg, strlen(responseMsg), 0);
                break;
            }
            default:
            {
                cout << "Invalid response\n";
                string response = "Enter a valid response";
                const char* responseMsg = response.c_str();
                // Send response back to client
                send(clientSocket, responseMsg, strlen(responseMsg), 0);
                break;
            }    
        }

    }

    // Close server socket (never reached in this loop)
    close(serverSocket); 

    return 0; 
}
