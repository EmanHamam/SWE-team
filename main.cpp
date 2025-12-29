#include <iostream>
#include "sqlite3.h"
#include <string>
#include <windows.h>
#include <conio.h>
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
bool executeMenuAction(int choice)
{
    system("cls");

    switch (choice)
    {
    case 0:
        cout << "Showing all properties...\n";
        break;

    case 1:
        cout << "Login screen...\n";
        break;

    case 2:
        cout << "Search properties...\n";
        break;

    case 3:
        cout << "Exiting system...\n";
        return false; // EXIT PROGRAM
    }

    cout << "\n\nPress any key to return to main menu...";
    _getch();
    return true;
}
void runMainMenu()
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
            running = executeMenuAction(selected);
        }
    }
}


// Callback function to handle query results
static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        std::cout << (azColName[i] ? azColName[i] : "NULL") << " = "
                  << (argv[i] ? argv[i] : "NULL") << std::endl;
    }
    std::cout << std::endl;
    return 0;
}

int main() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc;
    const char* db_name = "test.db";

    // 1. Open Database
    rc = sqlite3_open(db_name, &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 0;
    } else {
        std::cout << "Opened database successfully\n";
    }

    // 2. Create SQL statement
    std::string sql = "CREATE TABLE IF NOT EXISTS greetings ("  \
                      "id INTEGER PRIMARY KEY AUTOINCREMENT," \
                      "message TEXT NOT NULL);" \
                      "INSERT INTO greetings (message) VALUES ('Hello World from SQLite!');" \
                      "SELECT * FROM greetings;";

    // 3. Execute SQL statement
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    } else {
        std::cout << "Operation done successfully\n";
    }

    // 4. Close Database
    sqlite3_close(db);

    runMainMenu();
    return 0;
}
