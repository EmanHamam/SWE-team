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

public:
    // ================= ADD PROPERTY =================
    void addProperty(sqlite3* db) {
        system("cls");
        int width = 60, height = 18;
        int startX = 10, startY = 2;

        // Draw frame
        for (int y = startY; y <= startY + height; y++) {
            gotoxy(startX, y);
            for (int x = startX; x <= startX + width; x++) {
                if (y == startY || y == startY + height) cout << "-";
                else if (x == startX || x == startX + width) cout << "|";
                else cout << " ";
            }
        }

        // Title
        gotoxy(startX + 20, startY);
        textattr(HIGHLIGHTED_PEN);
        cout << " ADD NEW PROPERTY ";
        textattr(NORMAL_PEN);

        // Labels
        gotoxy(startX + 2, startY + 3);  cout << "Name:";
        gotoxy(startX + 2, startY + 5);  cout << "Location:";
        gotoxy(startX + 2, startY + 7);  cout << "Price:";
        gotoxy(startX + 2, startY + 9);  cout << "Type (Buy/Rent):";
        gotoxy(startX + 2, startY + 11); cout << "Contact Number:";
        gotoxy(startX + 2, startY + 13); cout << "Owner ID:";

        // Show available owners
        gotoxy(startX + 2, startY + 15);
        textattr(8);
        cout << "Available Owners: ";

        string ownerSql = "SELECT owner_id, name FROM owners;";
        sqlite3_stmt* ownerStmt;
        if (sqlite3_prepare_v2(db, ownerSql.c_str(), -1, &ownerStmt, nullptr) == SQLITE_OK) {
            bool first = true;
            while (sqlite3_step(ownerStmt) == SQLITE_ROW) {
                if (!first) cout << ", ";
                cout << sqlite3_column_int(ownerStmt, 0) << "-" << (const char*)sqlite3_column_text(ownerStmt, 1);
                first = false;
            }
        }
        sqlite3_finalize(ownerStmt);
        textattr(NORMAL_PEN);

        // Input fields setup
        int lineno = 6;
        int maxLen = 35;
        char sr[6] = {32, 32, 48, 32, 32, 48};  // Start range (space, space, '0', space, space, '0')
        char er[6] = {126, 126, 57, 126, 126, 57}; // End range (~, ~, '9', ~, ~, '9')

        int editorX = startX + 20;
        char** input = multiLineEditor(editorX, startY + 3, maxLen, sr, er, lineno,false);

        // Trim and validate input
        string name = trim(input[0]);
        string location = trim(input[1]);
        string priceStr = trim(input[2]);
        string type = trim(input[3]);
        string infoNumber = trim(input[4]);
        string ownerIdStr = trim(input[5]);

        // Free memory
        for (int i = 0; i < lineno; i++) delete[] input[i];
        delete[] input;

        // Validation
        gotoxy(startX + 2, startY + height + 2);

        if (name.empty() || location.empty() || priceStr.empty() || type.empty() || infoNumber.empty() || ownerIdStr.empty()) {
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

        // Insert property
        string insertSql = "INSERT INTO properties (name, location, price, type, isAvailable, InfoNumber, owner_id) VALUES ('"
            + name + "', '" + location + "', " + to_string(price) + ", '" + type + "', 1, '" + infoNumber + "', " + to_string(ownerId) + ");";

        char* errMsg = nullptr;
        if (sqlite3_exec(db, insertSql.c_str(), nullptr, nullptr, &errMsg) == SQLITE_OK) {
            // Get the last inserted property ID
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
        cout << " ID    | NAME                | LOCATION          | PRICE              | TYPE      | AVAILABLE\n";
        textattr(15);

        string listSql = "SELECT id, name, location, price, type, isAvailable FROM properties;";
        sqlite3_stmt* listStmt;
        vector<int> propertyIds;

        if (sqlite3_prepare_v2(db, listSql.c_str(), -1, &listStmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(listStmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(listStmt, 0);
                string name = (const char*)sqlite3_column_text(listStmt, 1);
                string location = (const char*)sqlite3_column_text(listStmt, 2);
                double price = sqlite3_column_double(listStmt, 3);
                string type = (const char*)sqlite3_column_text(listStmt, 4);
                int available = sqlite3_column_int(listStmt, 5);

                propertyIds.push_back(id);

                cout << setw(5) << id << " | ";
                cout << setw(18) << name.substr(0, 18) << " | ";
                cout << setw(16) << location.substr(0, 16) << " | ";
                cout << "$" << setw(16) << fixed << setprecision(2) << price << " | ";
                cout << setw(8) << type << " | ";
                cout << (available ? "Yes" : "No") << "\n";
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

        // Check for appointments referencing this property
        string checkAppointmentsSql = "SELECT COUNT(*) FROM appointments WHERE property_id = " + to_string(id) + ";";
        sqlite3_stmt* appStmt;
        int appointmentCount = 0;
        if (sqlite3_prepare_v2(db, checkAppointmentsSql.c_str(), -1, &appStmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(appStmt) == SQLITE_ROW) {
                appointmentCount = sqlite3_column_int(appStmt, 0);
            }
        }
        sqlite3_finalize(appStmt);

        // Delete related appointments first (if any)
        if (appointmentCount > 0) {
            string deleteAppointmentsSql = "DELETE FROM appointments WHERE property_id = " + to_string(id) + ";";
            sqlite3_exec(db, deleteAppointmentsSql.c_str(), nullptr, nullptr, nullptr);
        }

        // Delete property
        string deleteSql = "DELETE FROM properties WHERE id = " + to_string(id) + ";";
        char* errMsg = nullptr;

        cout << "\n";
        if (sqlite3_exec(db, deleteSql.c_str(), nullptr, nullptr, &errMsg) == SQLITE_OK) {
            textattr(10);
            cout << "Property deleted successfully!";
            if (appointmentCount > 0) {
                cout << "\n";
                textattr(8);
                cout << "(" << appointmentCount << " related appointment(s) also removed)";
            }
        } else {
            textattr(12);
            cout << "Failed to delete property: " << errMsg;
            sqlite3_free(errMsg);
        }
        _getch();
    }

    void viewPropertiesByOwner(sqlite3* db) {
    int ownerId;
    cout << "Enter Owner ID: ";
    cin >> ownerId;

    std::string sql = "SELECT p.id, p.name, p.location, p.price, p.type, p.isAvailable, o.name "
                      "FROM properties p "
                      "JOIN owners o ON p.owner_id = o.owner_id "
                      "WHERE o.owner_id = " + std::to_string(ownerId) + ";";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Failed to prepare statement.\n";
        return;
    }

    vector<Property> props;
    vector<string> ownerNames;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Property p;
        p.id = sqlite3_column_int(stmt, 0);
        p.name = (const char*)sqlite3_column_text(stmt, 1);
        p.location = (const char*)sqlite3_column_text(stmt, 2);
        p.price = sqlite3_column_double(stmt, 3);
        p.type = (const char*)sqlite3_column_text(stmt, 4);
        p.available = sqlite3_column_int(stmt, 5);
        props.push_back(p);

        string ownerName = (const char*)sqlite3_column_text(stmt, 6);
        ownerNames.push_back(ownerName);
    }
    sqlite3_finalize(stmt);

    if (props.empty()) {
        cout << "No properties found for this owner.\n";
        _getch();
        return;
    }


    system("cls");
    setAttr(11);
    cout << "====== Properties for Owner: " << ownerNames[0] << " ======\n\n";
    setAttr(240);
    cout << " ID    | NAME                | LOCATION          | PRICE              | TYPE      | AVAILABLE\n";
    setAttr(15);

    for (int i = 0; i < (int)props.size(); i++) {
        cout << setw(5) << props[i].id << " | ";
        cout << setw(18) << props[i].name << " | ";
        cout << setw(16) << props[i].location << " | ";
        cout << "$" << setw(16) << fixed << setprecision(2) << props[i].price << " | ";
        cout << setw(8) << props[i].type << " | ";
        cout << (props[i].available ? "Yes" : "No") << "\n";
    }

    _getch();
}



    void lockUnlockProperty(sqlite3* db) {
        int id;
        cout << "Enter Property ID to lock/unlock: ";
        cin >> id;

        sqlite3_stmt* stmt;
        string sql = "SELECT isAvailable FROM properties WHERE id=" + to_string(id);
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int available = sqlite3_column_int(stmt,0);
                sqlite3_finalize(stmt);

                string sqlUpdate = "UPDATE properties SET isAvailable=" + to_string(available == 1 ? 0 : 1) + " WHERE id=" + to_string(id);
                if (sqlite3_exec(db, sqlUpdate.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK) {
                    cout << "Property " << (available == 1 ? "locked" : "unlocked") << " successfully.\n";
                } else {
                    cout << "Failed to update property.\n";
                }
                _getch();
            } else {
                cout << "Property not found.\n";
                sqlite3_finalize(stmt);
                _getch();
            }
        }
    }
  void updateProperty(sqlite3* db) {
    int id;
    cout << "Enter Property ID to update: ";
    cin >> id;
    cin.ignore();

    sqlite3_stmt* stmt;
    string sql = "SELECT id, name, location, price, type, isAvailable, InfoNumber FROM properties WHERE id=" + to_string(id);
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            Property p;
            p.id = sqlite3_column_int(stmt,0);
            p.name = (const char*)sqlite3_column_text(stmt,1);
            p.location = (const char*)sqlite3_column_text(stmt,2);
            p.price = sqlite3_column_double(stmt,3);
            p.type = (const char*)sqlite3_column_text(stmt,4);
            p.available = sqlite3_column_int(stmt,5);
            p.infoNumber = (const char*)sqlite3_column_text(stmt,6);

            sqlite3_finalize(stmt);

            int lineno = 5;
            int maxLen = 50;
            char sr[5] = {32,32,32,32,32};
            char er[5] = {126,126,126,126,126};
            char** editedLines;

            string fields[5] = {"Name", "Location", "Price", "Type", "Contact"};
            string values[5] = {p.name, p.location, to_string(p.price), p.type, p.infoNumber};

            // Draw labels with proper spacing
            int startX = 5;
            int maxLabelWidth = 0;

            // Find the longest label + value combination
            for(int i=0;i<lineno;i++) {
                int currentWidth = fields[i].length() + values[i].length() + 15;
                if (currentWidth > maxLabelWidth) maxLabelWidth = currentWidth;
            }

            for(int i=0;i<lineno;i++) {
                gotoxy(startX, 5 + i*2);
                cout << fields[i] << " (last value: " << values[i] << "): ";
            }

            // Position editor at a fixed location to ensure all fields are visible
            int editorX[5];
            for(int i=0;i<lineno;i++)
                editorX[i] = startX + maxLabelWidth;

            // Call multiLineEditor
            editedLines = multiLineEditor(editorX[0], 5, maxLen, sr, er, lineno,false);

            string newName = strlen(editedLines[0]) > 0 ? string(editedLines[0]) : p.name;
            string newLocation = strlen(editedLines[1]) > 0 ? string(editedLines[1]) : p.location;
            double newPrice = strlen(editedLines[2]) > 0 ? stod(editedLines[2]) : p.price;
            string newType = strlen(editedLines[3]) > 0 ? string(editedLines[3]) : p.type;
            string newContact = strlen(editedLines[4]) > 0 ? string(editedLines[4]) : p.infoNumber;

            string sqlUpdate = "UPDATE properties SET name='" + newName +
                                "', location='" + newLocation +
                                "', price=" + to_string(newPrice) +
                                ", type='" + newType +
                                "', InfoNumber='" + newContact +
                                "' WHERE id=" + to_string(p.id);

            if (sqlite3_exec(db, sqlUpdate.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK)
                cout << "Property updated successfully.\n";
            else
                cout << "Failed to update property.\n";

            for(int i=0;i<lineno;i++) delete[] editedLines[i];
            delete[] editedLines;

            _getch();
        } else {
            cout << "Property not found.\n";
            sqlite3_finalize(stmt);
            _getch();
        }
    }
}

};

#endif
