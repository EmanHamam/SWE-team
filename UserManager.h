#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <iostream>
#include <string>
#include "sqlite3.h"
#include "DBManager.h"

using namespace std;

struct UserSession {
    int userId = -1;
    string email = "";
    bool isLoggedIn = false;
    bool isAdmin = false;
};

class UserManager {
public:
    // This allows us to access the user from anywhere
    static UserSession currentUser;

    bool login(DBManager* dbManager) {
        system("cls");
        string email, pass;
        cout << "\n\n\t--- LOGIN SYSTEM ---" << endl;
        cout << "\tEmail: "; cin >> email;
        cout << "\tPassword: "; cin >> pass;

        // Use DBManager's authenticate function
        User user;
        if (dbManager->authenticate(email, pass, user)) {
            currentUser.userId = user.id;
            currentUser.email = user.email;
            currentUser.isLoggedIn = true;
            currentUser.isAdmin = user.isAdmin;
            
            cout << "\n\tLogin Successful! Welcome " << email << endl;
            _getch();
            return true;
        } else {
            cout << "\n\tInvalid email or password." << endl;
            _getch();
            return false;
        }
    }

    void logout() {
        currentUser = UserSession();
    }
};

// Initialize static member
UserSession UserManager::currentUser;

#endif
