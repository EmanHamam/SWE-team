#ifndef PROPERTYMANAGERRR_H_INCLUDED
#define PROPERTYMANAGERRR_H_INCLUDED
struct Property {
    int id;
    string name;
    string location;
    double price;
    string type;
    int available;
    string infoNumber;
    int noOfRooms;
    int noOfBaths;
    double area;
};

void setAttr(int i) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), i);
}

void setXY(int x, int y) {
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

class PropertyManager {
private:
    DBManager* dbManager;

public:
    PropertyManager(DBManager* db = nullptr) : dbManager(db) {}
    void setDBManager(DBManager* db) { dbManager = db; }
//     ================= PROPERTY DETAILS =================
    void showDetails(Property p, sqlite3* db, DBManager* dbMgr = nullptr) {

        bool showHidden = false;
        int selectedOption = 0;

        while (true) {
            system("cls");
            setAttr(11);

            for (int i = 0; i < 22; i++) {
                setXY(25, 3 + i); cout << "||";
                setXY(75, 3 + i); cout << "||";
            }

            setXY(25, 3);  cout << "====================================================";
            setXY(25, 25); cout << "====================================================";

            setAttr(14);
            setXY(38, 5); cout << "PROPERTY DOSSIER: " << p.id;

            setAttr(15);
            setXY(30, 7);  cout << "Name:            " << p.name;
            setXY(30, 8);  cout << "Location:        " << p.location;
            setXY(30, 9);  cout << "Number of Rooms: " << p.noOfRooms;
            setXY(30, 10); cout << "Number of Baths: " << p.noOfBaths;
            setXY(30, 11); cout << "Area:            " << p.area << " m";

            if (p.type == "Rent") {
                double pricePerDay = p.price;
                double pricePerMonth = p.price * 30 * 0.8;
                setXY(30, 12); cout << "Price/Day:       $" << fixed << setprecision(2) << pricePerDay;
                setXY(30, 13); cout << "Price/Month:     $" << pricePerMonth << " (20% Disc!)";
            } else {
                setXY(30, 12); cout << "Purchase Price:  $" << fixed << setprecision(2) << p.price;
            }

            setXY(30, 15); cout << "Availability:    ";
            setAttr(p.available ? 10 : 12);
            cout << (p.available ? "AVAILABLE" : "LOCKED / UNAVAILABLE");

            setAttr(15);
            setXY(30, 17); cout << "Contact:         "
                               << (showHidden ? p.infoNumber : "******** (Unlock Below)");

            // Guest / Logged-in UI
            setXY(27, 19);
            if (!UserManager::currentUser.isLoggedIn) {
                setAttr(14);
                cout << "Guest Mode: Login to unlock contact info.";
            } else {
                setAttr(10);
                cout << "User: " << UserManager::currentUser.email;
            }

            int key = _getch();
            if (key == 13 && UserManager::currentUser.isLoggedIn && p.available) {
                showHidden = true;
            }
            else if (key == 27) return;
        }
    }

    // ================= VIEW PROPERTIES WITH CUSTOM QUERY =================
    void viewProperties(sqlite3* db, const string& query = "SELECT * FROM properties") {
        vector<Property> props = fetchProperties(db, query);

        if (props.empty()) {
            system("cls");
            cout << "No properties found. Press any key...";
            _getch();
            return;
        }

        int selected = 0;
        bool inView = true;

        system("cls");
        drawTableFrame();

        while (inView) {
            for (int i = 0; i < (int)props.size(); i++) {
                int rowY = 8 + (i * 2);

                setAttr(i == selected ? 240 : 15);
                setXY(4, rowY); cout << string(90, ' '); // clear row

                setXY(5, rowY);  cout << props[i].id;
                setXY(12, rowY); cout << props[i].name;
                setXY(32, rowY); cout << props[i].location;
                setXY(52, rowY); cout << "$" << fixed << setprecision(2) << props[i].price;
                setXY(68, rowY); cout << props[i].noOfRooms;
                setXY(80, rowY); cout << props[i].noOfBaths;
                setXY(88, rowY); cout << props[i].area << " m²";
            }

            setAttr(8);
            setXY(5, 27);
            cout << "[Arrows] Navigate | [Enter] Details | [Esc] Back";

            int key = _getch();
            if (key == 0 || key == 224) {
                key = _getch();
                if (key == 72) selected = (selected == 0) ? (int)props.size() - 1 : selected - 1;
                else if (key == 80) selected = (selected == (int)props.size() - 1) ? 0 : selected + 1;
            }
            else if (key == 27) {
                inView = false;
            }
            else if (key == 13) {
                showDetails(props[selected], db, dbManager);
                system("cls");
                drawTableFrame();
            }
        }
    }

private:
    vector<Property> fetchProperties(sqlite3* db, const string& query) {
        vector<Property> list;
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
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
                list.push_back(p);
            }
        }
        sqlite3_finalize(stmt);
        return list;
    }

    void drawTableFrame() {
        setAttr(11);
        setXY(30, 2); cout << "========= PROPERTY INVENTORY =========";
        setAttr(240);
        setXY(4, 6);
        cout << " ID | NAME              | LOCATION        | PRICE        | ROOMS NO    | BATHS NO    | AREA     ";
        setAttr(15);
    }
};


#endif // PROPERTYMANAGERRR_H_INCLUDED
