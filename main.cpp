#include <iostream>
#include "sqlite3.h"
#include <string>
#include <windows.h>
#include <conio.h>
#include "PropertyManager.h"
#include "UserManager.h"
#include "DBManager.h"
#include "FilterationManager.h"
#include "AdminManager.h"
#include "ConsoleUtils.h"
using namespace std;

bool isLoggedIn = false;
int currentUserId = -1;
string currentUserEmail = "";
bool isAdmin = false;

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

    const char* menuGuest[] = {
        "View Properties",
        "Login",
        "Signup",
        "Search Properties",
        "Exit"
    };

    const char* menuUser[] = {
        "View Properties",
        "Logout",
        "Search Properties",
        "Exit"
    };

    const char* menuAdmin[] = {
        "Add Property",
        "Delete Property",
        "Update Property",
        "Lock/Unlock Property",
        "View All Properties",
        "View Properties By Owner Id",
        "Logout"
    };

    const char** menu;
    int numOptions;

    if (isLoggedIn && isAdmin) {
        menu = menuAdmin;
        numOptions = 7;
    }
    else if (isLoggedIn) {
        menu = menuUser;
        numOptions = 4;
    }
    else {
        menu = menuGuest;
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
    SearchManager sm(dbManager);
    AdminManager am;

    if (!isLoggedIn)
    {

        switch (choice)
        {
        case 0: pm.ViewAllProperies(db,dbManager); break;
        case 1: // Login
            if (um.login(db))
            {
                isLoggedIn = true;
                if (currentUserEmail == "admin@system.com") isAdmin = true;
            }
            break;
        case 2: um.signup(db); break;
        case 3: sm.interactiveSearch(db); break;
        case 4: cout << "Exiting system...\n"; return false;
        }
    }
    else if (isAdmin)
    {
        switch (choice)
        {
        case 0: am.addProperty(db); break;
        case 1: am.deleteProperty(db); break;
        case 2: am.updateProperty(db); break;
        case 3: am.lockUnlockProperty(db); break;
        case 4: pm.ViewAllProperies(db,dbManager); break;
        case 5: am.viewPropertiesByOwner(db); break;
        case 6: isLoggedIn = false; isAdmin = false; currentUserId=-1; currentUserEmail=""; cout << "Logged out successfully!"; break;
        }
    }
    else
    {
        switch (choice)
        {
        case 0: pm.ViewAllProperies(db,dbManager); break;
        case 1: isLoggedIn = false; currentUserId = -1; currentUserEmail=""; cout << "Logged out successfully!"; break;
        case 2: sm.interactiveSearch(db); break;
        case 3: cout << "Exiting system...\n"; return false;
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
        int maxOption = 0;

        if (!isLoggedIn) maxOption = 4;
        else if (isAdmin) maxOption = 6;
        else maxOption = 3;

        if (key == 72 || key == 'w' || key == 'W') selected = (selected <= 0) ? maxOption : selected - 1;
        else if (key == 80 || key == 's' || key == 'S') selected = (selected >= maxOption) ? 0 : selected + 1;
        else if (key == 13)
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

    if (!dbManager.getDB()) return 0;

    dbManager.initializeDatabase();
    runMainMenu(dbManager.getDB(), &dbManager);

    return 0;
}
