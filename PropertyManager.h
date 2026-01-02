#ifndef PROPERTY_MANAGER_H
#define PROPERTY_MANAGER_H

#include "UserManager.h"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include "sqlite3.h"
#include <windows.h>
#include <conio.h>
#include "DBManager.h"
#include "Property.h"

using namespace std;

extern bool isLoggedIn;
extern string currentUserEmail;
void setAttr(int i)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), i);
}

void setXY(int x, int y)
{
    COORD coord = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

class PropertyManager
{
public:
    PropertyManager(DBManager *db = nullptr) : dbManager(db) {}
    void DisplayProperties(const vector<Property> &props, sqlite3 *db, DBManager *dbManager)
    {
        if (props.empty())
        {
            system("cls");
            cout << "No properties found. Press any key...";
            _getch();
            return;
        }

        int selected = 0;
        bool inView = true;

        system("cls");
        drawTableFrame();

        while (inView)
        {
            for (int i = 0; i < (int)props.size(); i++)
            {
                int rowY = 8 + (i * 2);

                setAttr(i == selected ? 240 : 15);

                setXY(4, rowY);
                cout << "                                                                                      ";

                setXY(5, rowY);
                cout << props[i].id;
                setXY(12, rowY);
                cout << props[i].name;
                setXY(32, rowY);
                cout << props[i].location;
                setXY(52, rowY);
                cout << "$" << fixed << setprecision(2) << props[i].price;
                setXY(68, rowY);
                cout << props[i].type;
                setXY(80, rowY);
                cout << props[i].area<< " m";

            }

            setAttr(8);
            setXY(5, 27);
            cout << "[Arrows] Navigate | [Enter] Details | [Esc] Back";

            int key = _getch();

            if (key == 0 || key == 224)
            {
                key = _getch();
                if (key == 72)
                    selected = (selected == 0) ? props.size() - 1 : selected - 1;
                else if (key == 80)
                    selected = (selected == props.size() - 1) ? 0 : selected + 1;
            }
            else if (key == 13)
            {
                showDetails(props[selected], db);
                system("cls");
                drawTableFrame();
            }
            else if (key == 27)
            {
                inView = false;
            }
        }
    }
    void ViewAllProperies(sqlite3 *db, DBManager *dbManager)
    {
        vector<Property> props = dbManager->getAllProperties();
        DisplayProperties(props, db, dbManager);
    }

private:
    DBManager *dbManager;
    void setDBManager(DBManager *db)
    {
        dbManager = db;
    }

    void showDetails(Property p, sqlite3 *db, DBManager *dbMgr = nullptr)
    {
        bool showHidden = false;
        int selectedOption = 0;

        while (true)
        {
            system("cls");
            setAttr(15);

            setAttr(11);
            // Draw Box (Extended to 24 rows to fit new data)
            for (int i = 0; i < 23; i++)
            {
                setXY(25, 3 + i);
                cout << "||";
                setXY(75, 3 + i);
                cout << "||";
            }
            setXY(25, 3);
            cout << "====================================================";
            setXY(25, 26);
            cout << "====================================================";

            setAttr(14);
            setXY(38, 5);
            cout << "PROPERTY DOSSIER: " << p.id;

            setAttr(15);
            setXY(30, 7);
            cout << "Name:            " << p.name;
            setXY(30, 8);
            cout << "Location:        " << p.location;
            setXY(30, 9);
            cout << "No Of Rooms      " << p.noOfRooms;
            setXY(30, 10);
            cout << "No Of BathRooms  " << p.noOfBaths;
            setXY(30, 11);
            cout << "Area             " << p.area <<"m";





            if (p.type == "Rent")
            {
                double pricePerDay = p.price;
                double pricePerMonth = p.price * 30 * 0.8;

                setXY(30, 12);
                cout << "Price/Day:       $" << fixed << setprecision(2) << pricePerDay;
                setXY(30, 13);
                cout << "Price/Month:     $" << pricePerMonth << " (20% Disc!)";
            }
            else
            {

                setXY(30, 13);
                cout << "Purchase Price:  $" << fixed << setprecision(2) << p.price;
            }

            setXY(30, 15);
            cout << "Availability:    ";
            if (!p.available)
            {
                setAttr(12);
                cout << "LOCKED / UNAVAILABLE";
            }
            else
            {
                setAttr(10);
                cout << "AVAILABLE FOR " << (p.type == "Rent" ? "LEASING" : "SELL");
            }

            setAttr(15);
            setXY(30, 18);
            cout << "Contact:         " << (showHidden ? p.infoNumber : "******** (Unlock Below)");

            setXY(27, 20);
            setAttr(14);

            if (!isLoggedIn)
            {
                setAttr(14);
                cout << " Guest Mode: Login to unlock contact info.";
                string options[] = {"1. Go Back", "2. Login", "3. Exit System"};
                for (int i = 0; i < 3; i++)
                {
                    setXY(30, 22 + i);
                    if (i == selectedOption)
                    {
                        setAttr(240);
                        cout << " > " << options[i] << " ";
                    }
                    else
                    {
                        setAttr(15);
                        cout << "   " << options[i];
                    }
                }
            }
            else
            {
                setAttr(10);
                cout << "User: " << currentUserEmail;
                setXY(30, 22);
                if (showHidden)
                {
                    setAttr(10);
                    cout << "CONFIRMED! Contact revealed.";
                    setXY(30, 24);
                    setAttr(8);
                    cout << "Press any key to return...";
                    _getch();
                    return;
                }
                else
                {
                    setAttr(240);
                    cout << " [ PRESS ENTER TO " << (p.type == "Rent" ? "RENT" : "BUY") << " ] ";
                    setXY(30, 24);
                    setAttr(15);
                    cout << " [ ESC ] to Cancel ";
                }
            }

            // Input handling
            int key = _getch();
            if (key == 224)
            {
                key = _getch();
                if (key == 72) selectedOption = (selectedOption == 0) ? 2 : selectedOption - 1;
                if (key == 80) selectedOption = (selectedOption == 2) ? 0 : selectedOption + 1;
            }
            else if (key == 13)
            {
                if (!isLoggedIn)
                {
                    if (selectedOption == 0) return;
                    if (selectedOption == 1)
                    {
                        UserManager um;
                        if (um.login(db)) continue;
                    }
                    if (selectedOption == 2) exit(0);
                }
                else if (p.available)
                {
                    showHidden = true;
                }
            }
            else if (key == 27) return;
        }
    }

    void drawTableFrame()
    {
        setAttr(11);
        setXY(30, 2);
        cout << "========= PROPERTY INVENTORY =========";
        setAttr(240);
        setXY(4, 6);
        cout << " ID | NAME              | LOCATION          | PRICE          | Type        | AREA     ";
        setAttr(15);
    }
};

#endif
