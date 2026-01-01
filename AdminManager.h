#ifndef ADMIN_MANAGER_H
#define ADMIN_MANAGER_H

#include "PropertyManager.h"
#include "UserManager.h"
#include "sqlite3.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <conio.h>
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include "ConsoleUtils.h"
#define NORMAL_PEN 0x07
#define HIGHLIGHTED_PEN 0x70
using namespace std;

extern bool isLoggedIn;
extern string currentUserEmail;

class AdminManager {
private:
    string trim(const char* s) {
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

    // Draw a fancy table for owners
    void displayOwnersTable(sqlite3* db, int startX, int startY) {
        string ownerSql = "SELECT owner_id, name FROM owners ORDER BY owner_id LIMIT 10;";
        sqlite3_stmt* ownerStmt;

        vector<int> ownerIds;
        vector<string> ownerNames;

        if (sqlite3_prepare_v2(db, ownerSql.c_str(), -1, &ownerStmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(ownerStmt) == SQLITE_ROW) {
                ownerIds.push_back(sqlite3_column_int(ownerStmt, 0));
                ownerNames.push_back((const char*)sqlite3_column_text(ownerStmt, 1));
            }
        }
        sqlite3_finalize(ownerStmt);

        if (ownerIds.empty()) {
            gotoxy(startX, startY);
            textattr(12);
            cout << "No owners available!";
            textattr(NORMAL_PEN);
            return;
        }

        // Draw table header
        gotoxy(startX, startY);
        textattr(240);
        cout << " ID   | Owner Name            ";
        textattr(NORMAL_PEN);

        gotoxy(startX, startY + 1);
        cout << "------+------------------------";

        // Draw owner data
        for (size_t i = 0; i < ownerIds.size(); i++) {
            gotoxy(startX, startY + 2 + i);
            textattr(11);
            cout << " " << setw(4) << left << ownerIds[i];
            textattr(15);
            cout << " | " << setw(20) << left << ownerNames[i].substr(0, 20);
        }
        textattr(NORMAL_PEN);
    }

public:
    // ================= ADD PROPERTY =================
    void addProperty(sqlite3* db) {
        system("cls");
        int width = 50, height = 22;
        int startX = 5, startY = 2;

        // Draw frame
        for (int y = startY; y <= startY + height; y++) {
            gotoxy(startX, y);
            for (int x = startX; x <= startX + width; x++) {
                if (y == startY || y == startY + height) cout << "=";
                else if (x == startX || x == startX + width) cout << "|";
                else cout << " ";
            }
        }

        // Title
        gotoxy(startX + 15, startY);
        textattr(HIGHLIGHTED_PEN);
        cout << " ADD NEW PROPERTY ";
        textattr(NORMAL_PEN);

        // Labels for input fields
        gotoxy(startX + 2, startY + 2);  cout << "Property Name:";
        gotoxy(startX + 2, startY + 4);  cout << "Location:";
        gotoxy(startX + 2, startY + 6);  cout << "Price:";
        gotoxy(startX + 2, startY + 8);  cout << "Type (Buy/Rent):";
        gotoxy(startX + 2, startY + 10); cout << "Contact Number:";
        gotoxy(startX + 2, startY + 12); cout << "Number of Rooms:";
        gotoxy(startX + 2, startY + 14); cout << "Number of Baths:";
        gotoxy(startX + 2, startY + 16); cout << "Area:";
        gotoxy(startX + 2, startY + 18); cout << "Owner ID:";

        // Display owners table OUTSIDE the box on the right side
        int ownerTableX = startX + width + 5;
        gotoxy(ownerTableX, startY + 2);
        textattr(14);
        cout << "Available Owners:";
        textattr(NORMAL_PEN);
        displayOwnersTable(db, ownerTableX, startY + 3);

        // Input fields setup
        int lineno = 9;
        int maxLen = 30;
        char sr[9] = {32, 32, 48, 32, 32, 48, 48, 48, 48};  // Start range
        char er[9] = {126, 126, 57, 126, 126, 57, 57, 57, 57}; // End range

        int editorX = startX + 20;
        char** input = multiLineEditor(editorX, startY + 2, maxLen, sr, er, lineno,false);

        // Trim and validate input
        string name = trim(input[0]);
        string location = trim(input[1]);
        string priceStr = trim(input[2]);
        string type = trim(input[3]);
        string infoNumber = trim(input[4]);
        string roomsStr = trim(input[5]);
        string bathsStr = trim(input[6]);
        string areaStr = trim(input[7]);
        string ownerIdStr = trim(input[8]);

        // Free memory
        for (int i = 0; i < lineno; i++) delete[] input[i];
        delete[] input;

        // Validation
        gotoxy(startX + 2, startY + height + 2);

        if (name.empty() || location.empty() || priceStr.empty() || type.empty() ||
            infoNumber.empty() || roomsStr.empty() || bathsStr.empty() ||
            areaStr.empty() || ownerIdStr.empty()) {
            textattr(12);
            cout << "All fields are required!";
            _getch();
            return;
        }

        // Validate type
        if (type != "Buy" && type != "Rent" && type != "buy" && type != "rent") {
            textattr(12);
            cout << "Type must be 'Buy' or 'Rent'!";
            _getch();
            return;
        }

        // Normalize type
        if (type == "buy") type = "Buy";
        if (type == "rent") type = "Rent";

        double price = stod(priceStr);
        int rooms = stoi(roomsStr);
        int baths = stoi(bathsStr);
        double area = stod(areaStr);
        int ownerId = stoi(ownerIdStr);

        // Validate owner exists
        string checkOwnerSql = "SELECT owner_id FROM owners WHERE owner_id = " + to_string(ownerId) + ";";
        sqlite3_stmt* checkStmt;
        bool ownerExists = false;
        if (sqlite3_prepare_v2(db, checkOwnerSql.c_str(), -1, &checkStmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(checkStmt) == SQLITE_ROW) {
                ownerExists = true;
            }
        }
        sqlite3_finalize(checkStmt);

        if (!ownerExists) {
            textattr(12);
            cout << "Owner ID does not exist!";
            _getch();
            return;
        }

        // Insert property with all fields
        string insertSql = "INSERT INTO properties (name, location, price, type, isAvailable, InfoNumber, NoOfRooms, NoOfBaths, Area, owner_id) VALUES ('"
            + name + "', '" + location + "', " + to_string(price) + ", '" + type + "', 1, '"
            + infoNumber + "', " + to_string(rooms) + ", " + to_string(baths) + ", "
            + to_string(area) + ", " + to_string(ownerId) + ");";

        char* errMsg = nullptr;
        if (sqlite3_exec(db, insertSql.c_str(), nullptr, nullptr, &errMsg) == SQLITE_OK) {
            int propertyId = (int)sqlite3_last_insert_rowid(db);
            textattr(10);
            cout << "Property added successfully! Property ID: " << propertyId;
        } else {
            textattr(12);
            cout << "Failed to add property: " << errMsg;
            sqlite3_free(errMsg);
        }
        _getch();
    }

    // ================= DELETE PROPERTY =================
    void deleteProperty(sqlite3* db) {
        system("cls");

        // Show all properties in a table format
        textattr(11);
        cout << "====== ALL PROPERTIES ======\n\n";
        textattr(240);
        cout << " ID  | NAME              | LOCATION        | PRICE       | TYPE  | ROOMS | BATHS | AREA    | OWNER ID\n";
        textattr(15);

        string listSql = "SELECT id, name, location, price, type, NoOfRooms, NoOfBaths, Area, owner_id FROM properties;";
        sqlite3_stmt* listStmt;
        vector<int> propertyIds;

        if (sqlite3_prepare_v2(db, listSql.c_str(), -1, &listStmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(listStmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(listStmt, 0);
                string name = (const char*)sqlite3_column_text(listStmt, 1);
                string location = (const char*)sqlite3_column_text(listStmt, 2);
                double price = sqlite3_column_double(listStmt, 3);
                string type = (const char*)sqlite3_column_text(listStmt, 4);
                int rooms = sqlite3_column_int(listStmt, 5);
                int baths = sqlite3_column_int(listStmt, 6);
                double area = sqlite3_column_double(listStmt, 7);
                int ownerId = sqlite3_column_int(listStmt, 8);

                propertyIds.push_back(id);

                cout << setw(4) << id << " | ";
                cout << setw(17) << name.substr(0, 17) << " | ";
                cout << setw(15) << location.substr(0, 15) << " | ";
                cout << "$" << setw(10) << fixed << setprecision(0) << price << " | ";
                cout << setw(5) << type << " | ";
                cout << setw(5) << rooms << " | ";
                cout << setw(5) << baths << " | ";
                cout << setw(7) << fixed << setprecision(1) << area << " | ";
                cout << setw(8) << ownerId << "\n";
            }
        }
        sqlite3_finalize(listStmt);

        if (propertyIds.empty()) {
            textattr(12);
            cout << "\nNo properties found in the system!";
            _getch();
            return;
        }

        // Input property ID
        cout << "\n";
        textattr(14);
        cout << "Enter Property ID to delete: ";
        textattr(15);

        int id;
        cin >> id;
        cin.ignore();

        // Check if property exists
        string checkSql = "SELECT id, name FROM properties WHERE id = " + to_string(id) + ";";
        sqlite3_stmt* checkStmt;
        string propName = "";
        bool exists = false;

        if (sqlite3_prepare_v2(db, checkSql.c_str(), -1, &checkStmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(checkStmt) == SQLITE_ROW) {
                exists = true;
                propName = (const char*)sqlite3_column_text(checkStmt, 1);
            }
        }
        sqlite3_finalize(checkStmt);

        if (!exists) {
            textattr(12);
            cout << "\nProperty not found!";
            _getch();
            return;
        }

        // Confirmation
        cout << "\n";
        textattr(14);
        cout << "Are you sure you want to delete property: " << propName << "? (Y/N): ";
        textattr(NORMAL_PEN);

        char confirm = _getch();
        if (confirm != 'Y' && confirm != 'y') {
            cout << "\n";
            textattr(8);
            cout << "Deletion cancelled.";
            _getch();
            return;
        }
    }

    // ================= VIEW PROPERTIES BY OWNER =================
    void viewPropertiesByOwner(sqlite3* db) {
        system("cls");

        textattr(14);
        cout << "====== VIEW PROPERTIES BY OWNER ======\n\n";
        textattr(NORMAL_PEN);

        // Display owners table
        displayOwnersTable(db, 5, 3);

        cout << "\n\n";
        textattr(14);
        cout << "Enter Owner ID: ";
        textattr(15);

        int ownerId;
        cin >> ownerId;
        cin.ignore();

        std::string sql = "SELECT p.id, p.name, p.location, p.price, p.type, p.isAvailable, p.InfoNumber, "
                          "p.NoOfRooms, p.NoOfBaths, p.Area, o.name, o.owner_id "
                          "FROM properties p "
                          "JOIN owners o ON p.owner_id = o.owner_id "
                          "WHERE o.owner_id = " + std::to_string(ownerId) + ";";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            textattr(12);
            cout << "Failed to prepare statement.\n";
            _getch();
            return;
        }

        vector<Property> props;
        string ownerName = "";

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Property p;
            p.id = sqlite3_column_int(stmt, 0);
            p.name = (const char*)sqlite3_column_text(stmt, 1);
            p.location = (const char*)sqlite3_column_text(stmt, 2);
            p.price = sqlite3_column_double(stmt, 3);
            p.type = (const char*)sqlite3_column_text(stmt, 4);
            p.available = sqlite3_column_int(stmt, 5);
            p.infoNumber = (const char*)sqlite3_column_text(stmt, 6);
            p.noOfRooms = sqlite3_column_int(stmt, 7);
            p.noOfBaths = sqlite3_column_int(stmt, 8);
            p.area = sqlite3_column_double(stmt, 9);
            ownerName = (const char*)sqlite3_column_text(stmt, 10);
            props.push_back(p);
        }
        sqlite3_finalize(stmt);

        if (props.empty()) {
            textattr(12);
            cout << "\nNo properties found for this owner.\n";
            _getch();
            return;
        }

        system("cls");
        textattr(11);
        cout << "====== Properties for Owner: " << ownerName << " (ID: " << ownerId << ") ======\n\n";
        textattr(240);
        cout << " ID  | NAME              | LOCATION        | PRICE       | TYPE  | CONTACT      | ROOMS | BATHS | AREA    | AVAIL\n";
        textattr(15);

        for (size_t i = 0; i < props.size(); i++) {
            cout << setw(4) << props[i].id << " | ";
            cout << setw(17) << props[i].name.substr(0, 17) << " | ";
            cout << setw(15) << props[i].location.substr(0, 15) << " | ";
            cout << "$" << setw(10) << fixed << setprecision(0) << props[i].price << " | ";
            cout << setw(5) << props[i].type << " | ";
            cout << setw(12) << props[i].infoNumber.substr(0, 12) << " | ";
            cout << setw(5) << props[i].noOfRooms << " | ";
            cout << setw(5) << props[i].noOfBaths << " | ";
            cout << setw(7) << fixed << setprecision(1) << props[i].area << " | ";
            cout << (props[i].available ? "Yes" : "No") << "\n";
        }

        _getch();
    }



