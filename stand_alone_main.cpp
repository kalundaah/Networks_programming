#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

using namespace std;


// Placeholders for processing
string line;
string cell;
vector<string> row;

// Storage
vector<vector<string>> books_vector;

void DisplayCatalog() {
    cout << "Display Catalog" << endl << endl;
    for (const auto& book: books_vector) {
        for(const auto& book_field: book) {
            cout << book_field << "\t\t";         
        }
        // To separate each book entry with a line break
        cout << endl;
    }
}

vector<string> searchforItem(){
    try {
        cout << "Search for Item" << endl << endl;
        // Variable that stores the 
        string query;
        vector<string> found_row;

        cout << "Enter the book you wish to find" << endl;
        
        getline(cin, query);

        bool found = false; // The boolean that completes the logic

        for (const auto& row : books_vector) {
            for (const auto& cell : row) {
                if (cell == query) {
                    found = true;
                    found_row = row;
                     for (const auto& book_field: found_row) {
                        cout << book_field << "\t\t";         
                    }
                    break; // Value found, no need to continue searching
                }
            }
            if (found) {
                break; // Value found in some row, no need to continue searching
            }
        }
        
        if(!found) {
            throw exception();
        }
        return found_row;
    } catch(...) {
        cout << endl;
        cerr << "No book found" << endl;
        cout << endl;
    }
    

}

int Purchaseitem(){
    try {
        cout << "Purchase Item" << endl << endl;
        vector <string> query = searchforItem();
        
        // grab price cell
        string priceholder = query.at(query.size() - 1);
        
        // Convert to integer
        int price = stoi(priceholder);
        // delete &priceholder;

        return price;
    }
    catch(...) {
        cout << endl;
        cerr << "No book found" << endl;
        cout << endl;
    }
}

void PayforItem(){
    try {
        cout << "Pay for Item" << endl << endl;
        int price = Purchaseitem();
        string response;
        cout << "Pay " << price << " for the book?" << endl;
        getline(cin, response);

        cout << "Paid";
    } catch(...) {
        cout << endl;
        cerr << "No book found" << endl;
        cout << endl;
    }
    
}

int ReadFromFile() {
    cout << "Read from file" << endl << endl;
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
    cout << "Display Menu" << endl << endl;
    cout << "1. Display Catalog" << endl;
    cout << "2. Search for Item" << endl;
    cout << "3. Purchase Item" << endl;
    cout << "4. Pay for Item" << endl;
}

int main(int argc, char const *argv[])
{
    ReadFromFile();
    displayMenu();
    DisplayCatalog();

    
  
    // Getting Item after searching
    vector<string> search_for_item_found = searchforItem();

    Purchaseitem();
    PayforItem();
    
    return 0;
}


