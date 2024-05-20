#include <stdio.h>
#include <math.h>

void showMenu() {
    printf("Select an operation to perform on two integers:\n");
    printf("1. Addition\n");
    printf("2. Multiplication\n");
    printf("Enter your choice (1 or 2): ");
}

void showFollowUpMenu() {
    printf("Select a follow-up operation:\n");
    printf("1. Square the result\n");
    printf("2. Square root the result\n");
    printf("Enter your choice (1 or 2): ");
}

int main() {
    int num1, num2, choice1, choice2;
    double result;

    // Input two integers from the user
    printf("Enter the first integer: ");
    scanf("%d", &num1);
    printf("Enter the second integer: ");
    scanf("%d", &num2);

    // Display the main menu and get the user's choice
    showMenu();
    scanf("%d", &choice1);

    // Perform addition or multiplication based on user's choice
    if (choice1 == 1) {
        result = num1 + num2;
    } else if (choice1 == 2) {
        result = num1 * num2;
    } else {
        printf("Invalid choice. Exiting program.\n");
        return 1;
    }

    // Display the follow-up menu and get the user's choice
    showFollowUpMenu();
    scanf("%d", &choice2);

    // Perform squaring or square root operation based on user's choice
    if (choice2 == 1) {
        result = result * result;
    } else if (choice2 == 2) {
        if (result < 0) {
            printf("Cannot take the square root of a negative number. Exiting program.\n");
            return 1;
        }
        result = sqrt(result);
    } else {
        printf("Invalid choice. Exiting program.\n");
        return 1;
    }

    // Display the result
    printf("The final result is: %f\n", result);

    return 0;
}
