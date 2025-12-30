#include <iostream>
#include "sqlite3.h"
#include <string>
#include <windows.h>
#include <conio.h>
#include "PropertyManager.h"
#include "UserManager.h"
#include "DBManager.h"
using namespace std;

void textattr(int i)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), i);
}

void gotoxy(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
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
void drawMenu(int selected)
{
    system("cls");

    drawHeader();
    const char* menu[] = {
        "View Properties",
        "Login",
        "Search Properties",
        "Exit"
    };
    int y = 12;
    for (int i = 0; i < 4; i++)
    {
        gotoxy(38, y + i * 2);

        if (i == selected)
        {
            textattr(240);
            cout << "> " << menu[i];
        }
        else
        {
            textattr(15);
            cout << "  " << menu[i];
        }
    }

    textattr(15);
}
bool executeMenuAction(int choice, sqlite3* db, DBManager* dbManager)
{
    system("cls");
    PropertyManager pm;
    UserManager um;

    switch (choice)
    {
    case 0:
        pm.viewAllProperties(db, dbManager);
        break;

    case 1:
        um.login(dbManager);
        break;

    case 2:
        cout << "Search properties...\n";
        break;

    case 3:
        cout << "Exiting system...\n";
        return false; // EXIT PROGRAM
    }

    return true;
}
void runMainMenu(sqlite3* db, DBManager* dbManager)
{
    int selected = 0;
    char key;
    bool running = true;
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

    while (running)
    {
        drawMenu(selected);
        key = _getch();

        if (key == 72) // UP
        {
            selected--;
            if (selected < 0) selected = 3;
        }
        else if (key == 80) // DOWN
        {
            selected++;
            if (selected > 3) selected = 0;
        }
        else if (key == 13) // ENTER
        {
            running = executeMenuAction(selected, db, dbManager);
        }
    }
}


int main() {
    DBManager dbManager("test.db");
    
    if (!dbManager.getDB()) {
        return 0;
    }

    dbManager.initializeDatabase();

    runMainMenu(dbManager.getDB(), &dbManager);

    return 0;
}
