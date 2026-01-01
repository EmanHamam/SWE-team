#ifndef CONSOLEUTILS_H_INCLUDED
#define CONSOLEUTILS_H_INCLUDED

#endif
#pragma once
#include <windows.h>
#include <iostream>
#define NORMAL_PEN 0x07
#define HIGHLIGHTED_PEN 0x70
using namespace std;
inline void gotoxy(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

inline void textattr(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

inline char** multiLineEditor(int xpos, int ypos, int l, char sr[], char er[], int lineno,bool pass)
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

        gotoxy(xpos + oldChar, ypos + oldLine * 2 + 1);
        cout << " ";

        // ÑÓã ÌãíÚ ÇáÓØæÑ
        for (int i = 0; i < lineno; i++)
        {
            gotoxy(xpos, ypos + i * 2);
            textattr(15); // áæä ÚÇÏí
            for (int j = 0; j < lasts[i]; j++)
            {
                if(pass&&i==1)
                    cout<<"*";
                 else
                 cout << lines[i][j];
            }
            for (int j = lasts[i]; j < l; j++) cout << ' ';
        }


        gotoxy(xpos + currentChar, ypos + currentLine * 2 + 1);
        textattr(240); // ÎáÝíÉ ÓæÏÇÁ Úáì ÃÈíÖ
        cout << "_";
        textattr(15);

        oldLine = currentLine;
        oldChar = currentChar;


        gotoxy(xpos + currentChar, ypos + currentLine * 2);

        ch = _getch();

        if (ch == -32)
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
