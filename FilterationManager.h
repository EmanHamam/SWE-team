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



struct SearchFilter {
    double maxPrice = 999999999;
    string type = "";        // Rent / Buy / empty
    string location = "";    // partial match
    int minRooms = 0;
    int minBaths = 0;
    double minArea = 0;
};

class SearchManager {

    public:
    SearchManager(DBManager* db = nullptr) : dbManager(db) {}

    void setDBManager(DBManager* db) {
        dbManager = db;
    }

    // ================= INTERACTIVE FILTER MENU =================
    void interactiveSearch(sqlite3* db) {
        bool filtering = true;
        int selectedFilter = 0;
        const int numOptions = 9;

        while (filtering) {
            system("cls");
            drawFilterMenu(selectedFilter, numOptions);
            displayCurrentFilters();

            int key = _getch();
            if (key == 0 || key == 224) {
                key = _getch();
                if (key == 72)
                    selectedFilter = (selectedFilter == 0) ? numOptions - 1 : selectedFilter - 1;
                else if (key == 80)
                    selectedFilter = (selectedFilter == numOptions - 1) ? 0 : selectedFilter + 1;
            }
            else if (key == 13) {
                if (selectedFilter == 6) {
                    viewFilteredResults(db);
                }
                else if (selectedFilter == 7) {
                    resetFilters();
                }
                else if (selectedFilter == 8) {
                    filtering = false;
                    system("cls");
                    setAttr(11);
                    setXY(30, 10);
                    cout << "Returning to Main Menu...";
                    setAttr(15);
                    _getch();
                }
                else {
                    handleFilterInput(selectedFilter);
                }
            }
        }
    }
private:
    DBManager* dbManager;
    PropertyManager pm;
    SearchFilter currentFilter;
// =============== Helpers Function ===============
    void drawFilterMenu(int selected, int numOptions) {
        setAttr(11);
        setXY(20, 2);
        cout << "====================================================";
        setXY(20, 3);
        cout << "           ADVANCED PROPERTY SEARCH & FILTER";
        setXY(20, 4);
        cout << "====================================================";

        const char* filterOptions[] = {
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

        for (int i = 0; i <= numOptions * 2; i++) {
            setXY(boxX, boxY + i); cout << "|";
            setXY(boxX + boxWidth, boxY + i); cout << "|";
        }
        for (int i = 0; i <= boxWidth; i++) {
            setXY(boxX + i, boxY); cout << "-";
            setXY(boxX + i, boxY + numOptions * 2); cout << "-";
        }

        for (int i = 0; i < numOptions; i++) {
            int yPos = boxY + 1 + i * 2;
            setXY(boxX + 2, yPos);
            setAttr(i == selected ? 240 : 15);
            cout << left << setw(30) << filterOptions[i];
        }
        setAttr(15);
    }

    // ================= DISPLAY CURRENT FILTERS =================
    void displayCurrentFilters() {
        setAttr(14);
        int x = 75, y = 6;
        setXY(x, y); cout << "CURRENT FILTERS:";

        setAttr(10);
        setXY(x, y += 2); cout << "Max Price:   $" << fixed << setprecision(2) << currentFilter.maxPrice;
        setXY(x, ++y); cout << "Type:        " << (currentFilter.type.empty() ? "Any" : currentFilter.type);
        setXY(x, ++y); cout << "Location:    " << (currentFilter.location.empty() ? "Any" : currentFilter.location);
        setXY(x, ++y); cout << "Rooms >=     " << currentFilter.minRooms;
        setXY(x, ++y); cout << "Bathrooms >= " << currentFilter.minBaths;
        setXY(x, ++y); cout << "Area >=      " << fixed << setprecision(0) << currentFilter.minArea << " m";

        setAttr(15);
    }

    // ================= HANDLE FILTER INPUT =================
    void handleFilterInput(int filterIndex) {
        system("cls");
        setAttr(14);
        setXY(30, 5);

        switch (filterIndex) {
        case 0:
            cout << "Enter Maximum Price ($): ";
            cin >> currentFilter.maxPrice;
            if (cin.fail()) { cin.clear(); cin.ignore(10000, '\n'); currentFilter.maxPrice = 999999999; }
            break;

        case 1:
            cout << "Enter Property Type (Rent/Buy or empty): ";
            cin.ignore();
            getline(cin, currentFilter.type);
            if (currentFilter.type == "rent" || currentFilter.type == "RENT") currentFilter.type = "Rent";
            else if (currentFilter.type == "buy" || currentFilter.type == "BUY") currentFilter.type = "Buy";
            else if (!currentFilter.type.empty()) currentFilter.type = "";
            break;

        case 2:
            cout << "Enter Location (partial match): ";
            cin.ignore();
            getline(cin, currentFilter.location);
            break;

        case 3:
            cout << "Enter Minimum Rooms: ";
            cin >> currentFilter.minRooms;
            if (cin.fail()) { cin.clear(); cin.ignore(10000, '\n'); currentFilter.minRooms = 0; }
            break;

        case 4:
            cout << "Enter Minimum Bathrooms: ";
            cin >> currentFilter.minBaths;
            if (cin.fail()) { cin.clear(); cin.ignore(10000, '\n'); currentFilter.minBaths = 0; }
            break;

        case 5:
            cout << "Enter Minimum Area (m): ";
            cin >> currentFilter.minArea;
            if (cin.fail()) { cin.clear(); cin.ignore(10000, '\n'); currentFilter.minArea = 0; }
            break;
        }

        if (filterIndex < 6) {
            setAttr(10);
            setXY(30, 7);
            cout << "Filter updated successfully!";
            setAttr(15);
            setXY(30, 9);
            cout << "Press any key to continue...";
            _getch();
        }
    }

    // ================= BUILD FILTER QUERY =================
    string buildFilterQuery() {
        string sql =
            "SELECT id, name, location, price, type, isAvailable, InfoNumber, "
            "NoOfRooms, NoOfBaths, Area FROM properties WHERE 1=1 ";

        if (currentFilter.maxPrice < 999999999)
            sql += " AND price <= " + to_string(currentFilter.maxPrice);

        if (!currentFilter.type.empty())
            sql += " AND type = '" + currentFilter.type + "'";

        if (!currentFilter.location.empty())
            sql += " AND LOWER(location) LIKE '%" + currentFilter.location + "%'";

        if (currentFilter.minRooms > 0)
            sql += " AND NoOfRooms >= " + to_string(currentFilter.minRooms);

        if (currentFilter.minBaths > 0)
            sql += " AND NoOfBaths >= " + to_string(currentFilter.minBaths);

        if (currentFilter.minArea > 0)
            sql += " AND Area >= " + to_string(currentFilter.minArea);

        sql += " LIMIT 10";
        return sql;
    }

    // ================= VIEW FILTERED RESULTS =================
    void viewFilteredResults(sqlite3* db) {
        string sql = buildFilterQuery();
        pm.AbstractView(sql, db, dbManager);
    }

    // ================= RESET FILTERS =================
    void resetFilters() {
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
