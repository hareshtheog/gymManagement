#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Database.h"
#include "AuthSystem.h"
#include "User.h"
#include <iostream>
#include <conio.h>

void clearScreen() {
    system("cls");
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    clearScreen();
    std::cout << "========================================\n";
    std::cout << "   GYM MANAGEMENT SYSTEM STARTUP\n";
    std::cout << "========================================\n";

    Database* db = Database::getInstance();
    if (!db->isConnected()) {
        std::cout << "\nDatabase connection failed!\n";
        std::cout << "Check MySQL service & DLL files.\n";
        _getch();
        return 1;
    }

    std::cout << "\nDatabase ready!\n";

    AuthSystem auth;
    User* currentUser = nullptr;
    int choice;

    do {
        std::cout << "\n=== MAIN MENU ===\n";
        std::cout << "1. Login\n";
        std::cout << "2. Register\n";
        std::cout << "3. Exit\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            currentUser = auth.login();
            if (currentUser) {
                currentUser->displayMenu();
                delete currentUser;
                currentUser = nullptr;
            }
            break;
        case 2:
            auth.registerUser();
            break;
        case 3:
            std::cout << "Goodbye!\n";
            break;
        default:
            std::cout << "Invalid choice.\n";
        }
    } while (choice != 3);

    db->closeConnection();
    return 0;
}