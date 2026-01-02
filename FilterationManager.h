#ifndef SEARCH_MANAGER_H
#define SEARCH_MANAGER_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <windows.h>
#include <conio.h>
#include "sqlite3.h"
#include "DBManager.h"
#include "PropertyManager.h"
using namespace std;
string toLower(string s)
{
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c)
    {
        return tolower(c);
    });
    return s;
}

struct SearchFilter
{
    double maxPrice = 999999999;
    string type = ""; // Rent / Buy / empty
    string location = "";
    int minRooms = 0;
    int minBaths = 0;
    double minArea = 0;
};

class SearchManager
{

public:
    SearchManager(DBManager *db = nullptr) : dbManager(db) {}

    void setDBManager(DBManager *db)
    {
        dbManager = db;
    }

    // ================= INTERACTIVE FILTER MENU =================
    void interactiveSearch(sqlite3 *db)
    {
        bool filtering = true;
        int selectedFilter = 0;
        const int numOptions = 9;
        bool needsClear = true;

        while (filtering)
        {
            if (needsClear)
            {
                system("cls");
                needsClear = false;
            }
            drawFilterMenu(selectedFilter, numOptions);
            displayCurrentFilters();

            int key = _getch();
            if (key == 0 || key == 224)
            {
                key = _getch();
                if (key == 72)
                    selectedFilter = (selectedFilter == 0) ? numOptions - 1 : selectedFilter - 1;
                else if (key == 80)
                    selectedFilter = (selectedFilter == numOptions - 1) ? 0 : selectedFilter + 1;
            }
            else if (key == 13)
            {

                if (selectedFilter == 6)
                {
                    viewFilteredResults(db);
                    needsClear = true;
                }
                else if (selectedFilter == 7)
                {
                    resetFilters();
                    needsClear = true;
                }
                else if (selectedFilter == 8)
                {
                    filtering = false;
                    system("cls");
                    setAttr(11);
                    setXY(30, 10);
                    cout << "Returning to Main Menu...";
                    setAttr(15);
                    _getch();
                }
                else
                {
                    handleFilterInput(selectedFilter);
                    needsClear = true;
                }
            }

            else if (key == 27)
            {
                system("cls");
                exit(0);
            }
        }
    }

private:
    DBManager *dbManager;
    PropertyManager pm;
    SearchFilter currentFilter;
    // =============== Helpers Function ===============
    void drawFilterMenu(int selected, int numOptions)
    {

        setAttr(11);
        setXY(20, 2);
        cout << "====================================================";
        setXY(20, 3);
        cout << "           ADVANCED PROPERTY SEARCH & FILTER";
        setXY(20, 4);
        cout << "====================================================";

        const char *filterOptions[] =
        {
            "Maximum Price ($)",
            "Property Type (Rent/Buy)",
            "Location",
            "Minimum Rooms",
            "Minimum Bathrooms",
            "Minimum Area (m)",
            "Apply Filters & View Results",
            "Reset Filters",
            "Back to Menu"
        };

        int boxX = 22, boxY = 6, boxWidth = 50;

        for (int i = 0; i <= numOptions * 2; i++)
        {
            setXY(boxX, boxY + i);
            cout << "|";
            setXY(boxX + boxWidth, boxY + i);
            cout << "|";
        }
        for (int i = 0; i <= boxWidth; i++)
        {
            setXY(boxX + i, boxY);
            cout << "-";
            setXY(boxX + i, boxY + numOptions * 2);
            cout << "-";
        }

        for (int i = 0; i < numOptions; i++)
        {
            int yPos = boxY + 1 + i * 2;
            setXY(boxX + 2, yPos);
            setAttr(i == selected ? 240 : 15);
            cout << left << setw(30) << filterOptions[i];
        }
        setAttr(15);
    }

    // ================= DISPLAY CURRENT FILTERS =================
    void displayCurrentFilters()
    {
        setAttr(14);
        int x = 75, y = 6;
        setXY(x, y);
        cout << "CURRENT FILTERS:";

        setAttr(10);
        setXY(x, y += 2);
        cout << "Max Price:   $" << fixed << setprecision(2) << currentFilter.maxPrice;
        setXY(x, ++y);
        cout << "Type:        " << (currentFilter.type.empty() ? "Any" : currentFilter.type);
        setXY(x, ++y);
        cout << "Location:    " << (currentFilter.location.empty() ? "Any" : currentFilter.location);
        setXY(x, ++y);
        cout << "Rooms        " << currentFilter.minRooms;
        setXY(x, ++y);
        cout << "Bathrooms    " << currentFilter.minBaths;
        setXY(x, ++y);
        cout << "Area         " << fixed << setprecision(0) << currentFilter.minArea << " m";

        setAttr(15);
    }

    // ================= HANDLE FILTER INPUT =================
    void handleFilterInput(int filterIndex)
    {
        system("cls");
        setAttr(14);
        setXY(30, 5);

        switch (filterIndex)
        {
        case 0:
            cout << "Enter Maximum Price ($): ";
            cin >> currentFilter.maxPrice;
            if (cin.fail())
            {
                cin.clear();
                cin.ignore(10000, '\n');
                currentFilter.maxPrice = 999999999;
            }
            showUpdateConfirmation();
            break;

        case 1:
            cout << "Enter Property Type (Rent/Buy or empty): ";
            cin.clear();
            getline(cin, currentFilter.type);
            if (toLower(currentFilter.type)=="rent")
                currentFilter.type = "Rent";
            else if (toLower(currentFilter.type)== "buy" )
                currentFilter.type = "Buy";
            else if (!currentFilter.type.empty())
                currentFilter.type = "";

            break;


        case 2:
            cout << "Enter Location (partial match): ";
            cin.clear();
            getline(cin, currentFilter.location);
            showUpdateConfirmation();
            break;

        case 3:
            cout << "Enter Minimum Rooms: ";
            cin >> currentFilter.minRooms;
            if (cin.fail())
            {
                cin.clear();
                cin.ignore(10000, '\n');
                currentFilter.minRooms = 0;
            }
            showUpdateConfirmation();
            break;

        case 4:
            cout << "Enter Minimum Bathrooms: ";
            cin >> currentFilter.minBaths;
            if (cin.fail())
            {
                cin.clear();
                cin.ignore(10000, '\n');
                currentFilter.minBaths = 0;
            }
            showUpdateConfirmation();
            break;

        case 5:
            cout << "Enter Minimum Area (m): ";
            cin >> currentFilter.minArea;
            if (cin.fail())
            {
                cin.clear();
                cin.ignore(10000, '\n');
                currentFilter.minArea = 0;
            }
            showUpdateConfirmation();
            break;
        }

    }
    void showUpdateConfirmation()
    {
        setAttr(10);
        setXY(30, 7);
        cout << "Filter updated successfully!";
        setAttr(15);
        setXY(30, 9);
        cout << "Press any key to continue...";
        _getch();
        system("cls");
    }

    void viewFilteredResults(sqlite3 *db)
    {
        auto props = dbManager->filterProperties(currentFilter.maxPrice, currentFilter.type, toLower(currentFilter.location), currentFilter.minRooms, currentFilter.minBaths, currentFilter.minArea);
        pm.DisplayProperties(props, db, dbManager);
    }

    void resetFilters()
    {
        currentFilter = SearchFilter();
        system("cls");
        setAttr(10);
        setXY(30, 10);
        cout << "All filters have been reset!";
        setAttr(15);
        setXY(30, 12);
        cout << "Press any key to continue...";
        _getch();
    }
};

#endif
