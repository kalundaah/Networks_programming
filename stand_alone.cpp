#include <iostream>
#include <cmath>

using namespace std;

void showMenu() {
    cout << "Select an operation to perform on two integers:" << endl;
    cout << "1. Addition" << endl;
    cout << "2. Multiplication" << endl;
    cout << "Enter your choice (1 or 2): ";
}

void showFollowUpMenu() {
    cout << "Select a follow-up operation:" << endl;
    cout << "1. Square the result" << endl;
    cout << "2. Square root the result" << endl;
    cout << "Enter your choice (1 or 2): ";
}

int main() {
    int num1, num2, choice1, choice2;
    double result;

    // Input two integers from the user
    cout << "Enter the first integer: ";
    cin >> num1;
    cout << "Enter the second integer: ";
    cin >> num2;

    // Display the main menu and get the user's choice
    showMenu();
    cin >> choice1;

    // Perform addition or multiplication based on user's choice
    if (choice1 == 1) {
        result = num1 + num2;
    } else if (choice1 == 2) {
        result = num1 * num2;
    } else {
        cout << "Invalid choice. Exiting program." << endl;
        return 1;
    }

    // Display the follow-up menu and get the user's choice
    showFollowUpMenu();
    cin >> choice2;

    // Perform squaring or square root operation based on user's choice
    if (choice2 == 1) {
        result = result * result;
    } else if (choice2 == 2) {
        if (result < 0) {
            cout << "Cannot take the square root of a negative number. Exiting program." << endl;
            return 1;
        }
        result = sqrt(result);
    } else {
        cout << "Invalid choice. Exiting program." << endl;
        return 1;
    }

    // Display the result
    cout << "The final result is: " << result << endl;

    return 0;
}
