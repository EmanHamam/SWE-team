#ifndef CONSOLEUTILS_H_INCLUDED
#define CONSOLEUTILS_H_INCLUDED

#endif
#pragma once
#include <windows.h>
#include <iostream>
#define NORMAL_PEN 0x07
#define HIGHLIGHTED_PEN 0x70

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
