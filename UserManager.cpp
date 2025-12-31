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



char** multiLineEditor(int xpos, int ypos, int l, char sr[], char er[], int lineno)
{
    char** lines = new char*[lineno];
    int* lasts = new int[lineno];

    for (int i = 0; i < lineno; i++)
    {
        lines[i] = new char[l + 1];
        memset(lines[i], ' ', l);
        lines[i][l] = '\0';
        lasts[i] = 0;
    }

    int currentLine = 0, currentChar = 0;
    int oldLine = 0, oldChar = 0;
    char ch;
    bool done = false;

    while (!done)
    {
        // ãÓÍ ÇáãÄÔÑ ÇáÞÏíã
        gotoxy(xpos + oldChar, ypos + oldLine * 2 + 1);
        cout << " ";

        // ÑÓã ÌãíÚ ÇáÓØæÑ
        for (int i = 0; i < lineno; i++)
        {
            gotoxy(xpos, ypos + i * 2);
            textattr(15); // áæä ÚÇÏí
            for (int j = 0; j < lasts[i]; j++)
            {
                if (i == 1) cout << '*'; // ßáãÉ ÇáãÑæÑ ãÎÝíÉ
                else cout << lines[i][j];
            }
            for (int j = lasts[i]; j < l; j++) cout << ' '; // ãÓÍ Ãí ÈÞÇíÇ
        }

        // ÑÓã ÇáãÄÔÑ ÊÍÊ ÇáãßÇä ÇáÍÇáí
        gotoxy(xpos + currentChar, ypos + currentLine * 2 + 1);
        textattr(240); // ÎáÝíÉ ÓæÏÇÁ Úáì ÃÈíÖ
        cout << "_";
        textattr(15);

        oldLine = currentLine;
        oldChar = currentChar;

        // æÖÚ ÇáãÄÔÑ Ýí ãßÇä ÇáßÊÇÈÉ (ÍÑÝ ÂÎÑ)
        gotoxy(xpos + currentChar, ypos + currentLine * 2);

        ch = _getch();

        if (ch == -32) // Arrow keys
        {
            ch = _getch();
            switch (ch)
            {
                case 72: // Up
                    if (currentLine > 0) { currentLine--; if(currentChar > lasts[currentLine]) currentChar = lasts[currentLine]; }
                    break;
                case 80: // Down
                    if (currentLine < lineno - 1) { currentLine++; if(currentChar > lasts[currentLine]) currentChar = lasts[currentLine]; }
                    break;
                case 75: // Left
                    if (currentChar > 0) currentChar--;
                    break;
                case 77: // Right
                    if (currentChar < lasts[currentLine]) currentChar++;
                    break;
                case 71: // Home
                    currentChar = 0;
                    break;
                case 79: // End
                    currentChar = lasts[currentLine];
                    break;
                case 83: // Delete
                    if (currentChar < lasts[currentLine])
                    {
                        for (int i = currentChar; i < lasts[currentLine] - 1; i++)
                            lines[currentLine][i] = lines[currentLine][i + 1];
                        lines[currentLine][lasts[currentLine] - 1] = ' ';
                        lasts[currentLine]--;
                    }
                    break;
            }
        }
        else
        {
            switch (ch)
            {
                case 8: // Backspace
                    if (currentChar > 0)
                    {
                        currentChar--;
                        for (int i = currentChar; i < lasts[currentLine] - 1; i++)
                            lines[currentLine][i] = lines[currentLine][i + 1];
                        lines[currentLine][lasts[currentLine] - 1] = ' ';
                        lasts[currentLine]--;
                    }
                    break;
                case 13: // Enter
                    if (currentLine == lineno - 1) done = true;
                    else { currentLine++; currentChar = 0; }
                    break;
                default:
                    if (ch >= sr[currentLine] && ch <= er[currentLine] && lasts[currentLine] < l)
                    {
                        for (int i = lasts[currentLine]; i > currentChar; i--)
                            lines[currentLine][i] = lines[currentLine][i - 1];
                        lines[currentLine][currentChar++] = ch;
                        lasts[currentLine]++;
                    }
            }
        }
    }

    // Null terminate lines
    for (int i = 0; i < lineno; i++)
        lines[i][lasts[i]] = '\0';

    delete[] lasts;
    return lines;
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
    system("cls"); // Clear console
    int width = 40, height = 10;
    int startX = 15, startY = 5;

    // Draw frame
    for (int y = startY; y <= startY + height; y++)
    {
        gotoxy(startX, y);
        for (int x = startX; x <= startX + width; x++)
        {
            if (y == startY || y == startY + height) cout << "-"; // top/bottom
            else if (x == startX || x == startX + width) cout << "|"; // sides
            else cout << " ";
        }
    }

    // Title
    gotoxy(startX + 12, startY); textattr(HIGHLIGHTED_PEN);
    cout << " LOGIN";
    textattr(NORMAL_PEN);

    // Labels
    gotoxy(startX + 2, startY + 3); cout << "Email: ";
    gotoxy(startX + 2, startY + 5); cout << "Password:  ";

    gotoxy(startX + 2, startY + height + 3);
    textattr(8);
    cout << "Do not have an account yet? Press [S] to Sign Up";
    char c = _getch();
    if (c == 's' || c == 'S')
    {
        signup(db);
        return false;
    }
        // Set allowed chars
    char sr[] = { ' ', ' ' };
    char er[] = { '~', '~' };

    // Start editor at positions next to labels
    char** input = multiLineEditor(startX + 13, startY + 3, 25, sr, er, 2);

    string email = trim(input[0]);
    string password = trim(input[1]);

    // Free memory
    for (int i = 0; i < 2; i++) delete[] input[i];
    delete[] input;

    if (email.empty() || password.empty())
    {
        gotoxy(startX + 2, startY + height + 2);
        cout << "Invalid email or password!\n";
        return false;
    }

    sqlite3_stmt* stmt;
    const char* sql = "SELECT id FROM users WHERE email=? AND password=?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        gotoxy(startX + 2, startY + height + 2);
        cout << "Database error!\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        currentUserId = sqlite3_column_int(stmt, 0);
        currentUserEmail = email;
        isLoggedIn = true;

        gotoxy(startX + 2, startY + height + 2);
        cout << "Login successful!\n";
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    gotoxy(startX + 2, startY + height + 2);
    cout << "Wrong email or password!\n";
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

    char** input = multiLineEditor(startX + 13, startY + 3, 25, sr, er, 2);

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