    // ================= LOCK/UNLOCK PROPERTY =================
    void lockUnlockProperty(sqlite3* db) {
        system("cls");

        textattr(14);
        cout << "====== LOCK/UNLOCK PROPERTY ======\n\n";
        textattr(NORMAL_PEN);

        int id;
        cout << "Enter Property ID to lock/unlock: ";
        cin >> id;
        cin.ignore();

        sqlite3_stmt* stmt;
        string sql = "SELECT isAvailable, name FROM properties WHERE id=" + to_string(id);
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int available = sqlite3_column_int(stmt, 0);
                string propName = (const char*)sqlite3_column_text(stmt, 1);
                sqlite3_finalize(stmt);

                string sqlUpdate = "UPDATE properties SET isAvailable=" + to_string(available == 1 ? 0 : 1) + " WHERE id=" + to_string(id);
                if (sqlite3_exec(db, sqlUpdate.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK) {
                    textattr(10);
                    cout << "\nProperty '" << propName << "' " << (available == 1 ? "locked" : "unlocked") << " successfully.\n";
                } else {
                    textattr(12);
                    cout << "\nFailed to update property.\n";
                }
                _getch();
            } else {
                textattr(12);
                cout << "\nProperty not found.\n";
                sqlite3_finalize(stmt);
                _getch();
            }
        }
    }
    // ================= UPDATE PROPERTY =================
    void updateProperty(sqlite3* db) {
        system("cls");

        textattr(14);
        cout << "====== UPDATE PROPERTY ======\n\n";
        textattr(NORMAL_PEN);

        int id;
        cout << "Enter Property ID to update: ";
        cin >> id;
        cin.ignore();

        sqlite3_stmt* stmt;
        string sql = "SELECT p.id, p.name, p.location, p.price, p.type, p.isAvailable, p.InfoNumber, "
                     "p.NoOfRooms, p.NoOfBaths, p.Area, p.owner_id "
                     "FROM properties p WHERE p.id=" + to_string(id);

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                Property p;
                p.id = sqlite3_column_int(stmt, 0);
                p.name = (const char*)sqlite3_column_text(stmt, 1);
                p.location = (const char*)sqlite3_column_text(stmt, 2);
                p.price = sqlite3_column_double(stmt, 3);
                p.type = (const char*)sqlite3_column_text(stmt, 4);
                p.available = sqlite3_column_int(stmt, 5);
                p.infoNumber = (const char*)sqlite3_column_text(stmt, 6);
                p.noOfRooms = sqlite3_column_int(stmt, 7);
                p.noOfBaths = sqlite3_column_int(stmt, 8);
                p.area = sqlite3_column_double(stmt, 9);
                int currentOwnerId = sqlite3_column_int(stmt, 10);

                sqlite3_finalize(stmt);

                system("cls");
                int width = 50, height = 24;
                int startX = 5, startY = 2;

                // Draw frame
                for (int y = startY; y <= startY + height; y++) {
                    gotoxy(startX, y);
                    for (int x = startX; x <= startX + width; x++) {
                        if (y == startY || y == startY + height) cout << "=";
                        else if (x == startX || x == startX + width) cout << "|";
                        else cout << " ";
                    }
                }

                // Title
                gotoxy(startX + 15, startY);
                textattr(HIGHLIGHTED_PEN);
                cout << " UPDATE PROPERTY ";
                textattr(NORMAL_PEN);

                // Display current property info
                gotoxy(startX + 2, startY + 2);
                textattr(11);
                cout << "Property ID: " << p.id;
                textattr(NORMAL_PEN);

                // Labels
                int labelY = startY + 4;
                gotoxy(startX + 2, labelY);      cout << "Name:";
                gotoxy(startX + 2, labelY + 2);  cout << "Location:";
                gotoxy(startX + 2, labelY + 4);  cout << "Price:";
                gotoxy(startX + 2, labelY + 6);  cout << "Type:";
                gotoxy(startX + 2, labelY + 8);  cout << "Contact:";
                gotoxy(startX + 2, labelY + 10); cout << "Rooms:";
                gotoxy(startX + 2, labelY + 12); cout << "Baths:";
                gotoxy(startX + 2, labelY + 14); cout << "Area:";
                gotoxy(startX + 2, labelY + 16); cout << "Owner ID:";

                gotoxy(startX + 2, labelY + 18);
                textattr(8);
                cout << "Blank = keep: " << p.name.substr(0,10) << "..";
                textattr(NORMAL_PEN);

                // Display owners table OUTSIDE the box on the right side
                int ownerTableX = startX + width + 5;
                gotoxy(ownerTableX, startY + 2);
                textattr(14);
                cout << "Available Owners:";
                textattr(NORMAL_PEN);
                displayOwnersTable(db, ownerTableX, startY + 3);

                int lineno = 9;
                int maxLen = 30;
                char sr[9] = {32, 32, 48, 32, 32, 48, 48, 48, 48};
                char er[9] = {126, 126, 57, 126, 126, 57, 57, 57, 57};

                int editorX = startX + 15;
                char** editedLines = multiLineEditor(editorX, labelY, maxLen, sr, er, lineno,false);

                string newName = strlen(editedLines[0]) > 0 ? trim(editedLines[0]) : p.name;
                string newLocation = strlen(editedLines[1]) > 0 ? trim(editedLines[1]) : p.location;
                string newPriceStr = trim(editedLines[2]);
                string newType = strlen(editedLines[3]) > 0 ? trim(editedLines[3]) : p.type;
                string newContact = strlen(editedLines[4]) > 0 ? trim(editedLines[4]) : p.infoNumber;
                string newRoomsStr = trim(editedLines[5]);
                string newBathsStr = trim(editedLines[6]);
                string newAreaStr = trim(editedLines[7]);
                string newOwnerIdStr = trim(editedLines[8]);

                double newPrice = newPriceStr.empty() ? p.price : stod(newPriceStr);
                int newRooms = newRoomsStr.empty() ? p.noOfRooms : stoi(newRoomsStr);
                int newBaths = newBathsStr.empty() ? p.noOfBaths : stoi(newBathsStr);
                double newArea = newAreaStr.empty() ? p.area : stod(newAreaStr);
                int newOwnerId = newOwnerIdStr.empty() ? currentOwnerId : stoi(newOwnerIdStr);

                for (int i = 0; i < lineno; i++) delete[] editedLines[i];
                delete[] editedLines;

                // Validate type
                if (newType != "Buy" && newType != "Rent" && newType != "buy" && newType != "rent") {
                    gotoxy(startX + 2, startY + height + 2);
                    textattr(12);
                    cout << "Type must be 'Buy' or 'Rent'!";
                    _getch();
                    return;
                }

                // Normalize type
                if (newType == "buy") newType = "Buy";
                if (newType == "rent") newType = "Rent";

                // Validate owner exists
                string checkOwnerSql = "SELECT owner_id FROM owners WHERE owner_id = " + to_string(newOwnerId) + ";";
                sqlite3_stmt* checkStmt;
                bool ownerExists = false;
                if (sqlite3_prepare_v2(db, checkOwnerSql.c_str(), -1, &checkStmt, nullptr) == SQLITE_OK) {
                    if (sqlite3_step(checkStmt) == SQLITE_ROW) {
                        ownerExists = true;
                    }
                }
                sqlite3_finalize(checkStmt);

                if (!ownerExists) {
                    gotoxy(startX + 2, startY + height + 2);
                    textattr(12);
                    cout << "Owner ID does not exist!";
                    _getch();
                    return;
                }

                string sqlUpdate = "UPDATE properties SET name='" + newName +
                                    "', location='" + newLocation +
                                    "', price=" + to_string(newPrice) +
                                    ", type='" + newType +
                                    "', InfoNumber='" + newContact +
                                    "', NoOfRooms=" + to_string(newRooms) +
                                    ", NoOfBaths=" + to_string(newBaths) +
                                    ", Area=" + to_string(newArea) +
                                    ", owner_id=" + to_string(newOwnerId) +
                                    " WHERE id=" + to_string(p.id);

                gotoxy(startX + 2, startY + height + 2);
                if (sqlite3_exec(db, sqlUpdate.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK) {
                    textattr(10);
                    cout << "Property updated successfully!";
                } else {
                    textattr(12);
                    cout << "Failed to update property.";
                }

                _getch();
            } else {
                textattr(12);
                cout << "\nProperty not found.\n";
                sqlite3_finalize(stmt);
                _getch();
            }
        }
    }

    // ================= VIEW ALL PROPERTIES (ADMIN VERSION) =================
    void viewAllPropertiesAdmin(sqlite3* db) {
        system("cls");

        textattr(11);
        cout << "====== ALL PROPERTIES (ADMIN VIEW) ======\n\n";
        textattr(240);
        cout << " ID  | NAME              | LOCATION        | PRICE       | TYPE  | CONTACT      | ROOMS | BATHS | AREA    | OWNER ID | AVAIL\n";
        textattr(15);

        string listSql = "SELECT id, name, location, price, type, InfoNumber, NoOfRooms, NoOfBaths, Area, owner_id, isAvailable FROM properties;";
        sqlite3_stmt* listStmt;

        if (sqlite3_prepare_v2(db, listSql.c_str(), -1, &listStmt, nullptr) == SQLITE_OK) {
            bool hasData = false;
            while (sqlite3_step(listStmt) == SQLITE_ROW) {
                hasData = true;
                int id = sqlite3_column_int(listStmt, 0);
                string name = (const char*)sqlite3_column_text(listStmt, 1);
                string location = (const char*)sqlite3_column_text(listStmt, 2);
                double price = sqlite3_column_double(listStmt, 3);
                string type = (const char*)sqlite3_column_text(listStmt, 4);
                string contact = (const char*)sqlite3_column_text(listStmt, 5);
                int rooms = sqlite3_column_int(listStmt, 6);
                int baths = sqlite3_column_int(listStmt, 7);
                double area = sqlite3_column_double(listStmt, 8);
                int ownerId = sqlite3_column_int(listStmt, 9);
                int available = sqlite3_column_int(listStmt, 10);

                cout << setw(4) << id << " | ";
                cout << setw(17) << name.substr(0, 17) << " | ";
                cout << setw(15) << location.substr(0, 15) << " | ";
                cout << "$" << setw(10) << fixed << setprecision(0) << price << " | ";
                cout << setw(5) << type << " | ";
                cout << setw(12) << contact.substr(0, 12) << " | ";
                cout << setw(5) << rooms << " | ";
                cout << setw(5) << baths << " | ";
                cout << setw(7) << fixed << setprecision(1) << area << " | ";
                cout << setw(8) << ownerId << " | ";
                cout << (available ? "Yes" : "No") << "\n";
            }

            if (!hasData) {
                textattr(12);
                cout << "\nNo properties found in the system!";
            }
        }
        sqlite3_finalize(listStmt);

        _getch();
    }
};

#endif
