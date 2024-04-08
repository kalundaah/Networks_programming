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

void DisplayCatalog() {
    for (const auto& book: books_vector) {
        for(const auto& book_field: book) {
            cout << book_field << '\t\t';         
        }
        // To separate each book entry with a line break
        cout << endl;
    }
}

vector<string> searchforItem(){
    // Variable that stores the 
    string query;
    vector<string> found_row;

    cout << "Enter the book you wish to find" << endl;
    
    cin >> query;

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

int Purchaseitem(){
    vector <string> query = searchforItem();
    
    // grab price cell
    string priceholder = query.at(query.size() - 1);
    
    // Convert to integer
    int price = stoi(priceholder);
    delete &priceholder;

    return price;
}

void PayforItem(){
    int price = Purchaseitem();
    char response;
    cout << "Pay " << price << " for the book?" << endl;
    cin >> response;
    cout << "Paid";
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

void displayMenu() {
    cout << "1. Display Catalog" << endl;
    cout << "2. Search for Item" << endl;
    cout << "3. Purchase Item" << endl;
    cout << "4. Pay for Item" << endl;
}

int main(int argc, char const *argv[])
{
    // logic ya SOCKET (Recipient)

    // Await response
        // switch if value 1,2,3,4
            // do corresponding task

    return 0;
}


