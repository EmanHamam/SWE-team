#include <iostream>
#include "sqlite3.h"
#include <string>
#include <windows.h>
#include <conio.h>
#include "PropertyManager.h"
#include "UserManager.h"
#include "DBManager.h"
#include "ConsoleUtils.h"

using namespace std;

bool isLoggedIn = false;
int currentUserId = -1;
string currentUserEmail = "";

/* ================= HEADER ================= */

void drawHeader() {
    textattr(14);
    gotoxy(15, 1); cout << "  _____  ______          _        ______  _____ _______       _______ ______ ";
    gotoxy(15, 2); cout << " |  __ \\|  ____|   /\\   | |      |  ____|/ ____|__   __|/\\   |__   __|  ____|";
    gotoxy(15, 3); cout << " | |__) | |__     /  \\  | |      | |__  | (___    | |  /  \\     | |  | |__   ";
    gotoxy(15, 4); cout << " |  _  /|  __|   / /\\ \\ | |      |  __|  \\___ \\   | | / /\\ \\    | |  |  __|  ";
    gotoxy(15, 5); cout << " | | \\ \\| |____ / ____ \\| |____  | |____ ____) |  | |/ ____ \\   | |  | |____ ";
    gotoxy(15, 6); cout << " |_|  \\_\\______/_/    \\_\\______| |______|_____/   |_/_/    \\_\\  |_|  |______|";

    textattr(11);
    gotoxy(35, 8); cout << "--- PROPERTY MANAGEMENT SYSTEM ---";
}

/* ================= MENU ================= */

void drawMenu(int selected, bool firstDraw)
{
    if (firstDraw) {
        system("cls");
        drawHeader();
    }

    const char* menuLoggedOut[] = {
        "View Properties",
        "Login",
        "Signup",
        "Search Properties",
        "Exit"
    };

    const char* menuLoggedIn[] = {
        "View Properties",
        "Logout",
        "Search Properties",
        "Exit"
    };

    const char** menu;
    int numOptions;

    if (isLoggedIn) {
        menu = menuLoggedIn;
        numOptions = 4;
    } else {
        menu = menuLoggedOut;
        numOptions = 5;
    }

    int y = 12;
    for (int i = 0; i < numOptions; i++)
    {
        gotoxy(38, y + i * 2);

        if (i == selected)
        {
            textattr(240);
            cout << "> " << menu[i] << "  ";
        }
        else
        {
            textattr(15);
            cout << "  " << menu[i] << "  ";
        }
    }
    textattr(15);
}

/* ================= ACTIONS ================= */

bool executeMenuAction(int choice, sqlite3* db, DBManager* dbManager)
{
    system("cls");
    PropertyManager pm;
    UserManager um;

    if (!isLoggedIn)
    {
        switch (choice)
        {
        case 0:
            pm.viewAllProperties(db);
            break;

        case 1: // Login
            if (um.login(db))
                isLoggedIn = true;
            break;

        case 2: // Signup
            um.signup(db);
            break;

        case 3:
            cout << "Search functionality coming soon...";
            break;

        case 4:
            cout << "Exiting system...\n";
            return false;
        }
    }
    else
    {
        switch (choice)
        {
        case 0:
            pm.viewAllProperties(db);
            break;

        case 1: // Logout
            isLoggedIn = false;
            currentUserId = -1;
            currentUserEmail = "";
            cout << "Logged out successfully!";
            break;

        case 2:
            cout << "Search functionality coming soon...";
            break;

        case 3:
            cout << "Exiting system...\n";
            return false;
        }
    }

    textattr(8);
    cout << "\n\nPress any key to return to main menu...";
    _getch();
    return true;
}

/* ================= MAIN LOOP ================= */

void runMainMenu(sqlite3* db, DBManager* dbManager)
{
    int selected = 0;
    char key;
    bool running = true;
    bool needsFullRedraw = true;

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

    while (running)
    {
        drawMenu(selected, needsFullRedraw);
        needsFullRedraw = false;

        key = _getch();

        int maxOption = isLoggedIn ? 3 : 4;

        if (key == 72 || key == 'w' || key == 'W') // UP
            selected = (selected <= 0) ? maxOption : selected - 1;

        else if (key == 80 || key == 's' || key == 'S') // DOWN
            selected = (selected >= maxOption) ? 0 : selected + 1;

        else if (key == 13) // ENTER
        {
            running = executeMenuAction(selected, db, dbManager);
            selected = 0;
            needsFullRedraw = true;
        }
    }
}

/* ================= MAIN ================= */

int main()
{
    DBManager dbManager("test.db");

    if (!dbManager.getDB())
        return 0;

    dbManager.initializeDatabase();
    runMainMenu(dbManager.getDB(), &dbManager);

    return 0;
}
