#include "UserManager.h"
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <cstring>
#include "ConsoleUtils.h"
#include <regex>
#define NORMAL_PEN 0x07
#define HIGHLIGHTED_PEN 0x70

using namespace std;

extern bool isLoggedIn;
extern int currentUserId;
extern string currentUserEmail;


void LeftMove(int* current)
{
    if (*current > 0) (*current)--;
}

void RightMove(int* current, int* last)
{
    if (*current < *last) (*current)++;
}

void PressedHome(int* current, int* last)
{
    *current = 0;
}

void PressedEnd(int* current, int* last)
{
    *current = *last;
}

void DeleteChar(char* line, int* current, int* last)
{
    if (*current > *last) return;
    for (int i = *current; i < *last; i++)
        line[i] = line[i + 1];
    line[*last] = ' ';
    if (*last > 0) (*last)--;
}

void Backspace(char* line, int* current, int* last)
{
    if (*current > 0)
    {
        (*current)--;
        for (int i = *current; i < *last; i++)
            line[i] = line[i + 1];
        line[*last] = ' ';
        if (*last > 0) (*last)--;
    }
}





string trim(const char* s)
{
    string str(s);

    size_t start = str.find_first_not_of(' ');
    if (start != string::npos)
        str = str.substr(start);
    else
        return "";

    while (!str.empty() && str.back() == ' ')
        str.pop_back();

    return str;
}
bool UserManager::login(sqlite3* db)
{
    string email, password;
    int focused = 0; // 0 = Email field, 1 = Password field, 2 = Sign Up button

    const int width = 40, height = 10;
    const int startX = 15, startY = 5;

    while (true)
    {
        system("cls");

        // === Draw frame - exactly like your original ===
        for (int y = startY; y <= startY + height; y++)
        {
            gotoxy(startX, y);
            for (int x = startX; x <= startX + width; x++)
            {
                if (y == startY || y == startY + height) cout << "-";
                else if (x == startX || x == startX + width) cout << "|";
                else cout << " ";
            }
        }

        // === Title - same as original ===
        gotoxy(startX + 12, startY);
        textattr(HIGHLIGHTED_PEN);
        cout << " LOGIN";
        textattr(NORMAL_PEN);

        // === Labels - same positions ===
        gotoxy(startX + 2, startY + 3); cout << "Email: ";
        gotoxy(startX + 2, startY + 5); cout << "Password:  ";

        // === Display current input (with highlight on focused field) ===
        // Email
        gotoxy(startX + 13, startY + 3);
        if (focused == 0) textattr(BACKGROUND_INTENSITY | FOREGROUND_BLUE);
        cout << string(25, ' ');
        gotoxy(startX + 13, startY + 3);
        cout << email.substr(0, 25);
        if (focused == 0) textattr(NORMAL_PEN);

        // Password (as stars)
        gotoxy(startX + 13, startY + 5);
        if (focused == 1) textattr(BACKGROUND_INTENSITY | FOREGROUND_BLUE);
        cout << string(25, ' ');
        gotoxy(startX + 13, startY + 5);
        cout << string(password.length(), '*');
        if (focused == 1) textattr(NORMAL_PEN);

        // === Sign Up button - centered below ===
        int btnX = startX + 14;
        int btnY = startY + height + 2;
        gotoxy(btnX, btnY);
        if (focused == 2)
            textattr(BACKGROUND_BLUE | FOREGROUND_INTENSITY);  // Highlighted button
        else
            textattr(FOREGROUND_INTENSITY);
        cout << "[ Sign Up ]";
        textattr(NORMAL_PEN);

        // Small navigation hint
        gotoxy(startX + 2, startY + height + 4);
        cout << "Use UP-DOWN arrows , ENTER to select";

        // === Input handling ===
        int key = _getch();

        if (key == 0 || key == 224)  // Arrow keys
        {
            key = _getch();
            if (key == 72) focused = max(0, focused - 1);        // Up
            if (key == 80) focused = min(2, focused + 1);        // Down
        }
        else if (key == '\t')
        {
            focused = (focused + 1) % 3;
        }
        else if (key == 13)  // Enter
        {
            if (focused == 2)  // Sign Up button selected
            {
                signup(db);               // Go to signup
                email.clear();
                password.clear();
                focused = 0;
                continue;                 // Return to login screen
            }
            else  // Enter on Email or Password field → edit + login
            {
                char sr[] = { ' ', ' ' };
                char er[] = { '~', '~' };

                char** input = multiLineEditor(startX + 13, startY + 3, 25, sr, er, 2, 1);

                email = trim(input[0]);
                password = trim(input[1]);

                for (int i = 0; i < 2; i++) delete[] input[i];
                delete[] input;

                // Validate and login
                if (email.empty() || password.empty())
                {
                    gotoxy(startX + 2, startY + height + 2);
                    textattr(FOREGROUND_RED | FOREGROUND_INTENSITY);
                    cout << "Email and password are required!                  ";
                    textattr(NORMAL_PEN);
                    _getch();
                    continue;
                }

                sqlite3_stmt* stmt;
                const char* sql = "SELECT id FROM users WHERE email=? AND password=?";

                if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
                {
                    gotoxy(startX + 2, startY + height + 2);
                    cout << "Database error!                                   ";
                    _getch();
                    continue;
                }

                sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

                if (sqlite3_step(stmt) == SQLITE_ROW)
                {
                    currentUserId = sqlite3_column_int(stmt, 0);
                    currentUserEmail = email;
                    isLoggedIn = true;

                    gotoxy(startX + 2, startY + height + 2);
                    textattr(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    cout << "Login successful! Welcome back.                   ";
                    textattr(NORMAL_PEN);
                    sqlite3_finalize(stmt);
                    _getch();
                    return true;
                }
                else
                {
                    gotoxy(startX + 2, startY + height + 2);
                    textattr(FOREGROUND_RED | FOREGROUND_INTENSITY);
                    cout << "Wrong email or password!                          ";
                    textattr(NORMAL_PEN);
                    sqlite3_finalize(stmt);
                    _getch();
                    email.clear();
                    password.clear();
                }
            }
        }
        else if (key == 27)  // ESC
        {
            return false;
        }
    }

    return false;
}
bool UserManager::signup(sqlite3* db)
{
    system("cls");
    int width = 40, height = 10;
    int startX = 15, startY = 5;

    // Frame
    for (int y = startY; y <= startY + height; y++)
    {
        gotoxy(startX, y);
        for (int x = startX; x <= startX + width; x++)
        {
            if (y == startY || y == startY + height) cout << "-";
            else if (x == startX || x == startX + width) cout << "|";
            else cout << " ";
        }
    }

    gotoxy(startX + 10, startY);
    textattr(HIGHLIGHTED_PEN);
    cout << " SIGN UP ";
    textattr(NORMAL_PEN);

    gotoxy(startX + 2, startY + 3); cout << "Email: ";
    gotoxy(startX + 2, startY + 5); cout << "Password: ";

    char sr[] = { ' ', ' ' };
    char er[] = { '~', '~' };

    char** input = multiLineEditor(startX + 13, startY + 3, 25, sr, er, 2,1);

    string email = trim(input[0]);
    string password = trim(input[1]);

    for (int i = 0; i < 2; i++) delete[] input[i];
    delete[] input;

    if (email.empty() || password.empty())
    {
        gotoxy(startX + 2, startY + height + 2);
        cout << "Email and password required!";
        _getch();
        return false;
    }


    regex emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    regex passwordRegex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[^A-Za-z0-9]).{8,}$");

    if (!regex_match(email, emailRegex))
    {
        gotoxy(startX + 2, startY + height + 2);
        cout << "Invalid email format!";
        _getch();
        return false;
    }

    if (!regex_match(password, passwordRegex))
    {
        gotoxy(startX + 2, startY + height + 2);
        cout << "Weak password!";
        gotoxy(startX + 2, startY + height + 3);
        cout << "Min 8 chars, upper, lower, digit, symbol";
        _getch();
        return false;
    }


    string checkSQL =
        "SELECT id FROM users WHERE email = '" + email + "';";

    sqlite3_stmt* checkStmt;
    if (sqlite3_prepare_v2(db, checkSQL.c_str(), -1, &checkStmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(checkStmt) == SQLITE_ROW)
        {
            gotoxy(startX + 2, startY + height + 2);
            cout << "Email already exists!";
            sqlite3_finalize(checkStmt);
            _getch();
            return false;
        }
    }
    sqlite3_finalize(checkStmt);


    string insertSQL =
        "INSERT INTO users (email, password, isAdmin) VALUES ('"
        + email + "', '" + password + "', 0);";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, insertSQL.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        gotoxy(startX + 2, startY + height + 2);
        cout << "Signup failed!";
        sqlite3_free(errMsg);
        _getch();
        return false;
    }

    gotoxy(startX + 2, startY + height + 2);
    cout << "Signup successful! You can login now.";
    _getch();
    return true;
}
