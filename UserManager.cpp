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
    int focused = 0; // 0=Email, 1=Password, 2=Sign Up, 3=Back
    const int width = 40, height = 10;
    const int startX = 15, startY = 5;

    while (true)
    {
        system("cls");

        // Draw frame (same as original)
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

        // Title
        gotoxy(startX + 12, startY);
        textattr(HIGHLIGHTED_PEN);
        cout << " LOGIN";
        textattr(NORMAL_PEN);

        // Labels
        gotoxy(startX + 2, startY + 3); cout << "Email: ";
        gotoxy(startX + 2, startY + 5); cout << "Password:  ";

        // Email field
        gotoxy(startX + 13, startY + 3);
        if (focused == 0) textattr(BACKGROUND_INTENSITY | FOREGROUND_BLUE);
        cout << string(25, ' ');
        gotoxy(startX + 13, startY + 3);
        cout << email.substr(0, 25);
        if (focused == 0) textattr(NORMAL_PEN);

        // Password field
        gotoxy(startX + 13, startY + 5);
        if (focused == 1) textattr(BACKGROUND_INTENSITY | FOREGROUND_BLUE);
        cout << string(25, ' ');
        gotoxy(startX + 13, startY + 5);
        cout << string(password.length(), '*');
        if (focused == 1) textattr(NORMAL_PEN);

        // Sign Up button
        gotoxy(startX + 14, startY + height + 2);
        if (focused == 2) textattr(BACKGROUND_BLUE | FOREGROUND_INTENSITY);
        else textattr(FOREGROUND_INTENSITY);
        cout << "[ Sign Up ]";
        textattr(NORMAL_PEN);

        // Back button (new)
        gotoxy(startX + 14, startY + height + 4);
        if (focused == 3) textattr(BACKGROUND_RED | FOREGROUND_INTENSITY);
        else textattr(FOREGROUND_INTENSITY);
        cout << "[ Back ]";
        textattr(NORMAL_PEN);

        // Hint
        gotoxy(startX + 2, startY + height + 6);
        cout << "up/down arrows to navigate - ENTER to select - ESC to go back";

        int key = _getch();

        if (key == 0 || key == 224) // Arrow keys
        {
            key = _getch();
            if (key == 72) focused = max(0, focused - 1); // Up
            if (key == 80) focused = min(3, focused + 1); // Down
        }
        else if (key == '\t')
        {
            focused = (focused + 1) % 4;
        }
        else if (key == 13) // Enter
        {
            if (focused == 2) // Sign Up
            {
                signup(db);
                email.clear();
                password.clear();
                focused = 0;
                continue;
            }
            else if (focused == 3) // Back
            {
                return false; // Go back to main menu
            }
            else // Email or Password → edit and try login
            {
                char sr[] = { ' ', ' ' };
                char er[] = { '~', '~' };
                char** input = multiLineEditor(startX + 13, startY + 3, 25, sr, er, 2, 1);
                email = trim(input[0]);
                password = trim(input[1]);
                for (int i = 0; i < 2; i++) delete[] input[i];
                delete[] input;

                if (email.empty() || password.empty())
                {
                    gotoxy(startX + 2, startY + height + 2);
                    textattr(FOREGROUND_RED | FOREGROUND_INTENSITY);
                    cout << "Email and password are required!            ";
                    textattr(NORMAL_PEN);
                    _getch();
                    continue;
                }

                sqlite3_stmt* stmt;
                const char* sql = "SELECT id FROM users WHERE email=? AND password=?";
                if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
                {
                    gotoxy(startX + 2, startY + height + 2);
                    cout << "Database error!                             ";
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
                    cout << "Login successful! Welcome back.             ";
                    textattr(NORMAL_PEN);
                    sqlite3_finalize(stmt);
                    _getch();
                    return true;
                }
                else
                {
                    gotoxy(startX + 2, startY + height + 2);
                    textattr(FOREGROUND_RED | FOREGROUND_INTENSITY);
                    cout << "Wrong email or password!                    ";
                    textattr(NORMAL_PEN);
                    sqlite3_finalize(stmt);
                    _getch();
                    email.clear();
                    password.clear();
                }
            }
        }
        else if (key == 27) // ESC key → back to main menu
        {
            return false;
        }
    }

    return false;
}bool UserManager::signup(sqlite3* db)
{
    string email, password;
    int focused = 0; // 0=Email, 1=Password, 2=Back
    const int width = 40, height = 10;
    const int startX = 15, startY = 5;

    while (true)
    {
        system("cls");

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

        // Title
        gotoxy(startX + 10, startY);
        textattr(HIGHLIGHTED_PEN);
        cout << " SIGN UP ";
        textattr(NORMAL_PEN);

        // Labels
        gotoxy(startX + 2, startY + 3); cout << "Email: ";
        gotoxy(startX + 2, startY + 5); cout << "Password:  ";

        // Email field
        gotoxy(startX + 13, startY + 3);
        if (focused == 0) textattr(BACKGROUND_INTENSITY | FOREGROUND_BLUE);
        cout << string(25, ' ');
        gotoxy(startX + 13, startY + 3);
        cout << email.substr(0, 25);
        if (focused == 0) textattr(NORMAL_PEN);

        // Password field
        gotoxy(startX + 13, startY + 5);
        if (focused == 1) textattr(BACKGROUND_INTENSITY | FOREGROUND_BLUE);
        cout << string(25, ' ');
        gotoxy(startX + 13, startY + 5);
        cout << string(password.length(), '*');
        if (focused == 1) textattr(NORMAL_PEN);

        // Back button
        gotoxy(startX + 16, startY + height + 3);
        if (focused == 2) textattr(BACKGROUND_RED | FOREGROUND_INTENSITY);
        else textattr(FOREGROUND_INTENSITY);
        cout << "[ Back ]";
        textattr(NORMAL_PEN);

        // Hint
        gotoxy(startX + 2, startY + height + 5);
        cout << "up/down arrows - ENTER to confirm/edit - ESC to go back";

        int key = _getch();

        if (key == 0 || key == 224)
        {
            key = _getch();
            if (key == 72) focused = max(0, focused - 1);
            if (key == 80) focused = min(2, focused + 1);
        }
        else if (key == '\t')
        {
            focused = (focused + 1) % 3;
        }
        else if (key == 13) // Enter
        {
            if (focused == 2) // Back
            {
                return false; // Return to login or main menu
            }
            else // Edit fields and try signup
            {
                char sr[] = { ' ', ' ' };
                char er[] = { '~', '~' };
                char** input = multiLineEditor(startX + 13, startY + 3, 25, sr, er, 2, 1);
                email = trim(input[0]);
                password = trim(input[1]);
                for (int i = 0; i < 2; i++) delete[] input[i];
                delete[] input;

                if (email.empty() || password.empty())
                {
                    gotoxy(startX + 2, startY + height + 2);
                    cout << "Email and password required!                ";
                    _getch();
                    continue;
                }

                regex emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
                regex passwordRegex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[^A-Za-z0-9]).{8,}$");

                if (!regex_match(email, emailRegex))
                {
                    gotoxy(startX + 2, startY + height + 2);
                    cout << "Invalid email format!                       ";
                    _getch();
                    continue;
                }
                if (!regex_match(password, passwordRegex))
                {
                    gotoxy(startX + 2, startY + height + 2);
                    cout << "Weak password! Min 8 chars + upper + digit + symbol";
                    _getch();
                    continue;
                }

                // Check if email exists
                sqlite3_stmt* checkStmt;
                string checkSQL = "SELECT id FROM users WHERE email=?";
                if (sqlite3_prepare_v2(db, checkSQL.c_str(), -1, &checkStmt, nullptr) == SQLITE_OK)
                {
                    sqlite3_bind_text(checkStmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
                    if (sqlite3_step(checkStmt) == SQLITE_ROW)
                    {
                        gotoxy(startX + 2, startY + height + 2);
                        cout << "Email already exists!                       ";
                        sqlite3_finalize(checkStmt);
                        _getch();
                        continue;
                    }
                    sqlite3_finalize(checkStmt);
                }

                // Insert new user
                string insertSQL = "INSERT INTO users (email, password, isAdmin) VALUES (?, ?, 0);";
                sqlite3_stmt* stmt;
                if (sqlite3_prepare_v2(db, insertSQL.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
                {
                    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
                    if (sqlite3_step(stmt) == SQLITE_DONE)
                    {
                        gotoxy(startX + 2, startY + height + 2);
                        textattr(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                        cout << "Signup successful! You can login now.       ";
                        textattr(NORMAL_PEN);
                        _getch();
                        return true;
                    }
                }
                gotoxy(startX + 2, startY + height + 2);
                cout << "Signup failed!                              ";
                _getch();
            }
        }
        else if (key == 27) // ESC
        {
            return false;
        }
    }

    return false;
}
